/*
	ActionView.cpp: View for adjusting settings for an individual Filer action
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
	Contributed by:
		Pete Goodeve, 2016
		Owen Pan <owen.pan@yahoo.com>, 2017
*/

#include "ActionView.h"

#include <LayoutBuilder.h>

#include "FilerDefs.h"
#include "RuleRunner.h"


ActionView::ActionView(const char* name, BMessage* action, const int32& flags)
	:
	BView(name, flags | B_FRAME_EVENTS),
 	fAction(NULL)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	RuleRunner::GetActions(fActions);

	fActionField = new BMenuField(NULL, ActionMenu());

	fValueBox = new AutoTextControl("valuebox", NULL, NULL, new BMessage());
	fValueBox->SetDivider(0);

	BLayoutBuilder::Group<>(this, B_HORIZONTAL, B_USE_DEFAULT_SPACING)
		.Add(fActionField, 0)
		.Add(fValueBox)
		.End();

	bool usedefaults = false;
	if (action) {
		fAction = new BMessage(*action);

		BString str;
		if (fAction->FindString("name", &str) == B_OK)
			SetAction(str.String());
		else
			usedefaults = true;

		if (!usedefaults && fAction->FindString("value", &str) == B_OK)
			fValueBox->SetText(str.String());
		else
			usedefaults = true;
	} else
		usedefaults = true;

	if (usedefaults) {
		if (!fAction)
			fAction = new BMessage;

		BString str;
		if (fActions.FindString("actions", 0, &str) == B_OK)
			SetAction(str.String());
		else
			SetAction("Nothing");

		fValueBox->SetText("");
	}

	BString toolTip(
		"\%FILENAME\%\t\tFull file name\n"
		"\%EXTENSION\%\tJust the extension\n"
		"\%BASENAME\%\tFile name without extension\n"
		"\%FOLDER\%\t\tFull location of the folder which contains the file\n"
		"\%FULLPATH\%\t\tFull location of the file\n"
		"\%DATE\%\t\t\tCurrent date in the format MM-DD-YYYY\n"
		"\%EURODATE\%\t\tCurrent date in the format DD-MM-YYYY\n"
		"\%REVERSEDATE\%\tCurrent date in the format YYYY-MM-DD\n"
		"\%TIME\%\t\t\tCurrent time using 24-hour time\n"
		"\%ATTR:xxxx\%\t\tAn extended attribute of the file");
	fValueBox->SetToolTip(toolTip.String());
}


ActionView::~ActionView()
{
	delete fActionField;
	delete fValueBox;
	delete fAction;
}


void
ActionView::AttachedToWindow()
{
	fActionField->Menu()->SetTargetForItems(this);
	fValueBox->SetTarget(this);
}


void
ActionView::MessageReceived(BMessage* msg)
{
	switch (msg->what)
	{
		case MSG_ACTION_CHOSEN:
		{
			BString name;
			if (msg->FindString("name", &name) == B_OK)
				SetAction(name.String());
			break;
		}
		default:
		{
			BView::MessageReceived(msg);
		}
	}
}


BMessage*
ActionView::GetAction() const
{
	BString str;
	if (fAction->FindString("value", &str) == B_OK)
		fAction->ReplaceString("value", fValueBox->Text());
	else
		fAction->AddString("value", fValueBox->Text());

	return fAction;
}


void
ActionView::SetAction(const char* name)
{
	BString namestr(name);
	
	if (fAction->FindString("name", &namestr) == B_OK)
		fAction->ReplaceString("name", name);
	else
		fAction->AddString("name", name);
	fActionField->MenuItem()->SetLabel(name);

	namestr = name;

	if (namestr.FindFirst("…") >= 0) {
		if (fValueBox->IsHidden())
			fValueBox->Show();
	} else {
		if (!fValueBox->IsHidden())
			fValueBox->Hide();
	}
}


BPopUpMenu*
ActionView::ActionMenu() const
{
	BPopUpMenu* menu = new BPopUpMenu("");

	BString name;
	for (int32 i = 0; fActions.FindString("actions", i, &name) == B_OK; i++) {
		BMessage* msg = new BMessage(MSG_ACTION_CHOSEN);
		msg->AddString("name", name.String());
		menu->AddItem(new BMenuItem(name.String(), msg));
	}

	return menu;
}
