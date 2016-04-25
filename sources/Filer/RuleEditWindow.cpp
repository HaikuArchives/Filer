/*
	RuleEditWindow.cpp: Rule editor class (duh)
	Released under the MIT license.
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Contributed by: Humdinger <humdingerb@gmail.com>, 2016
*/

#include <Alert.h>
#include <Application.h>
#include <Message.h>
#include <Messenger.h>
#include <Path.h>
#include <Roster.h>
#include <View.h>

#include "ActionView.h"
#include "AutoTextControl.h"
#include "FilerRule.h"
#include "main.h"
#include "RuleEditWindow.h"
#include "TestView.h"

// Internal message defs
enum
{
	M_DESCRIPTION_CHANGED = 'dsch',
	M_OK = 'mok ',
	M_CANCEL = 'cncl',
	
	M_ADD_TEST = 'adts',
	M_REMOVE_TEST = 'rmts',
	
	M_ADD_ACTION = 'adac',
	M_REMOVE_ACTION = 'rmac',
	
	M_SHOW_HELP = 'shhl',
	M_SHOW_DOCS = 'shdc'
};


RuleEditWindow::RuleEditWindow(BRect& rect, FilerRule* rule)
	:
	BWindow(rect, "Edit rule", B_TITLED_WINDOW,
		B_ASYNCHRONOUS_CONTROLS | B_NOT_RESIZABLE | B_CLOSE_ON_ESCAPE),
 	fOriginalID(-1)
{
	if (rule)
		fOriginalID = rule->GetID();
	else
		SetTitle("Add rule");

	BView* top = new BView(Bounds(), "top", B_FOLLOW_ALL, B_WILL_DRAW);
	top->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(top);

	// Description
	fDescriptionBox = new AutoTextControl(BRect(0, 0, 1, 1), "description",
		"Description: ", NULL, new BMessage(M_DESCRIPTION_CHANGED),
		B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
	top->AddChild(fDescriptionBox);

	float width, height;
	fDescriptionBox->GetPreferredSize(&width, &height);
	fDescriptionBox->ResizeTo(Bounds().Width() - 20, height);
	fDescriptionBox->MoveTo(10, 10);
	fDescriptionBox->SetDivider(be_plain_font->StringWidth("Description: ") + 5);

	if (rule)
		fDescriptionBox->SetText(rule->GetDescription());

	// Separator line	
	BRect rect(fDescriptionBox->Frame());
	rect.OffsetBy(0, rect.IntegerHeight() + 10);
	rect.bottom = rect.top + 1.0;
	BBox* box = new BBox(rect, NULL, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
	top->AddChild(box);

	// Set up the tests group and associated buttons
	rect.OffsetBy(0, 12.0);
	rect.bottom = rect.top + 20.0;
	fTestGroup = new BBox(rect, "whengroup", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
	fTestGroup->SetLabel("When");
	top->AddChild(fTestGroup);

	fAddTest = new BButton(BRect(0, 0, 1, 1),"addtestbutton", "Add",
		new BMessage(M_ADD_TEST), B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	fAddTest->ResizeToPreferred();
	fAddTest->MoveTo(10.0, fTestGroup->Bounds().bottom - 10.0
		- fAddTest->Bounds().Height());
	fTestGroup->AddChild(fAddTest);

	fRemoveTest = new BButton(BRect(0, 0, 1, 1),"removetestbutton", "Remove",
		new BMessage(M_REMOVE_TEST), B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	fRemoveTest->ResizeToPreferred();
	fRemoveTest->MoveTo(fAddTest->Frame().right + 10, fAddTest->Frame().top);
	fTestGroup->AddChild(fRemoveTest);
	fRemoveTest->SetEnabled(false);

	fTestGroup->ResizeBy(0, fAddTest->Bounds().Height() + 10.0);

	// Set up the actions group and associated buttons

	rect.OffsetTo(10, fTestGroup->Frame().bottom + 10.0);
	fActionGroup = new BBox(rect, "dogroup", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
	fActionGroup->SetLabel("Do");
	top->AddChild(fActionGroup);

	fAddAction = new BButton(BRect(0, 0, 1, 1), "addactionbutton", "Add",
	new BMessage(M_ADD_ACTION), B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	fAddAction->ResizeToPreferred();
	fAddAction->MoveTo(10.0, fActionGroup->Bounds().bottom - 10.0
		- fAddAction->Bounds().Height());
	fActionGroup->AddChild(fAddAction);

	fRemoveAction = new BButton(BRect(0, 0, 1, 1), "removeactionbutton", "Remove",
		new BMessage(M_REMOVE_ACTION), B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	fRemoveAction->ResizeToPreferred();
	fRemoveAction->MoveTo(fAddAction->Frame().right + 10, fAddAction->Frame().top);
	fActionGroup->AddChild(fRemoveAction);
	fRemoveAction->SetEnabled(false);

	fActionGroup->ResizeBy(0, fAddAction->Bounds().Height() + 10.0);


	fOK = new BButton(BRect(0, 0, 1, 1), "okbutton", "OK", new BMessage(M_OK),
		B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	fOK->ResizeToPreferred();
	fOK->MoveTo(Bounds().right - fOK->Bounds().Width() - 10,
		Bounds().bottom - fOK->Bounds().Height() - 10);
	// calling AddChild later to ensure proper keyboard navigation

	fCancel = new BButton(BRect(0, 0, 1 ,1), "cancelbutton", "Cancel",
		new BMessage(M_CANCEL), B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	fCancel->ResizeToPreferred();
	fCancel->MoveTo(fOK->Frame().left - fCancel->Bounds().Width() - 10, 
		fOK->Frame().top);

	fHelp = new BButton(BRect(0, 0, 1, 1), "helpbutton", "Help…",
		new BMessage(M_SHOW_HELP), B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	fHelp->ResizeToPreferred();
	fHelp->MoveTo(10, fOK->Frame().top);
	fHelp->SetTarget(be_app);
	top->AddChild(fHelp);

	top->AddChild(fCancel);
	top->AddChild(fOK);
	fOK->MakeDefault(true);

	if (rule) {
		for (int32 i = 0; i < rule->CountTests(); i++)
			AppendTest(rule->TestAt(i));

		for (int32 i = 0; i < rule->CountActions(); i++)
			AppendAction(rule->ActionAt(i));
	} else {
		AppendTest(NULL);
		AppendAction(NULL);
	}

	ResizeTo(Bounds().Width(), fActionGroup->Frame().bottom + 15
		+ fOK->Bounds().Height());

	fDescriptionBox->MakeFocus();
}


RuleEditWindow::~RuleEditWindow()
{
}


void
RuleEditWindow::MessageReceived(BMessage* msg)
{
	switch (msg->what)
	{
		case M_OK:
		{
			if (strlen(fDescriptionBox->Text()) < 1) {
				BAlert* alert = new BAlert("Filer",
					"You need to add a description if you want to add this "
					"rule to the list.", "OK");
				alert->Go();
				fDescriptionBox->MakeFocus(true);
				break;
			}

			SendRuleMessage();
			PostMessage(B_QUIT_REQUESTED);
			break;
		}
		case M_CANCEL:
		{
			PostMessage(B_QUIT_REQUESTED);
			break;
		}
		case M_ADD_TEST:
		{
			AppendTest(NULL);
			break;
		}
		case M_REMOVE_TEST:
		{
			RemoveTest();
			break;
		}
		case M_ADD_ACTION:
		{
			AppendAction(NULL);
			break;
		}
		case M_REMOVE_ACTION:
		{
			RemoveAction();
			break;
		}
		default:
			BWindow::MessageReceived(msg);
	}
}


void
RuleEditWindow::AppendTest(BMessage* test)
{
	TestView* view = new TestView(BRect(0, 0, 1, 1), "test", test,
		B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
	view->ResizeToPreferred();
	BRect rect = view->GetPreferredSize();
	fTestGroup->ResizeBy(0, rect.Height() + 10);

	if (fTestGroup->Bounds().Width() < rect.Width() + 20)
		ResizeBy(rect.Width() + 20 - fTestGroup->Bounds().Width(),0);

	TestView* last = (TestView*)fTestList.ItemAt(fTestList.CountItems() - 1);
	
	if (last)
		view->MoveTo(last->Frame().left, last->Frame().bottom + 10);
	else
		view->MoveTo(10, 15);

	fTestGroup->AddChild(view);
	fTestList.AddItem(view);

	fActionGroup->MoveBy(0, rect.Height() + 10);
	ResizeBy(0, rect.Height() + 10);

	if (fTestList.CountItems() > 1 && !fRemoveTest->IsEnabled())
		fRemoveTest->SetEnabled(true);
}


void
RuleEditWindow::RemoveTest()
{
	TestView* view = (TestView*)fTestList.RemoveItem(fTestList.CountItems() - 1);
	view->RemoveSelf();
	fTestGroup->ResizeBy(0, -view->Bounds().Height() - 10);
	fActionGroup->MoveBy(0, -view->Bounds().Height() - 10);
	ResizeBy(0, -view->Bounds().Height() - 10);
	delete view;

	if (fTestList.CountItems() == 1)
		fRemoveTest->SetEnabled(false);
}


void
RuleEditWindow::AppendAction(BMessage* action)
{
	ActionView* view = new ActionView(BRect(0, 0, 1, 1), "action", action,
		B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
	BRect rect = view->GetPreferredSize();
	view->ResizeTo(fActionGroup->Bounds().Width() - 20, rect.Height());
	fActionGroup->ResizeBy(0, rect.Height() + 10);

	if (fActionGroup->Bounds().Width() < rect.Width() + 20)
		ResizeBy(rect.Width() + 20 - fActionGroup->Bounds().Width(), 0);

	ActionView* last
		= (ActionView*)fActionList.ItemAt(fActionList.CountItems() - 1);

	if (last)
		view->MoveTo(last->Frame().left, last->Frame().bottom + 10);
	else
		view->MoveTo(10, 15);

	fActionGroup->AddChild(view);
	fActionList.AddItem(view);

	ResizeBy(0, rect.Height() + 10);

	if (fActionList.CountItems() > 1 && !fRemoveAction->IsEnabled())
		fRemoveAction->SetEnabled(true);
}


void
RuleEditWindow::RemoveAction()
{
	ActionView* view
		= (ActionView*)fActionList.RemoveItem(fActionList.CountItems() - 1);
	view->RemoveSelf();
	fActionGroup->ResizeBy(0, -view->Bounds().Height() - 10);
	ResizeBy(0, -view->Bounds().Height() - 10);
	delete view;

	if (fActionList.CountItems() == 1)
		fRemoveAction->SetEnabled(false);
}


BRect
RuleEditWindow::GetPreferredSize() const
{
	// Base minimum size, padding included
	BRect rect(0.0, 0.0, 320.0, 220.0);

	// Figure preferred height
	rect.bottom += fDescriptionBox->Bounds().Height() + 10.0;

	// 2 pixels for separator line + 10 pixels padding above it
	rect.bottom += 12.0;

	// Base minimum size for boxes (including inside padding) of 20 each and
	// outside padding of 10 pixels above each box
	rect.bottom += 40.0 + 20.0;

	rect.bottom += fOK->Bounds().Height() + 10.0;

	return rect;
}


void
RuleEditWindow::SendRuleMessage()
{
	FilerRule* rule = new FilerRule;

	rule->SetDescription(fDescriptionBox->Text());

	for (int32 i = 0; i < fTestList.CountItems(); i++)
	{
		TestView* view = (TestView*)fTestList.ItemAt(i);
		rule->AddTest(new BMessage(*view->GetTest()));
	}

	for (int32 i = 0; i < fActionList.CountItems(); i++)
	{
		ActionView* view = (ActionView*)fActionList.ItemAt(i);
		rule->AddAction(new BMessage(*view->GetAction()));
	}

	BMessage msg;
	msg.what = (fOriginalID >= 0) ? M_UPDATE_RULE : M_ADD_RULE;
	msg.AddPointer("item", rule);
	msg.AddInt64("id", fOriginalID);
	
	for (int32 i = 0; i < be_app->CountWindows(); i++)
	{
		BWindow* win = be_app->WindowAt(i);
		if (strcmp(win->Title(), "Filer settings") == 0)
			win->PostMessage(&msg);
	}
}
