/*
	RuleEditWindow.cpp: Rule editor class (duh)
	Released under the MIT license.
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Contributed by:
		Humdinger <humdingerb@gmail.com>, 2016
		Owen Pan <owen.pan@yahoo.com>, 2017
*/

#include "RuleEditWindow.h"

#include <Alert.h>
#include <Application.h>
#include <Catalog.h>

#include "FilerDefs.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "RuleEditWindow"


RuleEditWindow::RuleEditWindow(FilerRule* rule, BHandler* caller)
	:
	BWindow(BRect(0, 0, 420, 0), B_TRANSLATE("Edit rule"), B_TITLED_WINDOW,
		B_ASYNCHRONOUS_CONTROLS | B_CLOSE_ON_ESCAPE | B_AUTO_UPDATE_SIZE_LIMITS),
	fTestView(NULL),
	fActionView(NULL),
	fEmptyCount(0),
	fOriginalID(-1),
	fCaller(caller)
{
	if (rule)
		fOriginalID = rule->GetID();
	else
		SetTitle(B_TRANSLATE("Add rule"));

	// Description
	fDescriptionBox = new AutoTextControl("description",
		B_TRANSLATE("Description:"), NULL, new BMessage(MSG_NEW_DESCRIPTION));

	if (rule)
		fDescriptionBox->SetText(rule->GetDescription());

	// Set up the tests group and associated buttons
	fTestGroup = new BBox("whengroup");
	fTestGroup->SetLabel(B_TRANSLATE("When"));

	// Set up the actions group and associated buttons
	fActionGroup = new BBox("dogroup");
	fActionGroup->SetLabel(B_TRANSLATE("Do"));

	fOK = new BButton("okbutton", B_TRANSLATE("OK"), new BMessage(MSG_OK));
	// calling AddChild later to ensure proper keyboard navigation

	fCancel = new BButton("cancelbutton", B_TRANSLATE("Cancel"),
		new BMessage(MSG_CANCEL));

	fHelp = new BButton("helpbutton", B_TRANSLATE("Help" B_UTF8_ELLIPSIS),
		new BMessage(MSG_HELP));
	fHelp->SetTarget(be_app);

	fOK->MakeDefault(true);

	int spacing = B_USE_HALF_ITEM_SPACING;
	int inset = B_USE_HALF_ITEM_INSETS;

	fTestGroupLayout = BLayoutBuilder::Group<>(B_VERTICAL, spacing)
		.SetInsets(inset, inset, inset, inset);

	fActionGroupLayout = BLayoutBuilder::Group<>(B_VERTICAL, spacing)
		.SetInsets(inset, inset, inset, inset);

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

	if (fEmptyCount > 0)
		fOK->SetEnabled(false);

	CenterOnScreen();
	Show();
}


void
RuleEditWindow::MessageReceived(BMessage* msg)
{
	switch (msg->what)
	{
		case MSG_OK:
			SendRuleMessage();
		case MSG_CANCEL:
			PostMessage(B_QUIT_REQUESTED);
			break;
		case MSG_ADD_TEST:
			msg->FindPointer(kPointer, (void**) &fTestView);
			AppendTest(NULL);
			break;
		case MSG_REMOVE_TEST:
			msg->FindPointer(kPointer, (void**) &fTestView);
			RemoveTest();
			break;
		case MSG_ADD_ACTION:
			msg->FindPointer(kPointer, (void**) &fActionView);
			AppendAction(NULL);
			break;
		case MSG_REMOVE_ACTION:
			msg->FindPointer(kPointer, (void**) &fActionView);
			RemoveAction();
			break;
		default:
			BWindow::MessageReceived(msg);
	}
}


void
RuleEditWindow::UpdateEmptyCount(bool increment)
{
	if (increment)
		fEmptyCount++;
	else
		fEmptyCount--;

	fOK->SetEnabled(fEmptyCount == 0);
}


void
RuleEditWindow::AppendTest(BMessage* test)
{
	TestView* view = new TestView("test", test);

	if (fTestView == NULL) {
		fTestGroupLayout->AddView(view);
		fTestList.AddItem(view);
	} else {
		int32 index = fTestList.IndexOf(fTestView) + 1;
		fTestGroupLayout->AddView(index, view);
		fTestList.AddItem(view, index);
	}

	((TestView*) fTestList.FirstItem())
		->SetRemoveEnabled(fTestList.CountItems() > 1);
}


void
RuleEditWindow::RemoveTest()
{
	fTestList.RemoveItem(fTestView);
	fTestGroupLayout->RemoveView(fTestView);
	fTestView->RemoveSelf();
	delete fTestView;

	if (fTestList.CountItems() == 1)
		((TestView*) fTestList.FirstItem())->SetRemoveEnabled(false);
}


void
RuleEditWindow::AppendAction(BMessage* action)
{
	ActionView* view = new ActionView("action", action);

	if (fActionView == NULL) {
		fActionGroupLayout->AddView(view);
		fActionList.AddItem(view);
	} else {
		int32 index = fActionList.IndexOf(fActionView) + 1;
		fActionGroupLayout->AddView(index, view);
		fActionList.AddItem(view, index);
	}

	((ActionView*) fActionList.FirstItem())
		->SetRemoveEnabled(fActionList.CountItems() > 1);
}


void
RuleEditWindow::RemoveAction()
{
	fActionList.RemoveItem(fActionView);
	fActionGroupLayout->RemoveView(fActionView);
	fActionView->RemoveSelf();
	delete fActionView;

	if (fActionList.CountItems() == 1)
		((ActionView *) fActionList.FirstItem())->SetRemoveEnabled(false);
}


void
RuleEditWindow::SendRuleMessage()
{
	FilerRule* rule = new FilerRule;
	BString str(fDescriptionBox->Text());

	rule->SetDescription(str.Trim());

	for (int32 i = 0; i < fTestList.CountItems(); i++)
	{
		TestView* view = (TestView*)fTestList.ItemAt(i);
		rule->AddTest(view->GetTest());
	}

	for (int32 i = 0; i < fActionList.CountItems(); i++)
	{
		ActionView* view = (ActionView*)fActionList.ItemAt(i);
		rule->AddAction(view->GetAction());
	}

	BMessage msg;
	msg.what = (fOriginalID >= 0) ? MSG_UPDATE_RULE : MSG_ADD_RULE;
	msg.AddPointer("item", rule);
	msg.AddInt64("id", fOriginalID);
	
	fCaller->Looper()->PostMessage(&msg, fCaller);
}
