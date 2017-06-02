/*
	RuleEditWindow.cpp: Rule editor class (duh)
	Released under the MIT license.
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Contributed by:
		Humdinger <humdingerb@gmail.com>, 2016
		Owen Pan <owen.pan@yahoo.com>, 2017
*/

#include <Alert.h>
#include <Application.h>
#include <LayoutBuilder.h>
#include <Message.h>
#include <Messenger.h>
#include <Path.h>
#include <Roster.h>
#include <View.h>

#include "ActionView.h"
#include "AutoTextControl.h"
#include "FilerDefs.h"
#include "FilerRule.h"
#include "main.h"
#include "RuleEditWindow.h"
#include "TestView.h"


RuleEditWindow::RuleEditWindow(FilerRule* rule, BHandler* caller)
	:
	BWindow(BRect(0, 0, 400, 0), "Edit rule", B_TITLED_WINDOW,
		B_ASYNCHRONOUS_CONTROLS | B_CLOSE_ON_ESCAPE | B_AUTO_UPDATE_SIZE_LIMITS),
	fOriginalID(-1),
	fCaller(caller)
{
	if (rule)
		fOriginalID = rule->GetID();
	else
		SetTitle("Add rule");

	// Description
	fDescriptionBox = new AutoTextControl("description", "Description: ", NULL,
		new BMessage(MSG_NEW_DESCRIPTION));

	if (rule)
		fDescriptionBox->SetText(rule->GetDescription());

	// Set up the tests group and associated buttons
	fTestGroup = new BBox("whengroup");
	fTestGroup->SetLabel("When");

	fAddTest = new BButton("addtestbutton", "Add", new BMessage(MSG_ADD_TEST));

	fRemoveTest = new BButton("removetestbutton", "Remove",
		new BMessage(MSG_REMOVE_TEST));
	fRemoveTest->SetEnabled(false);

	// Set up the actions group and associated buttons
	fActionGroup = new BBox("dogroup");
	fActionGroup->SetLabel("Do");

	fAddAction = new BButton("addactionbutton", "Add",
		new BMessage(MSG_ADD_ACTION));

	fRemoveAction = new BButton("removeactionbutton", "Remove",
		new BMessage(MSG_REMOVE_ACTION));
	fRemoveAction->SetEnabled(false);


	fOK = new BButton("okbutton", "OK", new BMessage(MSG_OK));
	// calling AddChild later to ensure proper keyboard navigation

	fCancel = new BButton("cancelbutton", "Cancel", new BMessage(MSG_CANCEL));

	fHelp = new BButton("helpbutton", "Help…", new BMessage(MSG_HELP));
	fHelp->SetTarget(be_app);

	fOK->MakeDefault(true);

	int spacing = B_USE_HALF_ITEM_SPACING;
	int inset = B_USE_HALF_ITEM_INSETS;

	fTestGroupLayout = BLayoutBuilder::Group<>(B_VERTICAL, spacing)
		.SetInsets(inset, inset, inset, inset)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(fAddTest)
			.Add(fRemoveTest)
			.AddGlue()
			.End();

	fActionGroupLayout = BLayoutBuilder::Group<>(B_VERTICAL, spacing)
		.SetInsets(inset, inset, inset, inset)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(fAddAction)
			.Add(fRemoveAction)
			.AddGlue()
			.End();

	if (rule) {
		for (int32 i = 0; i < rule->CountTests(); i++)
			AppendTest(rule->TestAt(i));

		for (int32 i = 0; i < rule->CountActions(); i++)
			AppendAction(rule->ActionAt(i));
	} else {
		AppendTest(NULL);
		AppendAction(NULL);
	}

	fTestGroup->AddChild(fTestGroupLayout->View());
	fActionGroup->AddChild(fActionGroupLayout->View());

	inset = B_USE_SMALL_INSETS;

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(inset, inset, inset, inset)
		.Add(fDescriptionBox)
		.Add(fTestGroup)
		.Add(fActionGroup)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(fHelp)
			.AddGlue()
			.Add(fCancel)
			.Add(fOK)
			.End()
		.End();

	fDescriptionBox->MakeFocus();

	CenterOnScreen();
	Show();
}


RuleEditWindow::~RuleEditWindow()
{
}


void
RuleEditWindow::MessageReceived(BMessage* msg)
{
	switch (msg->what)
	{
		case MSG_OK:
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
		case MSG_CANCEL:
		{
			PostMessage(B_QUIT_REQUESTED);
			break;
		}
		case MSG_ADD_TEST:
		{
			AppendTest(NULL);
			break;
		}
		case MSG_REMOVE_TEST:
		{
			RemoveTest();
			break;
		}
		case MSG_ADD_ACTION:
		{
			AppendAction(NULL);
			break;
		}
		case MSG_REMOVE_ACTION:
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
	TestView* view = new TestView("test", test);

	fTestGroupLayout->AddView(fTestList.CountItems(), view);
	fTestList.AddItem(view);

	if (fTestList.CountItems() > 1 && !fRemoveTest->IsEnabled())
		fRemoveTest->SetEnabled(true);
}


void
RuleEditWindow::RemoveTest()
{
	TestView* view = (TestView*)fTestList.RemoveItem(fTestList.CountItems() - 1);
	fTestGroupLayout->RemoveView(view);
	view->RemoveSelf();
	delete view;

	if (fTestList.CountItems() == 1)
		fRemoveTest->SetEnabled(false);
}


void
RuleEditWindow::AppendAction(BMessage* action)
{
	ActionView* view = new ActionView("action", action);

	fActionGroupLayout->AddView(fActionList.CountItems(), view);
	fActionList.AddItem(view);

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
	msg.what = (fOriginalID >= 0) ? MSG_UPDATE_RULE : MSG_ADD_RULE;
	msg.AddPointer("item", rule);
	msg.AddInt64("id", fOriginalID);
	
	fCaller->Looper()->PostMessage(&msg, fCaller);
}
