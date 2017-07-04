/*
	TestView.cpp: view to display and edit settings for Filer tests
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
	Contributed by:
		Pete Goodeve, 2016
		Owen Pan <owen.pan@yahoo.com>, 2017
*/

#include "TestView.h"

#include <LayoutBuilder.h>
#include <Mime.h>

#include "FilerDefs.h"
#include "ModeMenu.h"
#include "RuleRunner.h"

extern BMessage gArchivedTypeMenu;


//#define USE_TRACE

#ifdef USE_TRACE
#define STRACE(x) printf(x)
#define MSGTRACE(x) x->PrintToStream();
#else
#define STRACE(x) /* */
#define MSGTRACE(x) /* */
#endif


TestView::TestView(const char* name, BMessage* test, const int32& flags)
	:
	BView(name, flags),
	fTest(NULL),
	fDataType(TEST_TYPE_NULL)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	RuleRunner::GetTestTypes(fTestTypes);

	fTestField = new BMenuField(NULL, TestMenu());

	BMessage modes;
	RuleRunner::GetModes(modes);

	fModeField = new BMenuField(NULL,
		new ModeMenu(fTestField->MenuItem(), this));

	fValueBox = new AutoTextControl("valuebox", NULL, NULL, new BMessage());
	fValueBox->SetDivider(0);

	fAddRemoveButtons = new AddRemoveButtons(MSG_ADD_TEST, MSG_REMOVE_TEST,
		this);

	BLayoutBuilder::Group<>(this, B_HORIZONTAL, B_USE_HALF_ITEM_SPACING)
		.Add(fTestField, 0)
		.Add(fModeField, 0)
		.Add(fValueBox)
		.Add(fAddRemoveButtons)
		.End();

	bool usedefaults = false;
	int8 modetype;
	int8 type;
	if (test) {
		STRACE(("\nTestView::TestView: test parameter\n"));
		MSGTRACE(test);

		fTest = new BMessage(*test);

		if (!SetTest(fTest))
			usedefaults = true;

		if (fTest->FindInt8("mode", &modetype) == B_OK)
			SetMode(modetype);
		else {
			fTest->FindInt8("name" ,&type);
			modes.MakeEmpty();
			RuleRunner::GetCompatibleModes(type, modes);
			modes.FindInt8("modes", 0, &modetype);
			SetMode(modetype);
		}

		BString str;
		if (fTest->FindString("value", &str) == B_OK)
			fValueBox->SetText(str.String());
	} else
		usedefaults = true;

	if (usedefaults) {
		if (!fTest)
			fTest = new BMessage;

		fTestTypes.FindInt8("tests", 0, &type);

		BMessage newtest;
		newtest.AddInt8("name", type);

		modes.MakeEmpty();
		RuleRunner::GetCompatibleModes(type, modes);
		modes.FindInt8("modes", 0, &modetype);

		newtest.AddInt8("mode", modetype);
		newtest.AddString("value", "");

		SetTest(&newtest);
		SetMode(modetype);
	}
}


TestView::~TestView()
{
	delete fTestField;
	delete fModeField;
	delete fValueBox;
	delete fAddRemoveButtons;
	delete fTest;
}


void
TestView::AttachedToWindow()
{
	BMenu* menu = fTestField->Menu();
	menu->SetTargetForItems(this);

	for (int32 i = 0; i < menu->CountItems(); i++)
	{
		BMenuItem* item = menu->ItemAt(i);
		if (item->Submenu())
			item->Submenu()->SetTargetForItems(this);
	}

	fValueBox->SetTarget(this);
}


void
TestView::MessageReceived(BMessage* msg)
{
	switch (msg->what)
	{
		case MSG_TEST_CHOSEN:
		{
			SetTest(msg);
			break;
		}
		case MSG_MODE_CHOSEN:
		{
			int8 modetype;
			if (msg->FindInt8("mode", &modetype) == B_OK)
				SetMode(modetype);
			break;
		}
		default:
		{
			BView::MessageReceived(msg);
			break;
		}
	}

}


BMessage*
TestView::GetTest() const
{
	BString str;
	if (fTest->FindString("value", &str) == B_OK)
		fTest->ReplaceString("value", fValueBox->Text());
	else
		fTest->AddString("value", fValueBox->Text());

	return fTest;
}


BPopUpMenu*
TestView::TestMenu() const
{
	// These ones will always exist. Type is the default because it's probably
	// going to be the one most used
	BMessage* msg;
	BPopUpMenu* menu = new BPopUpMenu("", true, false);

	// Read in the types in the MIME database which have extra attributes
	// associated with them

	BMimeType mime;
	BMessage types, info, attr;
	BString string;

	BMimeType::GetInstalledTypes(&types);

	int32 index = 0;
	while (types.FindString("types", index, &string) == B_OK)
	{
		index++;
		mime.SetTo(string.String());
		if (mime.GetAttrInfo(&info) != B_OK)
			continue;

		int32 infoindex = 0;
		BString attrName;
		BString attrPublicName;
		int32 attrType;

		char attrTypeName[B_MIME_TYPE_LENGTH];
		mime.GetShortDescription(attrTypeName);

		while (info.FindString("attr:name", infoindex, &attrName) == B_OK)
		{
			// This is where we create tests based on a particular type's
			// "special" attributes
			
			// Just string attributes are supported for now
			if (info.FindInt32("attr:type", infoindex, &attrType) != B_OK
				|| attrType != B_STRING_TYPE
				|| info.FindString("attr:public_name", infoindex,
				&attrPublicName) != B_OK) {
					infoindex++;
					continue;
			}

			BMenu* submenu = GetMenu(menu, attrTypeName);
			if (!submenu)
				submenu = AddMenuSorted(menu, attrTypeName);

			msg = new BMessage(MSG_TEST_CHOSEN);
			msg->AddInt8("name", AttributeTestType());
			msg->AddString("attrtype", attrName);
			msg->AddString("attrname", attrPublicName);
			msg->AddString("mimetype", string);
			msg->AddString("typename", attrTypeName);
			submenu->AddItem(new BMenuItem(attrPublicName.String(), msg));

			infoindex++;
		}
	}

	menu->AddItem(new BSeparatorItem(), 0);

	// All this weirdness is to have the "standard"	tests at the top and
	// the attribute tests at the bottom with a separator in between
	int8 type;
	int32 i = 0;
	while (fTestTypes.FindInt8("tests", i, &type) == B_OK)
		i++;

	i--;

	while (i >= 0)
	{
		fTestTypes.FindInt8("tests", i, &type);
		msg = new BMessage(MSG_TEST_CHOSEN);
		msg->AddInt8("name", type);
		menu->AddItem(new BMenuItem(sTestTypes[type].locale, msg), 0);
		i--;
	}

	return menu;
}


BMenu*
TestView::AddMenuSorted(BMenu* parent, const char* name) const
{
	if (!name)
		return NULL;

	BMenu* menu = new BMenu(name);

	for (int32 i = 0; i < parent->CountItems(); i++)
	{
		BMenuItem* item = parent->ItemAt(i);

		if (strcmp(item->Label(), name) > 0) {
//		printf("INSERT: %s is after %s\n", name, item->Label());
			parent->AddItem(menu, i);
			return menu;
		}
	}

//	if (parent->CountItems())
//		printf("%s is after %s\n", name, parent->ItemAt(parent->CountItems()
//			- 1)->Label());
//	else
//		printf("%s is last\n",name);

	parent->AddItem(menu);
	return menu;
}


BMenu*
TestView::GetMenu(BMenu* parent, const char* name) const
{
	// This is because FindMenu recursively searches a menu. We just want to
	// check the top level of fTestMenu

	if (!name)
		return NULL;

	for (int32 i = 0; i < parent->CountItems(); i++)
	{
		BMenuItem* item = parent->ItemAt(i);

		if (!item->Submenu())
			continue;

		if (strcmp(item->Label(), name) == 0)
			return item->Submenu();
	}

	return NULL;
}


bool
TestView::SetTest(BMessage* msg)
{
	STRACE(("\nTestView::SetTest\n"));
	if (!msg)
		return false;

	MSGTRACE(msg);

	// The easy way to update fTest is just copy the whole thing and update
	// the mode and value from the controls. This saves some conditionals when
	// dealing with attribute tests. The fields sent by the menu items (and passed
	// to this function) are the exact same as what is needed by RuleRunner's test
	// code.
	// There is one catch, however. The message passed here will NOT have the mode
	// or value, so we need to save them and copy them over

	int8 modetype;
	BString str;

	fTest->FindInt8("mode", &modetype);

	if (fTest != msg) {
		BString value;

		fTest->FindString("value", &value);
		*fTest = *msg;

		fTest->what = 0;

		int8 tmpType;
		if (fTest->FindInt8("mode", &tmpType) != B_OK)
			fTest->AddInt8("mode", modetype);

		if (fTest->FindString("value", &str) != B_OK)
			fTest->AddString("value", value);
	}

	BString label;

	int8 type;
	fTest->FindInt8("name", &type);
	if (type == AttributeTestType()) {
		BString str;
		fTest->FindString("typename", &str);
		label = str;
		fTest->FindString("attrname", &str);
		label << " : " << str;
	} else
		label = sTestTypes[type].locale;

	fDataType = RuleRunner::GetDataTypeForTest(type);
	fTestField->MenuItem()->SetLabel(label.String());

	// Now that the test button has been updated, make sure that the mode currently
	// set is supported by the current test
	int32 datatype = RuleRunner::GetDataTypeForMode(modetype);
	if (fDataType != datatype && datatype != TEST_TYPE_ANY) {
		STRACE(("Modes not compatible, refreshing.\n"));
		// Not compatible, so reset the mode to something compatible
		BMessage modes;
		RuleRunner::GetCompatibleModes(fDataType, modes);

		modes.FindInt8("modes", 0, &modetype);
		SetMode(modetype);
	}
	STRACE(("-------------------------\n"));

	return true;
}


void
TestView::SetMode(int8 modetype)
{
	if (modetype < 0)
		return;

	// This function assumes that the string passed to it is valid
	// for the test type
	int8 tmpType;
	if (fTest->FindInt8("mode", &tmpType) == B_OK)
		fTest->ReplaceInt8("mode", modetype);
	else
		fTest->AddInt8("mode", modetype);
	fModeField->MenuItem()->SetLabel(sModeTypes[modetype].locale);
}


const char*
TestView::GetValue()
{
	// Exists for future expansion when different controls are associated with
	// different tests
	return fValueBox->Text();
}
