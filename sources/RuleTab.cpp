/*
 * Copyright 2008, 2016. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *  DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
 *	Humdinger, humdingerb@gmail.com
 */
#include <Alert.h>
#include <Application.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>
#include <Messenger.h>
#include <ScrollView.h>
#include <View.h>

#include "FilerRule.h"
#include "FilerDefs.h"
#include "main.h"
#include "RuleEditWindow.h"
#include "RuleItem.h"
#include "RuleRunner.h"
#include "RuleTab.h"


RuleTab::RuleTab()
	:
	BView("Rules", B_SUPPORTS_LAYOUT)
{
	_BuildLayout();

	fRuleList = new BObjectList<FilerRule>(20, true);
	LoadRules(fRuleList);

	for (int32 i = 0; i < fRuleList->CountItems(); i++)
		fRuleItemList->AddItem(new RuleItem(fRuleList->ItemAt(i)));
	
	fRuleItemList->MakeFocus();
	if (fRuleItemList->CountItems() > 0)
		fRuleItemList->Select(0L);
	else {
		BAlert* alert = new BAlert("Filer",
			"It appears that there aren't any rules for "
			"organizing files. Would you like Filer to "
			"add some basic ones for you?",
			"No", "Yes");

		if (alert->Go() == 1) {
			FilerRule* rule = new FilerRule();

			// NOTE: If actions 
			rule->AddTest(MakeTest("Type", "is", "text/plain"));
			rule->AddAction(MakeAction("Move to folder…", "/boot/home/Documents"));
			rule->SetDescription("Store text files in my Documents folder");
			AddRule(rule);

			rule = new FilerRule();
			rule->AddTest(MakeTest("Type", "is", "application/pdf"));
			rule->AddAction(MakeAction("Move to folder…", "/boot/home/Documents"));
			rule->SetDescription("Store PDF files in my Documents folder");
			AddRule(rule);

			rule = new FilerRule();
			rule->AddTest(MakeTest("Type", "starts with", "image/"));
			rule->AddAction(MakeAction("Move to folder…", "/boot/home/Pictures"));
			rule->SetDescription("Store pictures in my Pictures folder");
			AddRule(rule);

			rule = new FilerRule();
			rule->AddTest(MakeTest("Type", "starts with","video/"));
			rule->AddAction(MakeAction("Move to folder…", "/boot/home/Videos"));
			rule->SetDescription("Store movie files in my Videos folder");
			AddRule(rule);

			rule = new FilerRule();
			rule->AddTest(MakeTest("Name", "ends with", ".zip"));
			rule->AddAction(MakeAction("Shell command…",
				"unzip %FULLPATH% -d /boot/home/Desktop"));
			rule->SetDescription("Extract ZIP files to the Desktop");
			AddRule(rule);

//			rule = new FilerRule();
//			rule->AddTest(MakeTest("","",""));
//			rule->AddAction(MakeAction("",""));
//			rule->SetDescription("");
//			AddRule(rule);
		}
		SaveRules(fRuleList);
	}
}


RuleTab::~RuleTab()
{
	delete fRuleList;
}


void
RuleTab::_BuildLayout()
{
	fMatchBox = new BCheckBox("matchoncebox",
		"Apply only the first matching rule",
		new BMessage(MSG_MATCH_ONCE));

	fRuleItemList = new BListView("rulelist", B_SINGLE_SELECTION_LIST,
		B_WILL_DRAW	| B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE | B_NAVIGABLE);
	fScrollView = new BScrollView("listscroll", fRuleItemList,
		B_FRAME_EVENTS | B_WILL_DRAW, false, true);

	fRuleItemList->SetSelectionMessage(new BMessage(MSG_RULE_SELECTED));
	fRuleItemList->SetInvocationMessage(new BMessage(MSG_SHOW_EDIT_WINDOW));
	
	fAddButton = new BButton("addbutton", "Add" B_UTF8_ELLIPSIS,
		new BMessage(MSG_SHOW_ADD_WINDOW));

	fEditButton = new BButton("editbutton", "Edit" B_UTF8_ELLIPSIS,
		new BMessage(MSG_SHOW_EDIT_WINDOW));
	fEditButton->SetEnabled(false);

	fRemoveButton = new BButton("removebutton", "Remove",
		new BMessage(MSG_REMOVE_RULE));
	fRemoveButton->SetEnabled(false);

	fMoveUpButton = new BButton("moveupbutton", "Move up",
		new BMessage(MSG_MOVE_RULE_UP));
	fMoveUpButton->SetEnabled(false);
	
	fMoveDownButton = new BButton("movedownbutton", "Move down",
		new BMessage(MSG_MOVE_RULE_DOWN));
	fMoveDownButton->SetEnabled(false);

	static const float spacing = be_control_look->DefaultItemSpacing();
	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(spacing)
		.Add(fMatchBox)
		.AddGroup(B_HORIZONTAL)
			.AddGroup(B_VERTICAL, 10.0f)
				.Add(fScrollView)
				.AddGroup(B_HORIZONTAL)
					.AddGlue()
					.Add(fAddButton)
					.Add(fEditButton)
					.Add(fRemoveButton)
					.AddGlue()
				.End()
			.End()
			.AddGroup(B_VERTICAL)
				.Add(fMoveUpButton)
				.Add(fMoveDownButton)
				.AddGlue()
			.End()
		.End();
}


void
RuleTab::AttachedToWindow()
{
	fMatchBox->SetTarget(Looper());
	fAddButton->SetTarget(this);
	fEditButton->SetTarget(this);
	fRemoveButton->SetTarget(this);
	fMoveUpButton->SetTarget(this);
	fMoveDownButton->SetTarget(this);
	fRuleItemList->SetTarget(this);

	if (fRuleItemList->CountItems() > 0)
		UpdateButtons();

	App* my_app = dynamic_cast<App*>(be_app);
	fMatchBox->SetValue(my_app->GetMatchSetting());

	BView::AttachedToWindow();
}


void
RuleTab::MessageReceived(BMessage* message)
{
//	message->PrintToStream();
	switch (message->what)
	{
		case MSG_SHOW_ADD_WINDOW:
		{
			BRect frame(Frame());
			ConvertToScreen(&frame);
			frame.right = frame.left + 400;
			frame.bottom = frame.top + 300;
			frame.OffsetBy(60, 30);

			RuleEditWindow* rulewin = new RuleEditWindow(frame, NULL, this);
			rulewin->Show();
			break;
		}
		case MSG_SHOW_EDIT_WINDOW:
		{
			BRect frame(Frame());
			ConvertToScreen(&frame);
			frame.right = frame.left + 400;
			frame.bottom = frame.top + 300;
			frame.OffsetBy(60, 30);

			FilerRule* rule = fRuleList->ItemAt(
				fRuleItemList->CurrentSelection());

			RuleEditWindow* rulewin = new RuleEditWindow(frame, rule, this);
			rulewin->Show();
			break;
		}
		case MSG_ADD_RULE:
		{
			FilerRule* item;
			if (message->FindPointer("item", (void**)&item) == B_OK)
				AddRule(item);

			UpdateButtons();
			SaveRules(fRuleList);
			break;
		}
		case MSG_REMOVE_RULE:
		{
			int32 selection = fRuleItemList->CurrentSelection();
			if (selection < 0)
				break;

			RemoveRule(selection);

			int32 count = fRuleItemList->CountItems();
			fRuleItemList->Select((selection > count - 1) ? count - 1 : selection);

			UpdateButtons();
			SaveRules(fRuleList);
			break;
		}
		case MSG_UPDATE_RULE:
		{
			FilerRule* rule;
			if (message->FindPointer("item", (void**)&rule) == B_OK)
			{
				int64 id;
				if (message->FindInt64("id",&id) != B_OK)
					debugger("Couldn't find update ID");

				for (int32 i = 0; i < fRuleList->CountItems(); i++)
				{
					FilerRule* oldrule = fRuleList->ItemAt(i);
					if (oldrule->GetID() == id)
					{
						*oldrule = *rule;
						RuleItem* item = (RuleItem*)fRuleItemList->ItemAt(i);
						item->SetText(rule->GetDescription());
						break;
					}
				}
				delete rule;
			}

			SaveRules(fRuleList);
			break;
		}
		case MSG_RULE_SELECTED:
		{
			UpdateButtons();
			break;
		}
		case MSG_MOVE_RULE_UP:
		{
			int32 selection = fRuleItemList->CurrentSelection();
			if (selection < 1)
				break;

			fRuleItemList->SwapItems(selection, selection - 1);
			fRuleList->SwapItems(selection, selection - 1);

			UpdateButtons();
			SaveRules(fRuleList);
			break;
		}
		case MSG_MOVE_RULE_DOWN:
		{
			int32 selection = fRuleItemList->CurrentSelection();
			if (selection > fRuleItemList->CountItems() - 1)
				break;

			fRuleItemList->SwapItems(selection, selection + 1);
			fRuleList->SwapItems(selection, selection + 1);

			UpdateButtons();
			SaveRules(fRuleList);
			break;
		}
		default:
			BView::MessageReceived(message);
			break;
	}
}


void
RuleTab::UpdateButtons()
{
	int32 selection = fRuleItemList->CurrentSelection();
	int32 count = fRuleItemList->CountItems();

	if (selection < 0)
		count = -1;

	fEditButton->SetEnabled((count >= 0) ? true : false);
	fRemoveButton->SetEnabled((count >= 0) ? true : false);
	fMoveUpButton->SetEnabled((count > 1 && selection > 0) ? true : false);
	fMoveDownButton->SetEnabled((count > 1 && selection < count - 1) ? true : false);
}


void
RuleTab::AddRule(FilerRule* rule)
{
	fRuleList->AddItem(rule);
	fRuleItemList->AddItem(new RuleItem(rule));

	if (fRuleItemList->CurrentSelection() < 0)
		fRuleItemList->Select(0L);
}


void
RuleTab::RemoveRule(int32 selection)
{
	fRuleList->RemoveItemAt(selection);
	fRuleItemList->RemoveItem(selection);
}
