/*
 * Copyright 2008, 2016. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *  DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
 *	Humdinger, humdingerb@gmail.com
 *	Owen Pan <owen.pan@yahoo.com>, 2017
 */
#include <Alert.h>
#include <Application.h>
#include <Catalog.h>
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

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "RuleTab"

static const char* const kDisable = B_TRANSLATE("Disable");


RuleTab::RuleTab()
	:
	BView(B_TRANSLATE("Rules"), B_SUPPORTS_LAYOUT)
{
	fRulePos.Set(-1, -1, -1, -1);
	_BuildLayout();

	fRuleList = new BObjectList<FilerRule>(20, true);
	fRuleList = static_cast<App*>(be_app)->GetRuleList();

	if (fRuleList->CountItems() == 0) {
		BAlert* alert = new BAlert("Filer",
			B_TRANSLATE("It appears that there aren't any rules for "
			"organizing files. Would you like Filer to "
			"add some basic ones for you?"),
			B_TRANSLATE("Cancel"), B_TRANSLATE("Add rules"));

		if (alert->Go() == 1) {
			AddDefaultRules(fRuleList);
			SaveRules(fRuleList);
		}
	}

	for (int32 i = 0; i < fRuleList->CountItems(); i++)
		fRuleItemList->AddItem(new RuleItem(fRuleList->ItemAt(i)));

	fRuleItemList->MakeFocus();
	if (fRuleItemList->CountItems() > 0)
		fRuleItemList->Select(0L);
}


RuleTab::~RuleTab()
{
	delete fRuleList;
	delete fRuleItemList;
	delete fScrollView;

	delete fMatchBox;
	delete fEditButton;
	delete fDisableButton;
	delete fAddRemoveButtons;
}


void
RuleTab::_BuildLayout()
{
	fMatchBox = new BCheckBox("matchoncebox",
		B_TRANSLATE("Apply only the first matching rule"),
		new BMessage(MSG_MATCH_ONCE));

	fRuleItemList = new BListView("rulelist", B_SINGLE_SELECTION_LIST,
		B_WILL_DRAW	| B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE | B_NAVIGABLE);
	fScrollView = new BScrollView("listscroll", fRuleItemList,
		B_FRAME_EVENTS | B_WILL_DRAW, false, true);

	fRuleItemList->SetSelectionMessage(new BMessage(MSG_RULE_SELECTED));
	fRuleItemList->SetInvocationMessage(new BMessage(MSG_SHOW_EDIT_WINDOW));
	
	fEditButton = new BButton("editbutton", B_TRANSLATE("Edit" B_UTF8_ELLIPSIS),
		new BMessage(MSG_SHOW_EDIT_WINDOW));
	fEditButton->SetEnabled(false);

	fDisableButton = new BButton("disablebutton", kDisable,
		new BMessage(MSG_DISABLE_RULE));
	fDisableButton->SetEnabled(false);

	float height;
	fEditButton->GetPreferredSize(NULL, &height);

	fAddRemoveButtons = new AddRemoveButtons(MSG_SHOW_ADD_WINDOW,
		MSG_REMOVE_RULE, this, height, B_USE_DEFAULT_SPACING, "Add rule",
		"Remove rule");
	fAddRemoveButtons->SetRemoveEnabled(false);

	fMoveUpButton = new BButton("moveupbutton", B_TRANSLATE("Move up"),
		new BMessage(MSG_MOVE_RULE_UP));
	fMoveUpButton->SetEnabled(false);
	
	fMoveDownButton = new BButton("movedownbutton", B_TRANSLATE("Move down"),
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
					.Add(fMoveUpButton)
					.Add(fMoveDownButton)
					.AddGlue()
				.End()
			.End()
			.AddGroup(B_VERTICAL)
				.Add(fEditButton)
				.Add(fDisableButton)
				.Add(fAddRemoveButtons)
				.AddGlue()
			.End()
		.End();
}


void
RuleTab::AttachedToWindow()
{
	fMatchBox->SetTarget(Looper());
	fEditButton->SetTarget(this);
	fDisableButton->SetTarget(this);
	fAddRemoveButtons->SetTarget(this);
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
			RuleEditWindow* rulewin = new RuleEditWindow(fRulePos, NULL, this);
			break;
		}
		case MSG_SHOW_EDIT_WINDOW:
		{
			FilerRule* rule = fRuleList->ItemAt(
				fRuleItemList->CurrentSelection());

			RuleEditWindow* rulewin = new RuleEditWindow(fRulePos, rule, this);
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
		case MSG_DISABLE_RULE:
		{
			int32 selection = fRuleItemList->CurrentSelection();

			fRuleList->ItemAt(selection)->Toggle();

			UpdateButtons();
			SaveRules(fRuleList);
			fRuleItemList->InvalidateItem(selection);
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
				if (message->FindInt64("id", &id) != B_OK)
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
	fAddRemoveButtons->SetRemoveEnabled((count >= 0) ? true : false);
	fMoveUpButton->SetEnabled((count > 1 && selection > 0) ? true : false);
	fMoveDownButton->SetEnabled((count > 1 && selection < count - 1) ? true : false);

	if (selection < 0) {
		fDisableButton->SetEnabled(false);
		fDisableButton->SetLabel(kDisable);
	} else {
		fDisableButton->SetEnabled(true);
		fDisableButton->SetLabel(fRuleList->ItemAt(selection)->Disabled() ?
			B_TRANSLATE("Enable") : kDisable);
	}
}


void
RuleTab::UpdateRuleWindowPos(BRect pos)
{
	fRulePos = pos;
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
