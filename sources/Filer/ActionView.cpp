/*
	ActionView.cpp: View for adjusting settings for an individual Filer action
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/

#include <Font.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <ScrollBar.h>
#include <String.h>

#include "ActionView.h"
#include "AutoTextControl.h"
#include "RuleRunner.h"

enum
{
	M_ACTION_CHOSEN = 'tsch',
	M_SHOW_ACTION_MENU = 'sham',
	M_VALUE_CHANGED = 'vlch'
};


ActionView::ActionView(const BRect& frame, const char* name, BMessage* action,
	const int32& resize, const int32& flags)
	:
	BView(frame, name, resize, flags | B_FRAME_EVENTS),
 	fAction(NULL)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	// Find the longest name in all the actions
	RuleRunner::GetActions(fActions);
	int32 i = 0;
	BString actionstr, wideststr;
	while (fActions.FindString("actions", i, &actionstr) == B_OK)
	{
		i++;
		if (actionstr.CountChars() > wideststr.CountChars())
			wideststr = actionstr;
	}

	fActionButton = new BButton(BRect(0, 0, 1, 1), "actionbutton",
		wideststr.String(), new BMessage(M_SHOW_ACTION_MENU));
	fActionButton->ResizeToPreferred();
	AddChild(fActionButton);

	BRect rect = fActionButton->Frame();
	rect.OffsetBy(rect.Width() + 5, 0);
	rect.right = Bounds().Width() - 10.0;
	if (rect.right < rect.left)
		rect.right = rect.left + 10;

	fValueBox = new AutoTextControl(rect, "valuebox", NULL, NULL,
		new BMessage(M_VALUE_CHANGED), B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
	AddChild(fValueBox);
	fValueBox->SetDivider(0);

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
}


ActionView::~ActionView()
{
	delete fAction;
}


void
ActionView::AttachedToWindow()
{
	fActionButton->SetTarget(this);
	fValueBox->SetTarget(this);

	// This seems stupid, but without it, fValueBox is *never* resized and I
	// can't find the cause of it. :(
	fValueBox->ResizeTo(Bounds().Width() - fValueBox->Frame().left,
		fValueBox->Bounds().Height());
	if (fValueBox->Bounds().Height() < fActionButton->Bounds().Height()) {
		fValueBox->MoveBy(0.0, (fActionButton->Bounds().Height()
			- fValueBox->Bounds().Height()) / 2.0);
	}
}


BRect
ActionView::GetPreferredSize()
{
	BRect rect(0.0, 0.0, 10.0, 10.0);

	rect.bottom = fActionButton->Frame().Height();
	rect.right = StringWidth("Move it to the Trash") + 5.0 + 100;

	return rect;
}


void
ActionView::ResizeToPreferred()
{
	BRect rect = GetPreferredSize();
	ResizeTo(rect.Width(),rect.Height());
}


void
ActionView::MessageReceived(BMessage* msg)
{
	switch (msg->what)
	{
		case M_SHOW_ACTION_MENU:
		{
			ShowActionMenu();
			break;
		}
		case M_ACTION_CHOSEN:
		{
			BString name;
			if (msg->FindString("name", &name) == B_OK)
				SetAction(name.String());
			break;
		}
		case M_VALUE_CHANGED:
		{
			BString str;
			if (fAction->FindString("value", &str) == B_OK)
				fAction->ReplaceString("value", fValueBox->Text());
			else
				fAction->AddString("value", fValueBox->Text());
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
	fActionButton->SetLabel(name);

	namestr = name;

	if (namestr.FindFirst("…") >= 0) {
		if (fValueBox->IsHidden())
			fValueBox->Show();
	} else {
		if (!fValueBox->IsHidden())
			fValueBox->Hide();
	}
}


void
ActionView::ShowActionMenu()
{
	BPopUpMenu* menu = new BPopUpMenu("");
	BMessage* msg;
	
	int32 i = 0;
	BString name;
	while (fActions.FindString("actions", i, &name) == B_OK)
	{
		i++;
		msg = new BMessage(M_ACTION_CHOSEN);
		msg->AddString("name", name.String());
		menu->AddItem(new BMenuItem(name.String(), msg));
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
