/*
	RuleRunner.cpp: class to handle running test and actions for the rules
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/
#include "RuleRunner.h"
#include "CppSQLite3.h"
#include "Database.h"
#include "FSUtils.h"
#include "PatternProcessor.h"

#include <Entry.h>
#include <fs_attr.h>
#include <Path.h>
#include <Mime.h>
#include <stdlib.h>
#include <Directory.h>
#include <Roster.h>

/*
	FilerAction message fields:
	"name"	- the particular action, i.e. copy, move, trash, delete, etc.
	"value"	- string data associated with the action. Usually a file path
	
	FilerTest message fields:
	"name"	- the particular data being compared -- name, date, etc. This is set to
				"Attribute" if it's an extended attribute
	"mode"	- the kind of comparison being used -- ==, != -- in plain English
	"value"	- the value to be compared to
	
	
	These fields are either not present or NULL in non-attribute tests:
	
	"mimetype"	- This is the MIME type associated with the attribute.
	"typename"	- This is the mime type's short description.
	"attrtype"	- The MIME description for the attribute type, like META:email for email
				  addresses.
	"attrname"	- The public name for the attribute to be compared.
	
	
	To add another test to this file, you'll need to implement an IsXxxxMatch() function,
	make sure it is called from within IsMatch(), and add the appropriate name to both
	sTestTypes[] and either sStringTests[], sDateTests[], or sNumberTests[]. 
	
	To add an action to this file, you'll need to implement an xxxxAction() function,
	make sure it is called from within RunAction, and add the appropriate action name
	to sActions[];
	
	In both cases, please make sure that you keep the location within the static array
	variables when adding or deleting entries
*/

// The various compare functions used by IsMatch to do the actual comparing
bool IsNameMatch(const BMessage &test, const entry_ref &ref);
bool IsTypeMatch(const BMessage &test, const entry_ref &ref);
bool IsSizeMatch(const BMessage &test, const entry_ref &ref);
bool IsLocationMatch(const BMessage &test, const entry_ref &ref);
bool IsModifiedMatch(const BMessage &test, const entry_ref &ref);
bool IsAttributeMatch(const BMessage &test, const entry_ref &ref);
bool StringCompare(const BString &from, const BString &to, const char *mode,
					const bool &match_case);

// The various action functions used by RunAction to do the heavy lifting
status_t MoveAction(const BMessage &action, entry_ref &ref);
status_t CopyAction(const BMessage &action, entry_ref &ref);
status_t RenameAction(const BMessage &action, entry_ref &ref);
status_t OpenAction(const BMessage &action, entry_ref &ref);
status_t ArchiveAction(const BMessage &action, entry_ref &ref);
status_t CommandAction(const BMessage &action, entry_ref &ref);
status_t TrashAction(const BMessage &action, entry_ref &ref);
status_t DeleteAction(const BMessage &action, entry_ref &ref);


// Internal variables for all the supported types of tests

static const char *sTestTypes[] =
{
	"Type",
	"Name",
	"Size",
	"Location",
//	"Last Changed",
	NULL
};

static const char *sStringTests[] =
{
	"Type",
	"Name",
	"Location",
	NULL
};

static const char *sNumberTests[] =
{
	"Size",
	NULL
};

static const char *sDateTests[] =
{
//	"Last Changed",
	NULL
};


static const char *sTestEditors[] =
{
	"type selector",
	"textbox",
	"textbox",
	"textbox",
//	"textbox",
	NULL
};


// Internal variables for all the supported types of compare operators

static const char *sAnyModes[] =
{
	"is",
	"is not",
	
	// for future expansion
//	"matches pattern",
//	"does not match pattern",
	
	NULL
};

static const char *sStringModes[] =
{	
	"starts with",
	"ends with",
	"contains",
	"does not contain",
	NULL
};

static const char *sNumberModes[] =
{
	"is more than",
	"is less than",
	"is at least",
	"is at most",
	NULL
};

static const char *sDateModes[] =
{
	"is before",
	"is after",
	NULL
};


// Internal variable for all the supported types of actions

static const char *sActions[] =
{
	"Move it to…",
	"Copy it to…",
	"Rename it to…",
	"Open it",
	"Add it to the archive…",
	"Move it to the Trash",
	"Delete it",
	"Terminal command…",	
	
	// Future expansion
//	"Shred it",
//	"E-mail it to…",
//	"Make a Deskbar link",
	
	NULL
};


RuleRunner::RuleRunner(void)
{
}


RuleRunner::~RuleRunner(void)
{
}


void
RuleRunner::GetTestTypes(BMessage &msg)
{
	int32 i = 0;
	while (sTestTypes[i])
	{
		msg.AddString("tests",sTestTypes[i]);
		i++;
	}
}


status_t
RuleRunner::GetCompatibleModes(const char *testtype, BMessage &msg)
{
	if (!testtype)
		return B_ERROR;
	
	return GetCompatibleModes(GetDataTypeForTest(testtype),msg);
}


status_t
RuleRunner::GetCompatibleModes(const int32 &type, BMessage &msg)
{
	int32 i = 0;
	switch (type)
	{
		case TEST_TYPE_STRING:
		{
			while (sAnyModes[i])
			{
				msg.AddString("modes",sAnyModes[i]);
				i++;
			}
			
			i = 0;
			while (sStringModes[i])
			{
				msg.AddString("modes",sStringModes[i]);
				i++;
			}
			break;
		}
		case TEST_TYPE_NUMBER:
		{
			while (sAnyModes[i])
			{
				msg.AddString("modes",sAnyModes[i]);
				i++;
			}
			
			i = 0;
			while (sNumberModes[i])
			{
				msg.AddString("modes",sNumberModes[i]);
				i++;
			}
			break;
		}
		case TEST_TYPE_DATE:
		{
			while (sAnyModes[i])
			{
				msg.AddString("modes",sAnyModes[i]);
				i++;
			}
			
			i = 0;
			while (sDateModes[i])
			{
				msg.AddString("modes",sDateModes[i]);
				i++;
			}
			break;
		}
		case TEST_TYPE_ANY:
		{
			while (sAnyModes[i])
			{
				msg.AddString("modes",sAnyModes[i]);
				i++;
			}
			break;
		}
		default:
			return B_BAD_VALUE;
	}
	return B_OK;
}


void
RuleRunner::GetModes(BMessage &msg)
{
	int32 i;
	
	i = 0;
	while (sAnyModes[i])
	{
		msg.AddString("modes",sAnyModes[i]);
		i++;
	}
	
	i = 0;
	while (sStringModes[i])
	{
		msg.AddString("modes",sStringModes[i]);
		i++;
	}
	
	i = 0;
	while (sNumberModes[i])
	{
		msg.AddString("modes",sNumberModes[i]);
		i++;
	}
	
	i = 0;
	while (sDateModes[i])
	{
		msg.AddString("modes",sDateModes[i]);
		i++;
	}
}


void
RuleRunner::GetActions(BMessage &msg)
{
	int32 i = 0;
	while (sActions[i])
	{
		msg.AddString("actions",sActions[i]);
		i++;
	}
}


BString
RuleRunner::GetEditorTypeForTest(const char *testname)
{
	int32 i = 0;
	while (sTestTypes[i])
	{
		if (strcmp(testname,sTestTypes[i]) == 0)
			return BString(sTestEditors[i]);
		i++;
	}
	return BString("");
}


int32
RuleRunner::GetDataTypeForTest(const char *testname)
{
	if (!testname)
		return TEST_TYPE_NULL;
	
	int32 i;
	
	i = 0;
	while (sStringTests[i])
	{
		if (strcmp(testname,sStringTests[i]) == 0)
			return TEST_TYPE_STRING;
		i++;
	}
	
	i = 0;
	while (sNumberTests[i])
	{
		if (strcmp(testname,sNumberTests[i]) == 0)
			return TEST_TYPE_NUMBER;
		i++;
	}
	
	i = 0;
	while (sDateTests[i])
	{
		if (strcmp(testname,sDateTests[i]) == 0)
			return TEST_TYPE_DATE;
		i++;
	}
	
	return TEST_TYPE_NULL;
}


int32
RuleRunner::GetDataTypeForMode(const char *modename)
{
	if (!modename)
		return TEST_TYPE_NULL;
	
	int32 i;
	
	i = 0;
	while (sStringModes[i])
	{
		if (strcmp(modename,sStringModes[i]) == 0)
			return TEST_TYPE_STRING;
		i++;
	}
	
	i = 0;
	while (sNumberModes[i])
	{
		if (strcmp(modename,sNumberModes[i]) == 0)
			return TEST_TYPE_NUMBER;
		i++;
	}
	
	i = 0;
	while (sDateModes[i])
	{
		if (strcmp(modename,sDateModes[i]) == 0)
			return TEST_TYPE_DATE;
		i++;
	}
	
	i = 0;
	while (sAnyModes[i])
	{
		if (strcmp(modename,sAnyModes[i]) == 0)
			return TEST_TYPE_ANY;
		i++;
	}
	
	return TEST_TYPE_NULL;
}

bool
RuleRunner::IsMatch(const BMessage &test, const entry_ref &ref)
{
	BString testname;
	if (test.FindString("name",&testname) != B_OK)
	{
		debugger("Couldn't find test name in RuleRunner::IsMatch");
		return false;
	}
	
	if (testname.Compare("Name") == 0)
		return IsNameMatch(test,ref);
	else if (testname.Compare("Size") == 0)
		return IsSizeMatch(test,ref);
	else if (testname.Compare("Location") == 0)
		return IsLocationMatch(test,ref);
	else if (testname.Compare("Type") == 0)
		return IsTypeMatch(test,ref);
	else if (testname.Compare("Last Changed") == 0)
		return IsModifiedMatch(test,ref);
	else if (testname.Compare("Attribute") == 0)
		return IsAttributeMatch(test,ref);
		
	return false;
}


status_t
RuleRunner::RunAction(const BMessage &action, entry_ref &ref)
{
	BString actionname;
	if (action.FindString("name",&actionname) != B_OK)
	{
		debugger("Couldn't find action name in RuleRunner::RunAction");
		return B_ERROR;
	}
	
	if (actionname.Compare("Move it to…") == 0)
		return MoveAction(action,ref);
	else if (actionname.Compare("Copy it to…") == 0)
		return CopyAction(action,ref);
	else if (actionname.Compare("Rename it to…") == 0)
		return RenameAction(action,ref);
	else if (actionname.Compare("Open it") == 0)
		return OpenAction(action,ref);
	else if (actionname.Compare("Add it to the archive…") == 0)
		return ArchiveAction(action,ref);
	else if (actionname.Compare("Terminal command…") == 0)
		return CommandAction(action,ref);
	else if (actionname.Compare("Move it to the Trash") == 0)
		return TrashAction(action,ref);
	else if (actionname.Compare("Delete it") == 0)
		return DeleteAction(action,ref);
	
	return B_ERROR;
}


status_t
RuleRunner::RunRule(FilerRule *rule, entry_ref &ref)
{
	if (!rule)
		return B_ERROR;
	
	bool pass;
	
	printf("Running rule '%s'\n",rule->GetDescription());
	
	if (rule->GetRuleMode() == FILER_RULE_ANY)
	{
		pass = false;
		for (int32 i = 0; i < rule->CountTests(); i++)
		{
			BMessage *test = rule->TestAt(i);
			if (IsMatch(*test,ref))
			{
				pass = true;
				break;
			}
		}
	}
	else	// And mode
	{
		pass = true;
		for (int32 i = 0; i < rule->CountTests(); i++)
		{
			BMessage *test = rule->TestAt(i);
			if (!IsMatch(*test,ref))
			{
				pass = false;
				break;
			}
		}
	}
	
	if (pass)
	{
		entry_ref realref;
		BEntry(&ref,true).GetRef(&realref);
		
		for (int32 i = 0; i < rule->CountActions(); i++)
		{
			BMessage *action = rule->ActionAt(i);
			
			// Note that this call passes the same ref object from one call to the
			// next. This allows the user to chain actions together. The only thing
			// required to do this is for the particular action to change the ref
			// passed to it.
			status_t status = RunAction(*action,realref);
			if (status != B_OK)
				return status;
		}
	}
	
	return B_OK;
}


bool
IsNameMatch(const BMessage &test, const entry_ref &ref)
{
	BString value;
	if (test.FindString("value",&value) != B_OK)
	{
		debugger("Couldn't get value in IsNameMatch");
		return false;
	}
	
	BString compare;
	if (test.FindString("mode",&compare) != B_OK)
	{
		debugger("Couldn't get mode in IsNameMatch");
		return false;
	}
	
	bool result = StringCompare(value,BString(ref.name),compare.String(),true);
	
	printf("\tName test: %s %s %s - %s\n",ref.name,compare.String(),value.String(),
								result ? "MATCH" : "NO MATCH");
	
	return result;
}


bool
IsTypeMatch(const BMessage &test, const entry_ref &ref)
{
	BString value;
	if (test.FindString("value",&value) != B_OK)
	{
		debugger("Couldn't get value in IsTypeMatch");
		return false;
	}

//if (value == "image/")
//	debugger("");
	
	BString compare;
	if (test.FindString("mode",&compare) != B_OK)
	{
		debugger("Couldn't get mode in IsTypeMatch");
		return false;
	}
	
	BString string;
	attr_info info;
	BNode node(&ref);
	if (node.InitCheck() != B_OK)
		return false;
	
	if (node.GetAttrInfo("BEOS:TYPE",&info) != B_OK)
	{
		BPath path(&ref);
		if (update_mime_info(path.Path(),0,1,0) != B_OK)
			return false;
	}
	
	if (node.ReadAttrString("BEOS:TYPE",&string) != B_OK)
		return false;
	
	
	bool result = StringCompare(value,string.String(),compare.String(),true);
	
	printf("\tType test: %s %s %s - %s\n",ref.name,compare.String(),value.String(),
								result ? "MATCH" : "NO MATCH");
	
	return result;
}


bool
IsSizeMatch(const BMessage &test, const entry_ref &ref)
{
	BString value;
	if (test.FindString("value",&value) != B_OK)
	{
		debugger("Couldn't get value in IsTypeMatch");
		return false;
	}
	
	BString compare;
	if (test.FindString("mode",&compare) != B_OK)
	{
		debugger("Couldn't get mode in IsTypeMatch");
		return false;
	}
	
	off_t fromsize = atoll(value.String());
	
	BFile file(&ref,B_READ_ONLY);
	if (file.InitCheck() != B_OK)
		return false;
	
	off_t tosize;
	file.GetSize(&tosize);
	file.Unset();
	
	bool result = false;
	
	if (strcmp(compare.String(),"is") == 0)
		result = (fromsize == tosize);
	else if (strcmp(compare.String(),"is not") == 0)
		result = (fromsize != tosize);
	else if (strcmp(compare.String(),"is more than") == 0)
		result = (fromsize > tosize);
	else if (strcmp(compare.String(),"is less than") == 0)
		result = (fromsize < tosize);
	else if (strcmp(compare.String(),"is at least") == 0)
		result = (fromsize >= tosize);
	else if (strcmp(compare.String(),"is at most") == 0)
		result = (fromsize <= tosize);
	
	printf("\tSize test: %s %s %lld - %s\n",ref.name,compare.String(),tosize,
								result ? "MATCH" : "NO MATCH");
	
	return result;
}


bool
IsLocationMatch(const BMessage &test, const entry_ref &ref)
{
	BString value;
	if (test.FindString("value",&value) != B_OK)
	{
		debugger("Couldn't get value in IsLocationMatch");
		return false;
	}
	
	BString compare;
	if (test.FindString("mode",&compare) != B_OK)
	{
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
	
	bool result = StringCompare(value,filepath.String(),compare.String(),true);
	
	printf("\tLocation test: %s %s %s - %s\n",filepath.String(),
								compare.String(),value.String(),
								result ? "MATCH" : "NO MATCH");
	
	return result;
}


bool
IsModifiedMatch(const BMessage &test, const entry_ref &ref)
{
	// TODO: Implement using Mr. Peeps! date-parsing code
	return false;
}


bool
IsAttributeMatch(const BMessage &test, const entry_ref &ref)
{
	BString value;
	if (test.FindString("value",&value) != B_OK)
	{
		debugger("Couldn't get value in IsTypeMatch");
		return false;
	}
	
	BString compare;
	if (test.FindString("mode",&compare) != B_OK)
	{
		debugger("Couldn't get mode in IsTypeMatch");
		return false;
	}
	
	BString attribute;
	if (test.FindString("attrtype",&attribute) != B_OK)
	{
		debugger("Couldn't get attribute in IsAttributeMatch");
		return false;
	}
	
	BString string;
	attr_info info;
	BNode node(&ref);
	if (node.InitCheck() != B_OK)
		return false;
	
	if (node.GetAttrInfo(attribute.String(),&info) != B_OK)
		return false;
	
	if (node.ReadAttrString(attribute.String(),&string) != B_OK)
		return false;
	
	
	bool result = StringCompare(value,string,compare.String(),true);
	
	
	BString attrname;
	if (test.FindString("attrname",&attrname) != B_OK)
		attrname = attribute;
	
	printf("\tAttribute test: %s %s %s - %s\n",attrname.String(),
								compare.String(),value.String(),
								result ? "MATCH" : "NO MATCH");
	
	return result;
}


bool
StringCompare(const BString &from, const BString &to, const char *mode,
			const bool &match_case)
{
	if (!mode)
	{
		debugger("NULL mode in StringCompare");
		return false;
	}
	
	if (strcmp(mode,"is") == 0)
		if (match_case)
			return from.Compare(to) == 0;
		else
			return from.ICompare(to) == 0;
	else if (strcmp(mode,"is not") == 0)
		if (match_case)
			return from.Compare(to) != 0;
		else
			return from.ICompare(to) != 0;
	else if (strcmp(mode,"contains") == 0)
		if (match_case)
			return to.FindFirst(from) >= 0;
		else
			return to.IFindFirst(from) >= 0;
	else if (strcmp(mode,"does not contain") == 0)
		if (match_case)
			return to.FindFirst(from) < 0;
		else
			return to.IFindFirst(from) < 0;
	else if (strcmp(mode,"starts with") == 0)
		if (match_case)
			return to.FindFirst(from) == 0;
		else
			return to.IFindFirst(from) == 0;
	else if (strcmp(mode,"ends with") == 0)
	{
		int32 pos;
		if (match_case)
			pos = to.FindLast(from);
		else
			pos = to.IFindLast(from);
		
		return (to.CountChars() - from.CountChars() == pos);
	}	
	
	return false;
}


status_t
MoveAction(const BMessage &action, entry_ref &ref)
{
	BString value;
	status_t status;
	status = action.FindString("value",&value);
	if (status != B_OK)
		return status;
	value = ProcessPatterns(value.String(),ref);
	
	BEntry entry(value.String(),true);
	status = entry.InitCheck();
	if (status != B_OK || (entry.Exists() && !entry.IsDirectory()))
		return B_ERROR;
	
	if (!entry.Exists())
		create_directory(value.String(),0777);
	
	BEntry source(&ref);
	status = source.InitCheck();
	if (status != B_OK)
		return B_ERROR;
	
	status = MoveFile(&source,&entry,false);
	if (status == B_OK)
	{
		printf("\tMoved %s to %s\n",ref.name,value.String());
		source.GetRef(&ref);
	}
	else
	{
		printf("\tCouldn't move %s to %s. Stopping here.\n\t\tError Message: %s\n",ref.name,
				value.String(),strerror(status));
	}
	
	return B_OK;
}


status_t
CopyAction(const BMessage &action, entry_ref &ref)
{
	BString value;
	status_t status;
	status = action.FindString("value",&value);
	if (status != B_OK)
		return status;
	value = ProcessPatterns(value.String(),ref);
	
	BEntry entry(value.String(),true);
	status = entry.InitCheck();
	if (status != B_OK || (entry.Exists() && !entry.IsDirectory()))
		return B_ERROR;
	
	if (!entry.Exists())
		create_directory(value.String(),0777);
	
	BEntry source(&ref);
	status = source.InitCheck();
	if (status != B_OK)
		return B_ERROR;
	
	status = CopyFile(&source,&entry,false);
	if (status == B_OK)
	{
		printf("\tCopied %s to %s\n",ref.name,value.String());
		source.GetRef(&ref);
	}
	else
	{
		printf("\tCouldn't copy %s to %s. Stopping here.\n\t\tError Message: %s\n",ref.name,
				value.String(),strerror(status));
	}
	
	
	return B_OK;
}


status_t
RenameAction(const BMessage &action, entry_ref &ref)
{
	BString value;
	status_t status;
	status = action.FindString("value",&value);
	if (status != B_OK)
		return status;
	value = ProcessPatterns(value.String(),ref);
	
	BEntry entry(value.String(),true);
	status = entry.InitCheck();
	if (status != B_OK || entry.Exists())
		return B_ERROR;
	
	BEntry source(&ref);
	status = source.InitCheck();
	if (status != B_OK)
		return B_ERROR;
	
	status = source.Rename(value.String());
	if (status == B_OK)
	{
		printf("\tRenamed %s to %s\n",ref.name,value.String());
		source.GetRef(&ref);
	}
	else
	{
		printf("\tCouldn't rename %s to %s. Stopping here.\n\t\tError Message: %s\n",ref.name,
				value.String(),strerror(status));
	}
	
	if (status == B_OK)
		source.GetRef(&ref);
	
	return B_OK;
}


status_t
OpenAction(const BMessage &action, entry_ref &ref)
{
	entry_ref app;
	BString appName("");
	if (be_roster->FindApp(&ref,&app) == B_OK)
		appName = app.name;
	
	status_t status = be_roster->Launch(&ref);
	
	if (status == B_OK)
		printf("\tOpened %s in program %s\n",ref.name,appName.String());
	else
	{
		// R5 (and probably others) don't seem to want to open folders in Tracker --
		// FindApp() returns B_OK, but sets the entry_ref of the app to open it to
		// the folder's ref, which is dumb. This works around this apparent stupidity.
		BString typestr;
		if (BNode(&ref).ReadAttrString("BEOS:TYPE",&typestr) == B_OK &&
			typestr.Compare("application/x-vnd.Be-directory") == 0)
		{
			BMessage *msg = new BMessage(B_REFS_RECEIVED);
			msg->AddRef("refs",&ref);
			be_roster->Launch("application/x-vnd.Be-TRAK",msg);
			printf("\tOpened %s in program Tracker\n",ref.name);
			return B_OK;
		}
		if (appName.CountChars() > 0)
			printf("\tCouldn't open %s in program %s\n",ref.name,appName.String());
		else
			printf("\tCouldn't open %s -- the system couldn't find a program to do it.\n",ref.name);
	}
	
	return status;
}


status_t
ArchiveAction(const BMessage &action, entry_ref &ref)
{
	BString value;
	status_t status;
	status = action.FindString("value",&value);
	if (status != B_OK)
		return status;
	value = ProcessPatterns(value.String(),ref);
	
	BPath path(&ref);
	BString parentstr = path.Path();
	parentstr.ReplaceLast(path.Leaf(),"");
	
	BString command = "";
	command << "cd '" << parentstr << "'; zip -9 -u -r '" << value << "' '"
			<< path.Leaf() << "'";
	
	int result = system(command.String());
	if (result)
		printf("\tCouldn't create archive %s\n\t\tError code: %d\n", value.String(),result);
	else
		printf("\tAdded %s to Archive %s\n",ref.name,value.String());	
	
	return B_OK;
}


status_t
CommandAction(const BMessage &action, entry_ref &ref)
{
	BString value;
	status_t status;
	status = action.FindString("value",&value);
	if (status != B_OK)
		return status;
	value = ProcessPatterns(value.String(),ref);
	
	int result = system(value.String());
	if (result)
	{
		printf("\tTerminal Command: %s\n\t\tPossible error: command returned %d\n",
				value.String(),result);
	}
	else
		printf("\tTerminal Command: %s\n",value.String());
	
	return B_OK;
}


status_t
TrashAction(const BMessage &action, entry_ref &ref)
{
	BEntry entry("/boot/home/Desktop/Trash",true);
	status_t status = entry.InitCheck();
	if (status != B_OK || !entry.Exists() || !entry.IsDirectory())
		return B_ERROR;
	
	BEntry source(&ref);
	status = source.InitCheck();
	if (status != B_OK)
		return B_ERROR;
	
	status = MoveFile(&source,&entry,false);
	if (status == B_OK)
	{
		printf("\tMoved %s to the Trash\n",ref.name);
		source.GetRef(&ref);
	}
	else
	{
		printf("\tCouldn't move %s to the Trash. Stopping here.\n\t\tError Message: %s\n",
				ref.name,strerror(status));
	}
	
	return B_OK;
}


status_t
DeleteAction(const BMessage &action, entry_ref &ref)
{
	BEntry entry(&ref);
	
	status_t status = entry.Remove();
	if (status == B_OK)
		printf("\tDeleted %s\n",BPath(ref.name).Path());
	else
	{
		printf("\tCouldn't delete %s. Stopping here.\n\t\tError Message: %s\n",
				BPath(ref.name).Path(),strerror(status));
	}
	
	return entry.Remove();
}


status_t
SaveRules(const char *path, BObjectList<FilerRule> *ruleList)
{
	BEntry entry("/boot/home/config/settings/FilerRules");
	if (entry.Exists())
		entry.Remove();
	
	CppSQLite3DB db;
	db.open("/boot/home/config/settings/FilerRules");
	
	// While we could use other means of obtaining table names, this table is also
	// used for maintaining the order of the rules, which must be preserved
	DBCommand(db,"create table RuleList (ruleid int primary key, name varchar);",
				"PrefsWindow::SaveRules");
	
	BString command;
	
	for (int32 i = 0; i < ruleList->CountItems(); i++)
	{
		FilerRule *rule = ruleList->ItemAt(i);
		
		// Test table:
		// 0) Entry type (test vs action)
		// 1) type
		// 2) mode
		// 3) value
		// 4) attribute type (if Attribute test)
		// 5) attribute type public name (short description)
		// 6) attribute public name (if Attribute test)
		
		BString tablename(EscapeIllegalCharacters(rule->GetDescription()));
		
		command = "create table ";
		command << tablename 
			<< "(entrytype varchar, testtype varchar, testmode varchar, testvalue varchar,
				attrtype varchar, attrtypename varchar, attrpublicname varchar);";
		DBCommand(db,command.String(), "PrefsWindow::SaveRules");
		
		command = "insert into RuleList values(";
		command << i << ",'" << tablename << "');";
		DBCommand(db,command.String(), "PrefsWindow::SaveRules");
		
		for (int32 j = 0; j < rule->CountTests(); j++)
		{
			BMessage *test = rule->TestAt(j);
			if (!test)
				continue;
			
			BString name,mode,value,mimeType,typeName, attrType, attrName;
			test->FindString("name",&name);
			test->FindString("mode",&mode);
			test->FindString("value",&value);
			test->FindString("mimetype",&mimeType);
			test->FindString("typename",&typeName);
			test->FindString("attrtype",&attrType);
			test->FindString("attrname",&attrName);
			
			command = "insert into ";
			command << tablename << " values('test', '" << EscapeIllegalCharacters(name.String()) 
					<< "', '" << EscapeIllegalCharacters(mode.String())
					<< "', '" << EscapeIllegalCharacters(value.String())
					<< "', '" << EscapeIllegalCharacters(mimeType.String())
					<< "', '" << EscapeIllegalCharacters(typeName.String())
					<< "', '" << EscapeIllegalCharacters(attrName.String())
					<< "');";
			
			DBCommand(db,command.String(),"PrefsWindow::SaveRules:save test");
		}
		
		for (int32 j = 0; j < rule->CountActions(); j++)
		{
			BMessage *action = rule->ActionAt(j);
			if (!action)
				continue;
			
			BString name,value;
			action->FindString("name",&name);
			action->FindString("value",&value);
			
			command = "insert into ";
			command << tablename << " values('action', '" << EscapeIllegalCharacters(name.String()) 
					<< "', '"
					<< "', '" << EscapeIllegalCharacters(value.String())
					<< "', '', '', '');";
			DBCommand(db,command.String(),"PrefsWindow::SaveRules:save action");
		}
	}
	
	
	db.close();
	
	return B_OK;
}


status_t
LoadRules(const char *path, BObjectList<FilerRule> *ruleList)
{
	BEntry entry("/boot/home/config/settings/FilerRules");
	if (!entry.Exists())
		return B_OK;
	
	CppSQLite3DB db;
	db.open("/boot/home/config/settings/FilerRules");
	
	// Because this particular build of sqlite3 does not support multithreading
	// because of lack of pthreads support, we need to do this in a slightly different order
	
	CppSQLite3Query query;
	query = DBQuery(db,"select name from RuleList order by ruleid;","PrefsWindow::LoadRules");
	
	BString command;
	while (!query.eof())
	{
		BString rulename = query.getStringField((int)0);
		
		FilerRule *rule = new FilerRule;
		rule->SetDescription(DeescapeIllegalCharacters(rulename.String()).String());
		
		ruleList->AddItem(rule);
		
		query.nextRow();
	}
	
	query.finalize();
	
	for (int32 i = 0; i < ruleList->CountItems(); i++)
	{
		FilerRule *rule = ruleList->ItemAt(i);
		
		if (!rule)
			continue;
		
		BString rulename(EscapeIllegalCharacters(rule->GetDescription()));
		
		// Now comes the fun(?) part: loading the tests and actions. Joy. :/
		command = "select * from ";
		command << rulename << " where entrytype = 'test';";
		query = DBQuery(db,command.String(),"PrefsWindow::LoadRules");
		
		while (!query.eof())
		{
			BString classname = DeescapeIllegalCharacters(query.getStringField(1));
			BMessage *test = new BMessage;
			
			test->AddString("name",classname);
			
			if (classname.ICompare("Attribute") == 0)
			{
				test->AddString("mimetype",DeescapeIllegalCharacters(query.getStringField(4)));
				test->AddString("typename",DeescapeIllegalCharacters(query.getStringField(5)));
				test->AddString("attrname",DeescapeIllegalCharacters(query.getStringField(6)));
			}
			
			test->AddString("mode",DeescapeIllegalCharacters(query.getStringField(2)).String());
			test->AddString("value",DeescapeIllegalCharacters(query.getStringField(3)).String());
			
			rule->AddTest(test);
			
			query.nextRow();
		}
		query.finalize();
		
		command = "select * from ";
		command << rulename << " where entrytype = 'action';";
		query = DBQuery(db,command.String(),"PrefsWindow::LoadRules");
		
		while (!query.eof())
		{
			BMessage *action = new BMessage;
			
			action->AddString("name",DeescapeIllegalCharacters(query.getStringField(1)));
			action->AddString("value",DeescapeIllegalCharacters(query.getStringField(3)));
			
			rule->AddAction(action);
			
			query.nextRow();
		}
		query.finalize();
		
	}
	
	db.close();
	return B_OK;
}

BMessage *
MakeTest(const char *name,const char *mode, const char *value, const char *mimeType,
		const char *typeName, const char *attrType, const char *attrName)
{
	BMessage *msg = new BMessage;
	msg->AddString("name",name);
	msg->AddString("mode",mode);
	msg->AddString("value",value);
	
	if (typeName || mimeType || attrType || attrName)
	{
		if(!(typeName && mimeType && attrType && attrName))
			debugger("The last 4 parameters must all be either NULL or non-NULL as a group");
		
		msg->AddString("typename",typeName);
		msg->AddString("mimetype",mimeType);
		msg->AddString("attrtype",attrType);
		msg->AddString("attrname",attrName);
	}
	return msg;
}


BMessage *
MakeAction(const char *name,const char *value)
{
	BMessage *msg = new BMessage;
	msg->AddString("name",name);
	msg->AddString("value",value);
	
	return msg;
}
