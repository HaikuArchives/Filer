/*
	TestView.cpp: view to display and edit settings for Filer tests
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
	Contributed by: Pete Goodeve, 2016
*/

#include <Font.h>
#include <ListItem.h>
#include <Mime.h>
#include <ScrollBar.h>

#include "AutoTextControl.h"
#include "FilerDefs.h"
#include "RuleRunner.h"
#include "TestView.h"

extern BMessage gArchivedTypeMenu;


//#define USE_TRACE

#ifdef USE_TRACE
#define STRACE(x) printf(x)
#define MSGTRACE(x) x->PrintToStream();
#else
#define STRACE(x) /* */
#define MSGTRACE(x) /* */
#endif


TestView::TestView(const BRect& frame, const char* name, BMessage* test,
	const int32& resize, const int32& flags)
	:
	BView(frame, name, resize, flags),
 	fTest(NULL)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	// Find the longest name in all the tests
	RuleRunner::GetTestTypes(fTestTypes);
	int32 i = 0;
	BString teststr, widesttest, widestmode;
	while (fTestTypes.FindString("tests", i, &teststr) == B_OK)
	{
		i++;
		if (teststr.CountChars() > widesttest.CountChars())
			widesttest = teststr;
	}

	// This will hopefully accomodate some of the attribute strings
	teststr = "Netpositive Password";
	if (teststr.CountChars() > widesttest.CountChars())
		widesttest = teststr;

	fTestButton = new BButton(BRect(0, 0, 1, 1), "testbutton",
		widesttest.String(), new BMessage(MSG_SHOW_TEST_MENU));
	fTestButton->ResizeToPreferred();
	AddChild(fTestButton);

	BRect rect = fTestButton->Frame();
	rect.OffsetBy(rect.Width() + 10.0, 0.0);

	// Find the longest name in all the modes
	BMessage modes;
	RuleRunner::GetModes(modes);
	i = 0;
	while (modes.FindString("modes", i, &teststr) == B_OK)
	{
		i++;
		if (teststr.CountChars() > widestmode.CountChars())
			widestmode = teststr;
	}

	fModeButton = new BButton(rect, "modebutton", widestmode.String(),
		new BMessage(MSG_SHOW_MODE_MENU));
	fModeButton->ResizeToPreferred();
	AddChild(fModeButton);

	rect = fModeButton->Frame();
	rect.OffsetBy(rect.Width() + 5, 0);
	rect.right = rect.left + StringWidth("application/x-vnd.dw-foo") + 5;
	fValueBox = new AutoTextControl(rect, "valuebox", NULL, NULL,
		new BMessage(), B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
	AddChild(fValueBox);
	fValueBox->SetDivider(0);
	if (fValueBox->Bounds().Height() < fModeButton->Bounds().Height()) {
		fValueBox->MoveBy(0.0,(fModeButton->Bounds().Height() 
		- fValueBox->Bounds().Height()) / 2.0);
	}

	SetupTestMenu();

	bool usedefaults = false;
	if (test) {
		STRACE(("\nTestView::TestView: test parameter\n"));
		MSGTRACE(test);

		fTest = new BMessage(*test);
		BString str;

		if (!SetTest(fTest))
			usedefaults = true;

		if (fTest->FindString("mode", &str) == B_OK)
			SetMode(str.String());
		else {
			fTest->FindString("name" ,&str);
			modes.MakeEmpty();
			RuleRunner::GetCompatibleModes(str.String(), modes);
			modes.FindString("modes", 0, &str);
			SetMode(str.String());
		}

		if (fTest->FindString("value", &str) == B_OK)
			fValueBox->SetText(str.String());
	} else
		usedefaults = true;

	if (usedefaults) {
		if (!fTest)
			fTest = new BMessage;

		BString str;
		fTestTypes.FindString("tests", 0, &str);

		BMessage newtest;
		newtest.AddString("name", str);

		modes.MakeEmpty();
		RuleRunner::GetCompatibleModes(str.String(), modes);
		modes.FindString("modes", 0, &str);

		newtest.AddString("mode", str);
		newtest.AddString("value", "");

		SetTest(&newtest);
		SetMode(str.String());
	}
}


void
TestView::AttachedToWindow()
{
	fTestButton->SetTarget(this);
	fModeButton->SetTarget(this);
	fValueBox->SetTarget(this);
}


BRect
TestView::GetPreferredSize()
{
	BRect rect(fValueBox->Frame());
	rect.left = rect.top = 0.0;
	rect.bottom += 10.0;

	return rect;
}


void
TestView::ResizeToPreferred()
{
	BRect rect = GetPreferredSize();
	ResizeTo(rect.Width(),rect.Height());
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
			BString mode;
			if (msg->FindString("mode",&mode) != B_OK)
				break;

			SetMode(mode.String());
			break;
		}
		case MSG_SHOW_TEST_MENU:
		{
			BPopUpMenu* menu
				= (BPopUpMenu*)BPopUpMenu::Instantiate(&fArchivedTestMenu);
			menu->SetTargetForItems(this);

			for (int32 i = 0; i < menu->CountItems(); i++)
			{
				BMenuItem* item = menu->ItemAt(i);
				if (item->Submenu())
					item->Submenu()->SetTargetForItems(this);
			}

			BPoint point;
			uint32 buttons;
			GetMouse(&point, &buttons);
			ConvertToScreen(&point);
			point.x -= 10.0;
			if (point.x < 0.0)
				point.x = 0.0;

			point.y -= 10.0;
			if (point.y < 0.0)
				point.y = 0.0;

			menu->SetAsyncAutoDestruct(true);
			menu->Go(point, true, true, true);
			break;
		}
		case MSG_SHOW_TYPE_MENU:
		{
			BPopUpMenu* menu
				= (BPopUpMenu*)BPopUpMenu::Instantiate(&gArchivedTypeMenu);
			menu->SetTargetForItems(this);

			for (int32 i = 0; i < menu->CountItems(); i++)
			{
				BMenuItem* item = menu->ItemAt(i);
				if (item->Submenu())
					item->Submenu()->SetTargetForItems(this);
			}

			BPoint point;
			uint32 buttons;
			GetMouse(&point, &buttons);
			ConvertToScreen(&point);
			point.x -= 10.0;
			if (point.x < 0.0)
				point.x = 0.0;

			point.y -= 10.0;
			if (point.y < 0.0)
				point.y = 0.0;

			menu->SetAsyncAutoDestruct(true);
			menu->Go(point, true, true, true);
			break;
		}
		case MSG_SHOW_MODE_MENU:
		{
			ShowModeMenu();
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


void
TestView::SetupTestMenu()
{
	// These ones will always exist. Type is the default because it's probably
	// going to be the one most used
	BMessage* msg;
	BPopUpMenu* menu = new BPopUpMenu("Test");	

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
			msg->AddString("name", "Attribute");
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
	BString testtype;
	int32 i = 0;
	while (fTestTypes.FindString("tests", i, &testtype) == B_OK)
		i++;

	i--;

	while (i >= 0)
	{
		fTestTypes.FindString("tests", i, &testtype);
		msg = new BMessage(MSG_TEST_CHOSEN);
		msg->AddString("name", testtype);
		menu->AddItem(new BMenuItem(testtype.String(), msg),0);
		i--;
	}

	menu->Archive(&fArchivedTestMenu);
	delete menu;
}


void
TestView::ShowModeMenu()
{
	BPopUpMenu* menu = new BPopUpMenu("String");
	BMessage* msg, modes;
	
	if (RuleRunner::GetCompatibleModes(fTestButton->Label(), modes) != B_OK)
		return;
	
	BString modestr;
	int32 i = 0;
	while (modes.FindString("modes", i, &modestr) == B_OK)
	{
		i++;
		msg = new BMessage(MSG_MODE_CHOSEN);
		msg->AddString("mode", modestr);
		menu->AddItem(new BMenuItem(modestr.String(), msg));
	}

	menu->SetTargetForItems(this);

	BPoint point;
	uint32 buttons;
	GetMouse(&point, &buttons);
	ConvertToScreen(&point);
	point.x -= 10.0;
	if (point.x < 0.0)
		point.x = 0.0;

	point.y -= 10.0;
	if (point.y < 0.0)
		point.y = 0.0;

	menu->SetAsyncAutoDestruct(true);
	menu->Go(point, true, true, true);
}


BMenu*
TestView::AddMenuSorted(BMenu* parent, const char* name)
{
	// XXX: TODO: This doesn't work for some reason -- the items aren't sorted :(

	if (!name)
		return NULL;

	BMenu* menu = new BMenu(name);

	for (int32 i = 0; i < parent->CountItems(); i++)
	{
		BMenuItem* item = parent->ItemAt(i);

		if (strcmp(item->Label(), name) == -1) {
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
TestView::GetMenu(BMenu* parent, const char* name)
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

	BString str, mode, value;

	fTest->FindString("mode", &mode);
	fTest->FindString("value", &value);
	*fTest = *msg;

	fTest->what = 0;

	if (fTest->FindString("mode" ,&str) != B_OK)
		fTest->AddString("mode", mode);

	if (fTest->FindString("value", &str) != B_OK)
		fTest->AddString("value", value);

	BString label;

	fTest->FindString("name", &str);
	int32 testtype;
	if (str == "Attribute") {
		fTest->FindString("typename", &str);
		label = str;
		fTest->FindString("attrname", &str);
		label << " : " << str;
		testtype = TEST_TYPE_STRING;

		// Truncate the label because it is likely too long for the button
		be_plain_font->TruncateString(&label, B_TRUNCATE_SMART,
			fTestButton->Bounds().Width() - 10.0);
	} else {
		label = str;
		testtype = RuleRunner::GetDataTypeForTest(label.String());
	}

	fTestButton->SetLabel(label.String());

	// Now that the test button has been updated, make sure that the mode currently
	// set is supported by the current test
	int32 modetype = RuleRunner::GetDataTypeForMode(fModeButton->Label());
	if (testtype != modetype && modetype != TEST_TYPE_ANY) {
		STRACE(("Modes not compatible, refreshing.\n"));
		// Not compatible, so reset the mode to something compatible
		BMessage modes;
		RuleRunner::GetCompatibleModes(testtype, modes);

		BString modestr;
		modes.FindString("modes", 0, &modestr);
		SetMode(modestr.String());
	}
	STRACE(("-------------------------\n"));

	return true;
}


void
TestView::SetMode(const char* mode)
{
	if (!mode)
		return;

	// This function assumes that the string passed to it is valid
	// for the test type
	BString str;
	if (fTest->FindString("mode", &str) == B_OK)
		fTest->ReplaceString("mode", mode);
	else
		fTest->AddString("mode", mode);
	fModeButton->SetLabel(mode);
}


const char*
TestView::GetValue()
{
	// Exists for future expansion when different controls are associated with
	// different tests
	return fValueBox->Text();
}
