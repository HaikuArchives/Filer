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
#include "main.h"
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
	fDecimalMark(static_cast<App*>(be_app)->GetDecimalMark())
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	RuleRunner::GetTestTypes(fTestTypes);

	fTestField = new BMenuField(NULL, TestMenu());

	BMessage modes;
	RuleRunner::GetModes(modes);

	fModeField = new BMenuField(NULL,
		new ModeMenu(fTestField->MenuItem(), this));

	BPopUpMenu* menu = new BPopUpMenu("");
	for (int32 i = 0; i < nSizeUnits; i++) {
		BMessage* msg = new BMessage(MSG_UNIT_CHOSEN);
		msg->AddInt8("unit", i);
		menu->AddItem(new BMenuItem(sSizeUnits[i], msg));
	}
	fUnitField = new BMenuField(NULL, menu);

	fValueBox = new AutoTextControl("valuebox", NULL, NULL, new BMessage());
	fValueBox->SetDivider(0);

	fAddRemoveButtons = new AddRemoveButtons(MSG_ADD_TEST, MSG_REMOVE_TEST,
		this, fValueBox->Bounds().Height());

	BLayoutBuilder::Group<>(this, B_HORIZONTAL, B_USE_HALF_ITEM_SPACING)
		.Add(fTestField, 0)
		.Add(fModeField, 0)
		.AddGroup(B_HORIZONTAL, 0)
			.Add(fValueBox)
			.Add(fUnitField, 0)
			.End()
		.AddStrut(0)
		.Add(fAddRemoveButtons)
		.End();

	if (test != NULL) {
		STRACE(("\nTestView::TestView: test parameter\n"));
		MSGTRACE(test);

		test->FindInt8("name" ,&fType);
		FindAttribute(test);
		fDataType = GetDataTypeForTest(fType);

		test->FindInt8("mode", &fMode);
		test->FindInt8("unit", &fUnit);

		BString str;
		if (test->FindString("value", &str) == B_OK) {
			if (fDataType == TEST_TYPE_NUMBER && fUnit > 0
				&& fDecimalMark != '.')
				str.ReplaceAll('.', fDecimalMark);

			fValueBox->SetText(str.String());
		}
	} else {
		fTestTypes.FindInt8("tests", 0, &fType);

		modes.MakeEmpty();
		RuleRunner::GetCompatibleModes(fType, modes);
		modes.FindInt8("modes", 0, &fMode);

		fUnit = 0;
	}

	SetMode();
	SetTest();
}


TestView::~TestView()
{
	delete fTestField;
	delete fModeField;
	delete fValueBox;
	delete fUnitField;
	delete fAddRemoveButtons;
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

	fUnitField->Menu()->SetTargetForItems(this);
	fValueBox->SetTarget(this);
}


void
TestView::MessageReceived(BMessage* msg)
{
	switch (msg->what)
	{
		case MSG_TEST_CHOSEN:
		{
			int8 type;
			if (msg->FindInt8("name", &type) == B_OK) {
				fType = type;
				FindAttribute(msg);
				SetTest();
			}
			break;
		}
		case MSG_MODE_CHOSEN:
		{
			int8 mode;
			if (msg->FindInt8("mode", &mode) == B_OK && fMode != mode) {
				fMode = mode;
				SetMode();
			}
			break;
		}
		case MSG_UNIT_CHOSEN:
		{
			int8 unit;
			if (msg->FindInt8("unit", &unit) == B_OK && fUnit != unit) {
				fUnit = unit;
				SetUnit();
			}
			break;
		}
		default:
			BView::MessageReceived(msg);
	}
}


static void
addAttribute(BMessage* msg, const BString& attrtype, const BString& attrname,
	const BString& mimetype, const BString& mimename)
{
	msg->AddString("attrtype", attrtype);
	msg->AddString("attrname", attrname);
	msg->AddString("mimetype", mimetype);
	msg->AddString("typename", mimename);
}


BMessage*
TestView::GetTest() const
{
	BMessage* test = new BMessage;
	BString str(fValueBox->Text());

	str.Trim();
	if (fDataType == TEST_TYPE_NUMBER && fUnit > 0 && fDecimalMark != '.')
		str.ReplaceAll(fDecimalMark, '.');

	test->AddInt8("name", fType);
	test->AddInt8("mode", fMode);
	test->AddInt8("unit", fUnit);
	test->AddString("value", str.Trim());

	if (fType == AttributeTestType())
		addAttribute(test, fAttrType, fAttrName, fMimeType, fTypeName);

	return test;
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
			addAttribute(msg, attrName, attrPublicName, string, attrTypeName);
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


void
TestView::SetTest()
{
	STRACE(("\nTestView::SetTest\n"));

	BString label;
	if (fType == AttributeTestType())
		label << fTypeName << " : " << fAttrName;
	else
		label = sTestTypes[fType].locale;

	fTestField->MenuItem()->SetLabel(label.String());
	fDataType = GetDataTypeForTest(fType);

	// Now that the test button has been updated, make sure that the mode currently
	// set is supported by the current test
	int32 datatype = GetDataTypeForMode(fMode);
	if (datatype != fDataType && datatype != TEST_TYPE_ANY) {
		STRACE(("Modes not compatible, refreshing.\n"));
		// Not compatible, so reset the mode to something compatible
		BMessage modes;
		RuleRunner::GetCompatibleModes(fDataType, modes);

		modes.FindInt8("modes", 0, &fMode);
		SetMode();
	}
	STRACE(("-------------------------\n"));

	if (fDataType == TEST_TYPE_NUMBER) {
		fValueBox->OnlyAllowDigits(true);
		SetUnit();

		if (fUnitField->IsHidden())
			fUnitField->Show();
	} else {
		fValueBox->OnlyAllowDigits(false);

		if (!fUnitField->IsHidden())
			fUnitField->Hide();
	}
}


void
TestView::SetMode()
{
	fModeField->MenuItem()->SetLabel(sModeTypes[fMode].locale);
}


void
TestView::SetUnit()
{
	fUnitField->MenuItem()->SetLabel(sSizeUnits[fUnit]);

	BTextView* view = fValueBox->TextView();

	if (fUnit > 0)
		view->AllowChar(fDecimalMark);
	else
		view->DisallowChar(fDecimalMark);
}


void TestView::ResetUnit()
{
	if (fUnit != 0) {
		fUnit = 0;
		SetUnit();
	}
}


void
TestView::FindAttribute(const BMessage* msg)
{
	if (fType == AttributeTestType()) {
		msg->FindString("attrtype", &fAttrType);
		msg->FindString("attrname", &fAttrName);
		msg->FindString("mimetype", &fMimeType);
		msg->FindString("typename", &fTypeName);
	}
}


const char*
TestView::GetValue()
{
	// Exists for future expansion when different controls are associated with
	// different tests
	return fValueBox->Text();
}
