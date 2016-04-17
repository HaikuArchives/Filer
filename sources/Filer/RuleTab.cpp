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
#include "RuleEditWindow.h"
#include "RuleItem.h"
#include "RuleRunner.h"
#include "RuleTab.h"

enum 
{
	M_SHOW_ADD_WINDOW = 'shaw',
	M_SHOW_EDIT_WINDOW = 'shew',
	M_REMOVE_RULE = 'shrr',
	M_REVERT = 'rvrt',
	M_RULE_SELECTED = 'rlsl',
	M_MOVE_RULE_UP = 'mvup',
	M_MOVE_RULE_DOWN = 'mvdn'
};


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
			rule->AddAction(MakeAction("Move it to…", "/boot/home/Documents"));
			rule->SetDescription("Store text files in my Documents folder");
			AddRule(rule);

			rule = new FilerRule();
			rule->AddTest(MakeTest("Type", "is", "application/pdf"));
			rule->AddAction(MakeAction("Move it to…", "/boot/home/Documents"));
			rule->SetDescription("Store PDF files in my Documents folder");
			AddRule(rule);

			rule = new FilerRule();
			rule->AddTest(MakeTest("Type", "starts with", "image/"));
			rule->AddAction(MakeAction("Move it to…", "/boot/home/Pictures"));
			rule->SetDescription("Store pictures in my Pictures folder");
			AddRule(rule);

			rule = new FilerRule();
			rule->AddTest(MakeTest("Type", "starts with","video/"));
			rule->AddAction(MakeAction("Move it to…", "/boot/home/Videos"));
			rule->SetDescription("Store movie files in my Videos folder");
			AddRule(rule);

			rule = new FilerRule();
			rule->AddTest(MakeTest("Name", "ends with", ".zip"));
			rule->AddAction(MakeAction("Terminal command…",
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
	fRuleItemList = new BListView("rulelist", B_SINGLE_SELECTION_LIST,
		B_WILL_DRAW	| B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE | B_NAVIGABLE);
	fScrollView = new BScrollView("listscroll", fRuleItemList,
		B_FRAME_EVENTS | B_WILL_DRAW, false, true);

	fRuleItemList->SetSelectionMessage(new BMessage(M_RULE_SELECTED));
	fRuleItemList->SetInvocationMessage(new BMessage(M_SHOW_EDIT_WINDOW));
//	fScrollView->ScrollBar(B_HORIZONTAL)->SetRange(0.0, 0.0);
	
	fAddButton = new BButton("addbutton", "Add" B_UTF8_ELLIPSIS,
		new BMessage(M_SHOW_ADD_WINDOW));

	fEditButton = new BButton("editbutton", "Edit" B_UTF8_ELLIPSIS,
		new BMessage(M_SHOW_EDIT_WINDOW));
	fEditButton->SetEnabled(false);

	fRemoveButton = new BButton("removebutton", "Remove",
		new BMessage(M_REMOVE_RULE));
	fRemoveButton->SetEnabled(false);

	fMoveUpButton = new BButton("moveupbutton", "Move up",
		new BMessage(M_MOVE_RULE_UP));
	fMoveUpButton->SetEnabled(false);
	
	fMoveDownButton = new BButton("movedownbutton", "Move down",
		new BMessage(M_MOVE_RULE_DOWN));
	fMoveDownButton->SetEnabled(false);

	static const float spacing = be_control_look->DefaultItemSpacing();
	BLayoutBuilder::Group<>(this, B_HORIZONTAL, B_USE_DEFAULT_SPACING)
		.SetInsets(spacing)
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
		.End();
}


void
RuleTab::AttachedToWindow()
{
	fAddButton->SetTarget(this);
	fEditButton->SetTarget(this);
	fRemoveButton->SetTarget(this);
	fMoveUpButton->SetTarget(this);
	fMoveDownButton->SetTarget(this);
	fRuleItemList->SetTarget(this);

	if (fRuleItemList->CountItems() > 0) {
		BMessenger messenger(this);
		BMessage message(M_RULE_SELECTED);
		messenger.SendMessage(&message);
	}	
	BView::AttachedToWindow();
}


void
RuleTab::DetachedFromWindow()
{
	if (fChanges)
		SaveRules(fRuleList);
	MakeEmpty();
}


void
RuleTab::MessageReceived(BMessage* message)
{
//	message->PrintToStream();
	switch(message->what)
	{
		case M_SHOW_ADD_WINDOW:
		{
			printf("Show Add Window\n");
			BRect frame(Frame());
			ConvertToScreen(&frame);
			frame.right = frame.left + 400;
			frame.bottom = frame.top + 300;
			frame.OffsetBy(60, 30);

			RuleEditWindow* rulewin = new RuleEditWindow(frame, NULL);
			rulewin->Show();
			break;
		}
		case M_SHOW_EDIT_WINDOW:
		{
			BRect frame(Frame());
			ConvertToScreen(&frame);
			frame.right = frame.left + 400;
			frame.bottom = frame.top + 300;
			frame.OffsetBy(60, 30);

			FilerRule* rule = fRuleList->ItemAt(
				fRuleItemList->CurrentSelection());

			RuleEditWindow* rulewin = new RuleEditWindow(frame, rule);
			rulewin->Show();
			break;
		}
		case M_ADD_RULE:
		{
			fChanges = true;
			FilerRule* item;
			if (message->FindPointer("item", (void**)&item) == B_OK)
				AddRule(item);
			break;
		}
		case M_REMOVE_RULE:
		{
			fChanges = true;
			if (fRuleItemList->CurrentSelection() >= 0)
				RemoveRule((RuleItem*)fRuleItemList->ItemAt(
					fRuleItemList->CurrentSelection()));
			break;
		}
		case M_UPDATE_RULE:
		{
			fChanges = true;
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
			break;
		}
		case M_REVERT:
		{
			while (fRuleItemList->CountItems() > 0)
				RemoveRule((RuleItem*)fRuleItemList->ItemAt(0L));
			fRuleList->MakeEmpty();
			fEditButton->SetEnabled(false);
			fRemoveButton->SetEnabled(false);

			LoadRules(fRuleList);
			break;
		}
		case M_RULE_SELECTED:
		{
			bool value = (fRuleItemList->CurrentSelection() >= 0);

			fEditButton->SetEnabled(value);
			fRemoveButton->SetEnabled(value);

			if (fRuleItemList->CountItems() > 1) {
				fMoveUpButton->SetEnabled(value);
				fMoveDownButton->SetEnabled(value);
			}
			break;
		}
		case M_MOVE_RULE_UP:
		{
			fChanges = true;
			int32 selection = fRuleItemList->CurrentSelection();
			if (selection < 1)
				break;

			fRuleItemList->SwapItems(selection, selection - 1);
			fRuleList->SwapItems(selection, selection - 1);
			break;
		}
		case M_MOVE_RULE_DOWN:
		{
			fChanges = true;
			int32 selection = fRuleItemList->CurrentSelection();
			if (selection > fRuleItemList->CountItems() - 1)
				break;

			fRuleItemList->SwapItems(selection, selection + 1);
			fRuleList->SwapItems(selection, selection + 1);
			break;
		}
		default:
			BView::MessageReceived(message);
			break;
	}
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
RuleTab::RemoveRule(RuleItem* item)
{
	// Select a new rule (if there is one) before removing the old one. BListView simply drops
	// the selection if the selected item is removed. What a pain in the neck. :/
	int32 itemindex = fRuleItemList->IndexOf(item);
	int32 selection = fRuleItemList->CurrentSelection();
	if (itemindex == selection && fRuleItemList->CountItems() > 1) {
		if (selection == fRuleItemList->CountItems() - 1)
			selection--;
		else
			selection++;
		fRuleItemList->Select(selection);
	}

	fRuleItemList->RemoveItem(item);

	FilerRule* rule = item->Rule();
	fRuleList->RemoveItem(rule);
	delete item;

	if (fRuleItemList->CountItems() <= 0) {
		fEditButton->SetEnabled(false);
		fRemoveButton->SetEnabled(false);
	}

	if (fRuleItemList->CountItems() < 2) {
		fMoveUpButton->SetEnabled(false);
		fMoveDownButton->SetEnabled(false);
	}
}


void
RuleTab::MakeEmpty()
{
	for (int32 i = fRuleItemList->CountItems() - 1; i >= 0; i--)
	{
		RuleItem* item = (RuleItem*)fRuleItemList->RemoveItem(i);
		delete item;
	}
}
