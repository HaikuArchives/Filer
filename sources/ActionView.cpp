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

	fAddRemoveButtons = new AddRemoveButtons(MSG_ADD_ACTION, MSG_REMOVE_ACTION,
		this);

	BLayoutBuilder::Group<>(this, B_HORIZONTAL, B_USE_HALF_ITEM_SPACING)
		.Add(fActionField, 0)
		.Add(fValueBox)
		.Add(fAddRemoveButtons)
		.End();

	bool usedefaults = false;
	if (action) {
		fAction = new BMessage(*action);

		int8 type;
		if (fAction->FindInt8("type", &type) == B_OK)
			SetAction(type);
		else
			usedefaults = true;

		BString str;
		if (!usedefaults && fAction->FindString("value", &str) == B_OK)
			fValueBox->SetText(str.String());
		else
			usedefaults = true;
	} else
		usedefaults = true;

	if (usedefaults) {
		if (!fAction)
			fAction = new BMessage;

		int8 type;
		if (fActions.FindInt8("actions", 0, &type) == B_OK)
			SetAction(type);
		else
			SetAction(0);

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
	delete fAddRemoveButtons;
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
			int8 type;
			if (msg->FindInt8("type", &type) == B_OK)
				SetAction(type);
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
ActionView::SetAction(int8 type)
{
	int8 tmpType;
	if (fAction->FindInt8("type", &tmpType) == B_OK)
		fAction->ReplaceInt8("type", type);
	else
		fAction->AddInt8("type", type);

	BString name(sActions[type].locale);
	fActionField->MenuItem()->SetLabel(name);

	if (name.FindFirst("…") >= 0) {
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

	int8 type;
	for (int32 i = 0; fActions.FindInt8("actions", i, &type) == B_OK; i++) {
		BMessage* msg = new BMessage(MSG_ACTION_CHOSEN);
		msg->AddInt8("name", type);
		menu->AddItem(new BMenuItem(sActions[type].locale, msg));
	}

	return menu;
}
