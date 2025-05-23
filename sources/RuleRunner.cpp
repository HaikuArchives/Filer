/*
	RuleRunner.cpp: class to handle running test and actions for the rules
	Released under the MIT license.
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Contributed by:
		Humdinger <humdingerb@gmail.com>, 2016
		Pete Goodeve
		Owen Pan <owen.pan@yahoo.com>, 2017
		Jaidyn Levesque <jadedctrl@teknik.io>, 2021
*/

#include "RuleRunner.h"

#include <fs_attr.h>

#include <Catalog.h>
#include <Directory.h>
#include <FindDirectory.h>
#include <MimeType.h>
#include <Path.h>
#include <Roster.h>

#include "ConflictWindow.h"
#include "CppSQLite3.h"
#include "Database.h"
#include "FSUtils.h"
#include "main.h"
#include "PatternProcessor.h"

/*
	FilerAction message fields:
	"name"	- the particular action, i.e. copy, move, trash, delete, etc.
	"value"	- string data associated with the action. Usually a file path
	
	FilerTest message fields:
	"name"	- the particular data being compared -- name, date, etc.
			  This is set to"Attribute" if it's an extended attribute
	"mode"	- the kind of comparison being used -- ==, != -- in plain English
	"value"	- the value to be compared to
	
	
	These fields are either not present or NULL in non-attribute tests:
	
	"mimetype"	- This is the MIME type associated with the attribute.
	"typename"	- This is the mime type's short description.
	"attrtype"	- The MIME description for the attribute type, like
				  META:email for email addresses.
	"attrname"	- The public name for the attribute to be compared.
	
	
	To add another test to this file, you'll need to implement an
	IsXxxxMatch(), function make sure it is called from within IsMatch(),
	and add the appropriate name to both sTestTypes[] and either
	sStringTests[], sDateTests[], or sNumberTests[]. 
	
	To add an action to this file, you'll need to implement an xxxxAction()
	function, make sure it is called from within RunAction, and add the
	appropriate action name to sActions[];
	
	In both cases, please make sure that you keep the location within the 
	static array variables when adding or deleting entries
*/

// The various compare functions used by IsMatch to do the actual comparing
static bool IsNameMatch(const BMessage& test, const entry_ref& ref);
static bool IsTypeMatch(const BMessage& test, const entry_ref& ref);
static bool IsSizeMatch(const BMessage& test, const entry_ref& ref);
static bool IsLocationMatch(const BMessage& test, const entry_ref& ref);
//bool IsModifiedMatch(const BMessage& test, const entry_ref& ref);
static bool IsAttributeMatch(const BMessage& test, const entry_ref& ref);
static bool StringCompare(const BString& from, const BString& to, int8 modetype,
				const bool& match_case);

// The various action functions used by RunAction to do the heavy lifting
static status_t MoveAction(const BMessage& action, entry_ref& ref,
					const char* desc);
static status_t CopyAction(const BMessage& action, entry_ref& ref,
					const char* desc);
static status_t RenameAction(const BMessage& action, entry_ref& ref);
static status_t OpenAction(entry_ref& ref);
static status_t ArchiveAction(const BMessage& action, entry_ref& ref);
static status_t CommandAction(const BMessage& action, entry_ref& ref);
static status_t TrashAction(entry_ref& ref);
static status_t DeleteAction(entry_ref& ref);


// Some convenience functions. Deleting the returned BMessage is the
// responsibility of the caller
static BMessage* MakeTest(int8 type, int8 modetype, const char* value,
					const char* mimeType = NULL, const char* typeName = NULL,
					const char* attrType = NULL, const char* attrName = NULL);
static BMessage* MakeAction(int8 type, const char* value);


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "RuleRunner"

#define LOCALIZE(s)	{ s, B_TRANSLATE(s) }


// Internal variables for all the supported types of tests

const NamePair sTestTypes[] =
{
	LOCALIZE("Type"),
	LOCALIZE("Name"),
	LOCALIZE("Size"),
	LOCALIZE("Location")
//	LOCALIZE("Date"),	//	"Last changed"
//	LOCALIZE("Attribute")
};
const unsigned nTestTypes = sizeof(sTestTypes) / sizeof(sTestTypes[0]);

static const TestType stringTests[] = {
	TEST_TYPE,
	TEST_NAME,
	TEST_LOCATION
};
static const unsigned
nStringTests = sizeof(stringTests) / sizeof(stringTests[0]);

static const TestType numberTests[] = {
	TEST_SIZE
};
static const unsigned
nNumberTests = sizeof(numberTests) / sizeof(numberTests[0]);

#if 0
static const TestType dateTests[] = {
//	"Last changed"
};
static const unsigned nDateTests = sizeof(dateTests) / sizeof(dateTests[0]);

static const char* sTestEditors[] =
{
	"type selector",
	"textbox",
	"textbox",
	"textbox",
//	"textbox",
	NULL
};
#endif


// Internal variables for all the supported types of compare operators

const NamePair sModeTypes[] = {
	LOCALIZE("is"),
	LOCALIZE("is not"),
	LOCALIZE("starts with"),
	LOCALIZE("ends with"),
	LOCALIZE("contains"),
	LOCALIZE("does not contain"),
	LOCALIZE("is more than"),
	LOCALIZE("is less than"),
	LOCALIZE("is at least"),
	LOCALIZE("is at most")
//	LOCALIZE("is before"),
//	LOCALIZE("is after")
};
static const unsigned nModeTypes = sizeof(sModeTypes) / sizeof(sModeTypes[0]);

enum ModeType {
	MODE_IS,
	MODE_NOT,
	MODE_START,
	MODE_END,
	MODE_CONTAIN,
	MODE_EXCLUDE,
	MODE_MORE,
	MODE_LESS,
	MODE_LEAST,
	MODE_MOST
//	MODE_BEFORE,
//	MODE_AFTER
};

static const ModeType anyModes[] = {
	MODE_IS,
	MODE_NOT
	// for future expansion
//	"matches pattern"
//	"does not match pattern"
};
static const unsigned nAnyModes = sizeof(anyModes) / sizeof(anyModes[0]);

static const ModeType stringModes[] = {
	MODE_START,
	MODE_END,
	MODE_CONTAIN,
	MODE_EXCLUDE
};
static const unsigned
nStringModes = sizeof(stringModes) / sizeof(stringModes[0]);

static const ModeType numberModes[] = {
	MODE_MORE,
	MODE_LESS,
	MODE_LEAST,
	MODE_MOST
};
static const unsigned
nNumberModes = sizeof(numberModes) / sizeof(numberModes[0]);

#if 0
static const ModeType dateModes[] = {
	MODE_BEFORE,
	MODE_AFTER
};
static const unsigned nDateModes = sizeof(dateModes) / sizeof(dateModes[0]);
#endif

const char* const sSizeUnits[] = {
	B_TRANSLATE("bytes"),
	B_TRANSLATE("KiB"),
	B_TRANSLATE("MiB"),
	B_TRANSLATE("GiB"),
	B_TRANSLATE("TiB")
};
const unsigned nSizeUnits = sizeof(sSizeUnits) / sizeof(sSizeUnits[0]);

enum {
	SIZE_BT,
	SIZE_KB,
	SIZE_MB,
	SIZE_GB,
	SIZE_TB
};

// Internal variable for all the supported types of actions

const NamePair sActions[] = {
	LOCALIZE("Move to folder"),
	LOCALIZE("Copy to folder"),
	LOCALIZE("Rename to"),
	LOCALIZE("Open"),
	LOCALIZE("Add to archive"),
	LOCALIZE("Move to Trash"),
	LOCALIZE("Delete"),
	LOCALIZE("Shell command"),
	LOCALIZE("Continue")
	// Future expansion
//	"Shred it",
//	"E-mail it to",
//	"Make a Deskbar link",
};
const unsigned nActions = sizeof(sActions) / sizeof(sActions[0]);

const char* const kArchiveMime = "application/zip";
const char* const kScriptMime = "text/plain";


RuleRunner::RuleRunner()
{
}


RuleRunner::~RuleRunner()
{
}


void
RuleRunner::GetTestTypes(BMessage& msg)
{
	for (uint32 i = 0; i < nTestTypes; i++)
		msg.AddInt8("tests", i);
}


status_t
RuleRunner::GetCompatibleModes(int8 testtype, BMessage& msg)
{
	if (testtype < 0)
		return B_ERROR;

	return GetCompatibleModes(GetDataTypeForTest(testtype), msg);
}


status_t
RuleRunner::GetCompatibleModes(const int32& type, BMessage& msg)
{
	if (type != TEST_TYPE_STRING && type != TEST_TYPE_NUMBER &&
		/*type != TEST_TYPE_DATE &&*/ type != TEST_TYPE_ANY)
		return B_BAD_VALUE;

	uint32 i;

	for (i = 0; i < nAnyModes; i++)
		msg.AddInt8("modes", anyModes[i]);

	switch (type)
	{
		case TEST_TYPE_STRING:
			for (i = 0; i < nStringModes; i++)
				msg.AddInt8("modes", stringModes[i]);
			break;
		case TEST_TYPE_NUMBER:
			for (i = 0; i < nNumberModes; i++)
				msg.AddInt8("modes", numberModes[i]);
			break;
#if 0
		case TEST_TYPE_DATE:
			for (i = 0; i < nDateModes; i++)
				msg.AddInt8("modes", dateModes[i]);
			break;
#endif
	}
	return B_OK;
}


void
RuleRunner::GetModes(BMessage& msg)
{
	uint32 i;

	for (i = 0; i < nAnyModes; i++)
		msg.AddInt8("modes", anyModes[i]);

	for (i = 0; i < nStringModes; i++)
		msg.AddInt8("modes", stringModes[i]);

	for (i = 0; i < nNumberModes; i++)
		msg.AddInt8("modes", numberModes[i]);

//	for (i = 0; i < nDateModes; i++)
//		msg.AddInt8("modes", dateModes[i]);
}


void
RuleRunner::GetActions(BMessage& msg)
{
	for (uint32 i = 0; i < nActions; i++)
		msg.AddInt8("actions", i);
}


#if 0
BString
RuleRunner::GetEditorTypeForTest(const char* testname)
{
	int32 i = 0;
	while (sTestTypes[i])
	{
		if (strcmp(testname, sTestTypes[i]) == 0)
			return BString(sTestEditors[i]);
		i++;
	}
	return BString("");
}
#endif


bool
RuleRunner::IsMatch(const BMessage& test, const entry_ref& ref)
{
	int8 testtype;
	if (test.FindInt8("name", &testtype) != B_OK) {
		debugger("Couldn't find test type in RuleRunner::IsMatch");
		return false;
	}

	if (testtype == TEST_NAME)
		return IsNameMatch(test, ref);
	else if (testtype == TEST_SIZE)
		return IsSizeMatch(test, ref);
	else if (testtype == TEST_LOCATION)
		return IsLocationMatch(test, ref);
	else if (testtype == TEST_TYPE)
		return IsTypeMatch(test, ref);
//	else if (testtype == TEST_DATE)	//	"Last changed"
//		return IsModifiedMatch(test, ref);
	else if (testtype == TEST_ATTRIBUTE)
		return IsAttributeMatch(test, ref);

	return false;
}


status_t
RuleRunner::RunAction(const BMessage& action, entry_ref& ref, const char* desc)
{
	int8 type;
	if (action.FindInt8("type", &type) != B_OK) {
		debugger("Couldn't find action type in RuleRunner::RunAction");
		return B_ERROR;
	}

	if (type == ACTION_MOVE)
		return MoveAction(action, ref, desc);
	else if (type == ACTION_COPY)
		return CopyAction(action, ref, desc);
	else if (type == ACTION_RENAME)
		return RenameAction(action, ref);
	else if (type == ACTION_OPEN)
		return OpenAction(ref);
	else if (type == ACTION_ARCHIVE)
		return ArchiveAction(action, ref);
	else if (type == ACTION_COMMAND)
		return CommandAction(action, ref);
	else if (type == ACTION_TRASH)
		return TrashAction(ref);
	else if (type == ACTION_DELETE)
		return DeleteAction(ref);
	else if (type == ACTION_CONTINUE)
		return CONTINUE_TESTS;	// arbitrary pos non-B_OK value

	return B_ERROR;
}


status_t
RuleRunner::RunRule(FilerRule* rule, entry_ref& ref)
{
	if (!rule)
		return B_ERROR;

	bool pass;
	const char* desc = rule->GetDescription();
	printf("Running rule '%s'\n", desc);

	if (rule->GetRuleMode() == FILER_RULE_ANY) {
		pass = false;
		for (int32 i = 0; i < rule->CountTests(); i++)
		{
			BMessage* test = rule->TestAt(i);
			if (IsMatch(*test, ref)) {
				pass = true;
				break;
			}
		}
	} else {	// And mode
		pass = true;
		for (int32 i = 0; i < rule->CountTests(); i++)
		{
			BMessage* test = rule->TestAt(i);
			if (!IsMatch(*test, ref)) {
				pass = false;
				break;
			}
		}
	}
	
	if (pass) {
		entry_ref realref;
		BEntry(&ref, true).GetRef(&realref);

		for (int32 i = 0; i < rule->CountActions(); i++)
		{
			BMessage* action = rule->ActionAt(i);

			// Note that this call passes the same ref object from one call to the
			// next. This allows the user to chain actions together. The only thing
			// required to do this is for the particular action to change the ref
			// passed to it.
			status_t status = RunAction(*action, realref, desc);
//			if (status == CONTINUE_TESTS)	// keep ref in sync with reality
			ref = realref;
			if (status != B_OK)
				return status;
		}
		return B_OK;
	}
	return CONTINUE_TESTS;
}


bool
IsNameMatch(const BMessage& test, const entry_ref& ref)
{
	BString value;
	if (test.FindString("value", &value) != B_OK) {
		debugger("Couldn't get value in IsNameMatch");
		return false;
	}

	int8 modetype;
	if (test.FindInt8("mode", &modetype) != B_OK) {
		debugger("Couldn't get mode in IsNameMatch");
		return false;
	}

	bool result = StringCompare(value, BString(ref.name), modetype, true);
	printf("\tName test: %s %s %s - %s\n", ref.name,
		sModeTypes[modetype].locale, value.String(),
		result ? "MATCH" : "NO MATCH");

	return result;
}


bool
IsTypeMatch(const BMessage& test, const entry_ref& ref)
{
	BString value;
	if (test.FindString("value", &value) != B_OK) {
		debugger("Couldn't get value in IsTypeMatch");
		return false;
	}

//	if (value == "image/")
//		debugger("");

	int8 modetype;
	if (test.FindInt8("mode", &modetype) != B_OK) {
		debugger("Couldn't get mode in IsTypeMatch");
		return false;
	}

	BMimeType mimeType;
	if (BMimeType::GuessMimeType(&ref, &mimeType) != B_OK)
		return false;

	bool result = StringCompare(value, mimeType.Type(), modetype, true);
	printf("\tType test: %s %s %s - %s\n", ref.name,
		sModeTypes[modetype].locale, value.String(),
		result ? "MATCH" : "NO MATCH");

	return result;
}


bool
IsSizeMatch(const BMessage& test, const entry_ref& ref)
{
	BString value;
	if (test.FindString("value", &value) != B_OK) {
		debugger("Couldn't get value in IsSizeMatch");
		return false;
	}

	int8 unit;
	if (test.FindInt8("unit", &unit) != B_OK) {
		debugger("Couldn't get unit in IsSizeMatch");
		return false;
	}

	int8 modetype;
	if (test.FindInt8("mode", &modetype) != B_OK) {
		debugger("Couldn't get mode in IsSizeMatch");
		return false;
	}

	BFile file(&ref, B_READ_ONLY);
	if (file.InitCheck() != B_OK)
		return false;

	off_t fileSize;
	file.GetSize(&fileSize);
	file.Unset();

	double sz = atof(value.String());
	for (int32 i = 0; i < unit; i++)
		sz *= 1024;

	off_t size = static_cast<off_t>(round(sz));

	bool result;

	switch (modetype) {
		case MODE_IS:
			result = fileSize == size;
			break;
		case MODE_NOT:
			result = fileSize != size;
			break;
		case MODE_MORE:
			result = fileSize > size;
			break;
		case MODE_LESS:
			result = fileSize < size;
			break;
		case MODE_LEAST:
			result = fileSize >= size;
			break;
		case MODE_MOST:
			result = fileSize <= size;
			break;
		default:
			result = false;
	}

	printf("\tSize test: %s %s %lld - %s\n", ref.name,
		sModeTypes[modetype].locale, size, result ? "MATCH" : "NO MATCH");

	return result;
}


bool
IsLocationMatch(const BMessage& test, const entry_ref& ref)
{
	BString value;
	if (test.FindString("value", &value) != B_OK) {
		debugger("Couldn't get value in IsLocationMatch");
		return false;
	}

	int8 modetype;
	if (test.FindInt8("mode", &modetype) != B_OK) {
		debugger("Couldn't get mode in IsLocationMatch");
		return false;
	}

	// This is a little tricky -- we resolve symlinks in the location test
	entry_ref realref;
	BEntry(&ref).GetRef(&realref);

	BPath path(&realref);
	BString filepath(path.Path());
	filepath.RemoveLast(path.Leaf());

	if (value[value.CountChars() - 1] != '/')
		value << "/";

	bool result = StringCompare(value, filepath.String(), modetype, true);

	printf("\tLocation test: %s %s %s - %s\n", filepath.String(),
		sModeTypes[modetype].locale, value.String(),
		result ? "MATCH" : "NO MATCH");

	return result;
}


#if 0
bool
IsModifiedMatch(const BMessage& test, const entry_ref& ref)
{
	// TODO: Implement using Mr. Peeps! date-parsing code
	return false;
}
#endif


bool
IsAttributeMatch(const BMessage& test, const entry_ref& ref)
{
	BString value;
	if (test.FindString("value", &value) != B_OK) {
		debugger("Couldn't get value in IsTypeMatch");
		return false;
	}

	int8 modetype;
	if (test.FindInt8("mode", &modetype) != B_OK) {
		debugger("Couldn't get mode in IsTypeMatch");
		return false;
	}

	BString attribute;
	if (test.FindString("attrtype", &attribute) != B_OK) {
		debugger("Couldn't get attribute in IsAttributeMatch");
		return false;
	}

	BString string;
	attr_info info;
	BNode node(&ref);
	if (node.InitCheck() != B_OK)
		return false;

	if (node.GetAttrInfo(attribute.String(), &info) != B_OK)
		return false;

	if (node.ReadAttrString(attribute.String(), &string) != B_OK)
		return false;

	bool result = StringCompare(value, string, modetype, true);

	BString attrname;
	if (test.FindString("attrname", &attrname) != B_OK)
		attrname = attribute;

	printf("\tAttribute test: %s %s %s - %s\n", attrname.String(),
		sModeTypes[modetype].locale, value.String(),
		result ? "MATCH" : "NO MATCH");

	return result;
}


bool
StringCompare(const BString& from, const BString& to, int8 modetype,
	const bool& match_case)
{
	if (modetype < 0) {
		debugger("NULL mode in StringCompare");
		return false;
	}

	if (modetype == MODE_IS)
		if (match_case)
			return from.Compare(to) == 0;
		else
			return from.ICompare(to) == 0;
	else if (modetype == MODE_NOT)
		if (match_case)
			return from.Compare(to) != 0;
		else
			return from.ICompare(to) != 0;
	else if (modetype == MODE_CONTAIN)
		if (match_case)
			return to.FindFirst(from) >= 0;
		else
			return to.IFindFirst(from) >= 0;
	else if (modetype == MODE_EXCLUDE)
		if (match_case)
			return to.FindFirst(from) < 0;
		else
			return to.IFindFirst(from) < 0;
	else if (modetype == MODE_START)
		if (match_case)
			return to.FindFirst(from) == 0;
		else
			return to.IFindFirst(from) == 0;
	else if (modetype == MODE_END) {
		int32 pos;
		if (match_case)
			pos = to.FindLast(from);
		else
			pos = to.IFindLast(from);

		return (to.CountChars() - from.CountChars() == pos);
	}	

	return false;
}


static status_t
MoveOrCopy(const BMessage& action, const entry_ref& ref, const char* desc,
	bool move)
{
	BEntry source(&ref);
	if (source.InitCheck() != B_OK)
		return B_ERROR;

	BString value;
	if (action.FindString("value", &value) != B_OK)
		return B_ERROR;

	value = ProcessPatterns(value.String(), ref);

	const char* destDir = value.String();
	BEntry entry(destDir, true);
	if (entry.InitCheck() != B_OK)
		return B_ERROR;

	if (entry.Exists()) {
		if(!entry.IsDirectory())
			return B_ERROR;
	} else if (create_directory(destDir, 0777) != B_OK)
		return B_ERROR;

	App* app = static_cast<App*>(be_app);
	bool doAll = app->DoAll();
	bool replace = app->Replace();

	entry_ref destRef;
	const char* name = ref.name;
	bool conflict = false;

	if (!(replace && doAll)) {
		BString destPath(destDir);

		if (destDir[strlen(destDir) - 1] != '/')
			destPath += '/';

		destPath += name;

		BEntry dest(destPath);
		if (dest.InitCheck() != B_OK)
			return B_ERROR;

		conflict = dest.Exists();
		if (conflict)
			dest.GetRef(&destRef);
	}

	if (conflict && !doAll) {
		BEntry parent;
		source.GetParent(&parent);
		if (parent.InitCheck() != B_OK)
			return B_ERROR;

		BPath path;
		parent.GetPath(&path);
		if (path.InitCheck() != B_OK)
			return B_ERROR;

		ConflictWindow* window = new ConflictWindow(path.Path(), ref, destDir,
			destRef, desc);
		replace = window->Go(doAll);

		app->Replace(replace);
		app->DoAll(doAll);
	}

	if (conflict && !replace) {
		printf("\tSkipped %s\n", name);
		return B_OK;
	}

	status_t status = (move ? MoveFile : CopyFile)(&source, &entry, false);
	if (status == B_OK)
		printf("\t%s %s to %s\n", move ? "Moved" : "Copied", name, destDir);
	else
		printf("\tCouldn't %s %s to %s. Stopping here.\n\t\t"
			"Error Message: %s\n", move ? "move" : "copy", name, destDir,
			strerror(status));

	return status;
}


status_t
MoveAction(const BMessage& action, entry_ref& ref, const char* desc)
{
	return MoveOrCopy(action, ref, desc, true);
}


status_t
CopyAction(const BMessage& action, entry_ref& ref, const char* desc)
{
	return MoveOrCopy(action, ref, desc, false);
}


status_t
RenameAction(const BMessage& action, entry_ref& ref)
{
	BString value;
	status_t status;
	status = action.FindString("value", &value);
	if (status != B_OK)
		return status;
	value = ProcessPatterns(value.String(), ref);

	BEntry entry(value.String(), true);
	status = entry.InitCheck();
	if (status != B_OK || entry.Exists())
		return B_ERROR;

	BEntry source(&ref);
	status = source.InitCheck();
	if (status != B_OK)
		return B_ERROR;

	status = source.Rename(value.String());
	if (status == B_OK) {
		printf("\tRenamed %s to %s\n", ref.name, value.String());
		source.GetRef(&ref);
	} else {
		printf("\tCouldn't rename %s to %s. Stopping here.\n\t\t"
			"Error Message: %s\n", ref.name, value.String(), strerror(status));
	}

	if (status == B_OK)
		source.GetRef(&ref);

	return B_OK;
}


status_t
OpenAction(entry_ref& ref)
{
	entry_ref app;
	BString appName("");
	if (be_roster->FindApp(&ref, &app) == B_OK)
		appName = app.name;

	status_t status = be_roster->Launch(&ref);

	if (status == B_OK)
		printf("\tOpened %s in program %s\n", ref.name, appName.String());
	else {
		// R5 (and probably others) don't seem to want to open folders in
		// Tracker -- FindApp() returns B_OK, but sets the entry_ref of the
		// app to open it to the folder's ref, which is dumb. This works
		// around this apparent stupidity.
		BString typestr;
		if (BNode(&ref).ReadAttrString("BEOS:TYPE", &typestr) == B_OK
				&& typestr.Compare("application/x-vnd.Be-directory") == 0) {
			BMessage* msg = new BMessage(B_REFS_RECEIVED);
			msg->AddRef("refs", &ref);
			be_roster->Launch("application/x-vnd.Be-TRAK", msg);
			printf("\tOpened %s in program Tracker\n", ref.name);
			return B_OK;
		}
		if (appName.CountChars() > 0) {
			printf("\tCouldn't open %s in program %s\n",
				ref.name, appName.String());
		} else
			printf("\tCouldn't open %s -- the system couldn't find a program "
				"to do it.\n", ref.name);
	}
	return status;
}


status_t
ArchiveAction(const BMessage& action, entry_ref& ref)
{
	BString value;
	status_t status;
	status = action.FindString("value", &value);
	if (status != B_OK)
		return status;
	value = ProcessPatterns(value.String(), ref);

	if (value.IsEmpty()) {
		printf("\tCouldn't create archive\n\t\tEmpty archive name\n");
		return B_OK;
	}

	BPath path(&ref);
	BString parentstr = path.Path();
	parentstr.ReplaceLast(path.Leaf(),"");

	if (value.EndsWith("/"))
		value += path.Leaf();
	else if (value == "." || value == ".."
		|| value.EndsWith("/.") || value.EndsWith("/.."))
		value << '/' << path.Leaf();
	else {
		BEntry entry(value);
		if (entry.InitCheck() == B_OK && entry.IsDirectory())
			value << '/' << path.Leaf();
	}

	BString command = "";
	command << "cd '" << parentstr << "'; zip -9 -u -r -y '" << value << "' '"
		<< path.Leaf() << "'";

	int result = system(command.String());
	if (result) {
		printf("\tCouldn't create archive %s\n\t\tError code: %d\n",
			value.String(), result);
	} else
		printf("\tAdded %s to Archive %s\n", ref.name, value.String());

	return B_OK;
}


status_t
CommandAction(const BMessage& action, entry_ref& ref)
{
	BString value;
	status_t status;
	status = action.FindString("value", &value);
	if (status != B_OK)
		return status;
	value = ProcessPatterns(value.String(), ref);

	int result = system(value.String());
	if (result) {
		printf("\tShell Command: %s\n\t\tPossible error: "
			"command returned %d\n", value.String(), result);
	} else
		printf("\tShell Command: %s\n", value.String());

	return B_OK;
}


status_t
TrashAction(entry_ref& ref)
{
	BPath path;
	find_directory(B_TRASH_DIRECTORY, &path);

	BEntry entry(path.Path(), true);
	status_t status = entry.InitCheck();
	if (status != B_OK || !entry.Exists() || !entry.IsDirectory())
		return B_ERROR;

	BEntry source(&ref);
	status = source.InitCheck();
	if (status != B_OK)
		return B_ERROR;

	status = MoveFile(&source, &entry, false);
	if (status == B_OK) {
		printf("\tMoved %s to the Trash\n", ref.name);
		source.GetRef(&ref);
	} else {
		printf("\tCouldn't move %s to the Trash. Stopping here.\n\t\t"
			"Error Message: %s\n", ref.name, strerror(status));
	}
	return B_OK;
}


status_t
DeleteAction(entry_ref& ref)
{
	BEntry entry(&ref);
	BPath path(&entry);

	status_t status = entry.Remove();
	if (status == B_OK)
		printf("\tDeleted %s\n", path.Path());
	else {
		printf("\tCouldn't delete %s. Stopping here.\n\t\tError Message: %s\n",
			path.Path(), strerror(status));
	}
	return status;
}


int32
GetDataTypeForTest(int8 testtype)
{
	if (testtype == TEST_ATTRIBUTE)
		return TEST_TYPE_STRING;

	if (testtype < 0)
		return TEST_TYPE_NULL;

	uint32 i;

	for (i = 0; i < nStringTests; i++)
		if (testtype == stringTests[i])
			return TEST_TYPE_STRING;

	for (i = 0; i < nNumberTests; i++)
		if (testtype == numberTests[i])
			return TEST_TYPE_NUMBER;

#if 0
	for (i = 0; i < nDateTests; i++)
		if (testtype == dateTests[i])
			return TEST_TYPE_DATE;
#endif

	return TEST_TYPE_NULL;
}


int32
GetDataTypeForMode(int8 modetype)
{
	if (modetype < 0)
		return TEST_TYPE_NULL;

	uint32 i;

	for (i = 0; i < nStringModes; i++)
		if (modetype == stringModes[i])
			return TEST_TYPE_STRING;

	for (i = 0; i < nNumberModes; i++)
		if (modetype == numberModes[i])
			return TEST_TYPE_NUMBER;

#if 0
	for (i = 0; i < nDateModes; i++)
		if (modetype == dateModes[i])
			return TEST_TYPE_DATE;
#endif

	for (i = 0; i < nAnyModes; i++)
		if (modetype == anyModes[i])
			return TEST_TYPE_ANY;

	return TEST_TYPE_NULL;
}


status_t
LoadRules(BObjectList<FilerRule>* ruleList)
{
	BPath path;
	find_directory(B_USER_SETTINGS_DIRECTORY, &path);

	path.Append("Filer");
	status_t ret = create_directory(path.Path(), 0777);
	path.Append("FilerRules");
	if (ret != B_OK)
		return ret;

	BFile file(path.Path(), B_READ_ONLY);
	ret = file.InitCheck();
	if (ret != B_OK)
		return ret;

	BMessage rulesMsg;
	ret = rulesMsg.Unflatten(&file);

	// Import from SQL if necessary
	if (ret != B_OK && ret != B_NO_MEMORY && LoadSQLRules(ruleList) == B_OK) {
		ret = BEntry(path.Path()).Rename("FilerRules.sql");
		if (ret == B_OK)
			return SaveRules(ruleList);
		return ret;
	}

	BMessage ruleMsg;
	for (int i = 0; rulesMsg.FindMessage("rule", i, &ruleMsg) == B_OK; i++) {
		FilerRule* rule = FilerRule::Instantiate(&ruleMsg);
		if (rule != NULL)
			ruleList->AddItem(rule);
	}
	return ret;
}


status_t
SaveRules(const BObjectList<FilerRule>* ruleList)
{
	BPath path;
	find_directory(B_USER_SETTINGS_DIRECTORY, &path);

	path.Append("Filer");
	status_t ret = create_directory(path.Path(), 0777);
	path.Append("FilerRules");
	if (ret != B_OK)
		return ret;

	BFile file(path.Path(), B_WRITE_ONLY | B_CREATE_FILE);
	ret = file.InitCheck();
	if (ret != B_OK)
		return ret;

	BMessage rulesMsg;
	for (int i = 0; i < ruleList->CountItems(); i++) {
		BMessage ruleMsg;
		FilerRule* rule = ruleList->ItemAt(i);
		rule->Archive(&ruleMsg, true);
		rulesMsg.AddMessage("rule", &ruleMsg);
	}
	return rulesMsg.Flatten(&file, NULL);
}


status_t
LoadSQLRules(BObjectList<FilerRule>* ruleList)
{
	BPath path;
	find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	path.Append("Filer/FilerRules");

	BEntry entry(path.Path());
	if (!entry.Exists())
		return B_OK;

	CppSQLite3DB db;
	db.open(path.Path());

	// Because this particular build of sqlite3 does not support multithreading
	// because of lack of pthreads support, we need to do this in a slightly
	// different order

	CppSQLite3Query query(DBQuery(db, "select * from RuleList limit 0;", "RuleTab::LoadRules"));
	bool legacy = query.numFields() == 2;
	query.finalize();

	BString select("select name");
	if (!legacy)
		select += ", disabled";

	select += " from RuleList order by ruleid;";
	query = DBQuery(db, select, "RuleTab::LoadRules");

	BString command;
	while (!query.eof())
	{
		BString rulename = query.getStringField((int)0);

		FilerRule* rule = new FilerRule;
		rule->SetDescription(DeescapeIllegalCharacters(rulename.String()).String());
		rule->Disabled(legacy ? false : query.getIntField(1));

		ruleList->AddItem(rule);

		query.nextRow();
	}

	query.finalize();

	for (int32 i = 0; i < ruleList->CountItems(); i++)
	{
		FilerRule* rule = ruleList->ItemAt(i);

		if (!rule)
			continue;

		BString rulename(EscapeIllegalCharacters(rule->GetDescription()));

		// Now comes the fun(?) part: loading the tests and actions. Joy. :/
		command = "select * from ";
		command << rulename << " where entrytype = 'test';";
		query = DBQuery(db, command.String(), "RuleTab::LoadRules");

		while (!query.eof())
		{
			BMessage* test = new BMessage;
			int8 type, modetype, unit;
			type = modetype = unit = 0;

			if (legacy) {
				BString classname =
					DeescapeIllegalCharacters(query.getStringField(1));
				BString modename =
					DeescapeIllegalCharacters(query.getStringField(2));

				for (uint32 i = 0; i < nTestTypes; i++)
					if (strcmp(classname.String(), sTestTypes[i].english) == 0) {
						type = i;
						break;
					}

				for (uint32 i = 0; i < nModeTypes; i++) {
					if (strcmp(modename.String(), sModeTypes[i].english) == 0) {
						modetype = i;
						break;
					}
				}
			} else {
				type = query.getIntField(1);
				modetype = query.getIntField(2);

				if (query.numFields() == 8)
					unit = query.getIntField(7);
			}

			test->AddInt8("name", type);

			if (type == TEST_ATTRIBUTE) {
				test->AddString("attrtype",
					DeescapeIllegalCharacters(query.getStringField(4)));
				test->AddString("typename",
					DeescapeIllegalCharacters(query.getStringField(5)));
				test->AddString("attrname",
					DeescapeIllegalCharacters(query.getStringField(6)));
			}

			test->AddInt8("mode", modetype);
			test->AddInt8("unit", unit);
			test->AddString("value",
				DeescapeIllegalCharacters(query.getStringField(3)).String());

			rule->AddTest(test);

			query.nextRow();
		}
		query.finalize();

		command = "select * from ";
		command << rulename << " where entrytype = 'action';";
		query = DBQuery(db, command.String(), "RuleTab::LoadRules");
		
		while (!query.eof())
		{
			int8 type = 0;
			BMessage* action = new BMessage;

			if (legacy) {
				BString actionname =
					DeescapeIllegalCharacters(query.getStringField(1));
				for (uint32 i = 0; i < nActions; i++)
					if (strcmp(actionname.RemoveLast(B_UTF8_ELLIPSIS).String(),
						sActions[i].english) == 0) {
						type = i;
						break;
					}
			} else
				type = query.getIntField(1);

			action->AddInt8("type", type);
			action->AddString("value",
				DeescapeIllegalCharacters(query.getStringField(3)));

			rule->AddAction(action);

			query.nextRow();
		}
		query.finalize();
	}
	db.close();

	return legacy ? SaveRules(ruleList) : B_OK;
}


BMessage*
MakeTest(int8 type, int8 modetype, const char* value,
	const char* mimeType, const char* typeName, const char* attrType, 
	const char* attrName)
{
	BMessage* msg = new BMessage;
	msg->AddInt8("name", type);
	msg->AddInt8("mode", modetype);
	msg->AddString("value", value);

	if (typeName || mimeType || attrType || attrName) {
		if (!(typeName && mimeType && attrType && attrName)) {
			debugger("The last 4 parameters must all be either NULL "
				"or non-NULL as a group");
		}
		msg->AddString("typename", typeName);
		msg->AddString("mimetype", mimeType);
		msg->AddString("attrtype", attrType);
		msg->AddString("attrname", attrName);
	}
	return msg;
}


BMessage*
MakeAction(int8 type, const char* value)
{
	BMessage* msg = new BMessage;
	msg->AddInt8("type", type);
	msg->AddString("value", value);

	return msg;
}


int8
AttributeTestType()
{
	return TEST_ATTRIBUTE;
}


bool
ActionHasTarget(int8 type)
{
	switch (type) {
		case ACTION_OPEN:
		case ACTION_TRASH:
		case ACTION_DELETE:
		case ACTION_CONTINUE:
			return false;
	}

	return true;
}


static bool
GetDirectoryPath(BString& str, const entry_ref& ref)
{
	BEntry entry(&ref);
	if (entry.InitCheck() != B_OK)
		return false;

	if (!entry.IsDirectory() && entry.GetParent(&entry) != B_OK)
		return false;

	BPath path(&entry);
	if (path.InitCheck() != B_OK)
		return false;

	str = path.Path();
	return true;
}


static bool
GetPathForRef(BString& str, const entry_ref& ref, const char* mime)
{
	BEntry entry(&ref, true);
	if (entry.InitCheck() != B_OK)
		return false;

	if (!entry.IsDirectory()) {
		entry_ref target;
		if (entry.GetRef(&target) != B_OK)
			return false;

		BMimeType mimeType;
		if (BMimeType::GuessMimeType(&target, &mimeType) != B_OK
			|| strcmp(mimeType.Type(), mime))
			return false;
	}

	BPath path(&ref);
	if (path.InitCheck() != B_OK)
		return false;

	str = path.Path();
	return true;
}


bool
SetTextForType(BString& text, int8 type, const entry_ref& ref, bool isTest)
{
	entry_ref target;
	BEntry entry(&ref, true);
	if (entry.InitCheck() != B_OK || entry.GetRef(&target) != B_OK)
		return false;

	if (isTest)
		switch (type) {
			case TEST_TYPE:
				return SetTextForMime(text, ref);
			case TEST_NAME:
				text = ref.name;
				return true;
			case TEST_SIZE:
				return SetTextForSize(text, target);
			case TEST_LOCATION:
				return GetDirectoryPath(text, target);
			default:
				return false;
		}

	switch (type) {
		case ACTION_MOVE:
		case ACTION_COPY:
			return GetDirectoryPath(text, target);
		case ACTION_RENAME:
			text = ref.name;
			return true;
		case ACTION_COMMAND:
			return !entry.IsFile() ? false
					: GetPathForRef(text, ref, kScriptMime);
		case ACTION_ARCHIVE:
			return GetPathForRef(text, target, kArchiveMime);
		default:
			return false;
	}
}


bool
SetTextForMime(BString& text, const entry_ref& ref)
{
	BMimeType mimeType;
	if (BMimeType::GuessMimeType(&ref, &mimeType) != B_OK)
		return false;

	text = mimeType.Type();
	return true;
}


bool
SetTextForSize(BString& text, const entry_ref& ref)
{
	BEntry entry(&ref);
	if (entry.InitCheck() != B_OK || !entry.IsFile())
		return false;

	off_t size;
	if (entry.GetSize(&size) != B_OK)
		return false;

	text = "";
	text << size;
	return true;
}


void
AddDefaultRules(BObjectList<FilerRule>* ruleList)
{
	FilerRule* rule = new FilerRule();

	rule->AddTest(MakeTest(TEST_TYPE, MODE_IS, "text/plain"));
	rule->AddAction(MakeAction(ACTION_MOVE, B_TRANSLATE_COMMENT(
		"/boot/home/Documents", "Path for default rule moving text files")));
	rule->SetDescription(B_TRANSLATE_COMMENT(
		"Store text files in my Documents folder",
		"One of the default rule names"));
	ruleList->AddItem(rule);

	rule = new FilerRule();
	rule->AddTest(MakeTest(TEST_TYPE, MODE_IS, "application/pdf"));
	rule->AddAction(MakeAction(ACTION_MOVE, B_TRANSLATE_COMMENT(
		"/boot/home/Documents", "Path for default rule moving PDF files")));
	rule->SetDescription(B_TRANSLATE_COMMENT(
		"Store PDF files in my Documents folder",
		"One of the default rule names"));
	ruleList->AddItem(rule);

	rule = new FilerRule();
	rule->AddTest(MakeTest(TEST_TYPE, MODE_START, "image/"));
	rule->AddAction(MakeAction(ACTION_MOVE, B_TRANSLATE_COMMENT(
		"/boot/home/Pictures", "Path for default rule moving image files")));
	rule->SetDescription(B_TRANSLATE_COMMENT(
		"Store pictures in my Pictures folder",
		"One of the default rule names"));
	ruleList->AddItem(rule);

	rule = new FilerRule();
	rule->AddTest(MakeTest(TEST_TYPE, MODE_START, "video/"));
	rule->AddAction(MakeAction(ACTION_MOVE, B_TRANSLATE_COMMENT(
		"/boot/home/Videos", "Path for default rule moving video files")));
	rule->SetDescription(B_TRANSLATE_COMMENT(
		"Store movie files in my Videos folder",
		"One of the default rule names"));
	ruleList->AddItem(rule);

	rule = new FilerRule();
	rule->AddTest(MakeTest(TEST_TYPE, MODE_IS, "application/zip"));
	rule->AddAction(MakeAction(ACTION_COMMAND,
		"unzip \"%FULLPATH%\" -d /boot/home/Desktop"));
	rule->SetDescription(B_TRANSLATE_COMMENT(
		"Extract ZIP files to the Desktop",
		"One of the default rule names"));
	ruleList->AddItem(rule);

//			rule = new FilerRule();
//			rule->AddTest(MakeTest("","",""));
//			rule->AddAction(MakeAction("",""));
//			rule->SetDescription("");
//			AddRule(rule);
}
