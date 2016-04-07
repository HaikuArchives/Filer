/*
	PrefsWindow.cpp: Window class to show and edit settings for the Filer
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/
#include "PrefsWindow.h"
#include "FilerRule.h"
#include "RuleItem.h"
#include "RuleEditWindow.h"
#include "RuleRunner.h"

#include <Application.h>
#include <View.h>
#include <Alert.h>
#include <ScrollView.h>

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


PrefsWindow::PrefsWindow(void)
 :	BWindow(BRect(100,100,450,350),"Filer Settings",B_TITLED_WINDOW,
 			B_ASYNCHRONOUS_CONTROLS | B_NOT_ZOOMABLE),
 	fChanges(false)
{
	fRuleList = new BObjectList<FilerRule>(20,true);
	
	AddShortcut('a',B_COMMAND_KEY,new BMessage(M_SHOW_ADD_WINDOW));
	AddShortcut('e',B_COMMAND_KEY,new BMessage(M_SHOW_EDIT_WINDOW));
	
	BView *top = new BView(Bounds(),"top",B_FOLLOW_ALL,B_WILL_DRAW);
	top->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(top);
	
	BRect rect(Bounds().InsetByCopy(10,10));
	rect.right -= B_V_SCROLL_BAR_WIDTH;
	
	fRuleItemList = new BListView(rect,"rulelist",B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL);
	fScrollView = new BScrollView("listscroll",fRuleItemList,
												B_FOLLOW_ALL,0,true,true);
	fScrollView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	top->AddChild(fScrollView);
	fRuleItemList->SetSelectionMessage(new BMessage(M_RULE_SELECTED));
	fRuleItemList->SetInvocationMessage(new BMessage(M_SHOW_EDIT_WINDOW));
	fScrollView->ScrollBar(B_HORIZONTAL)->SetRange(0.0,0.0);
	
	fAddButton = new BButton(BRect(0,0,1,1),"addbutton","Add…",
									new BMessage(M_SHOW_ADD_WINDOW),
									B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	fAddButton->ResizeToPreferred();
	fAddButton->MoveTo(10,Bounds().bottom - 20 - (fAddButton->Bounds().IntegerHeight() * 2));
	top->AddChild(fAddButton);
	
	fScrollView->ResizeBy(0,(fAddButton->Bounds().IntegerHeight() * -2) - 20 - B_H_SCROLL_BAR_HEIGHT);
	
	fEditButton = new BButton(BRect(0,0,1,1),"editbutton","Edit…",
									new BMessage(M_SHOW_EDIT_WINDOW),
									B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	fEditButton->ResizeToPreferred();
	fEditButton->MoveTo((Bounds().Width() - fEditButton->Bounds().Width()) / 2.0,
					fAddButton->Frame().top);
	top->AddChild(fEditButton);
	fEditButton->SetEnabled(false);
	
	
	fRemoveButton = new BButton(BRect(0,0,1,1),"removebutton","Remove",
									new BMessage(M_REMOVE_RULE),
									B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	fRemoveButton->ResizeToPreferred();
	fRemoveButton->MoveTo(Bounds().Width() - fRemoveButton->Bounds().Width() - 10,
					fAddButton->Frame().top);
	top->AddChild(fRemoveButton);
	fRemoveButton->SetEnabled(false);
	
	
	fMoveDownButton = new BButton(BRect(0,0,1,1),"movedownbutton","Move Down",
									new BMessage(M_MOVE_RULE_DOWN),
									B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	fMoveDownButton->ResizeToPreferred();
	fMoveDownButton->MoveTo((Bounds().Width() / 2.0) + 10.0,
					fAddButton->Frame().bottom + 10.0);
	
	
	fMoveUpButton = new BButton(BRect(0,0,1,1),"moveupbutton","Move Up",
									new BMessage(M_MOVE_RULE_UP),
									B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	fMoveUpButton->ResizeTo(fMoveDownButton->Bounds().Width(), fMoveDownButton->Bounds().Height());
	fMoveUpButton->MoveTo((Bounds().Width() / 2.0) - fMoveUpButton->Bounds().Width() - 10.0,
					fAddButton->Frame().bottom + 10.0);
	
	top->AddChild(fMoveUpButton);
	top->AddChild(fMoveDownButton);
	
	fMoveUpButton->SetEnabled(false);
	fMoveDownButton->SetEnabled(false);
	
	
	float minwidth = (fRemoveButton->Bounds().Width() * 3.0) + 40;
	SetSizeLimits(minwidth, 30000, 200, 30000);

	LoadRules(fRuleList);
	
	for (int32 i = 0; i < fRuleList->CountItems(); i++)
		fRuleItemList->AddItem(new RuleItem(fRuleList->ItemAt(i)));
	
	fRuleItemList->MakeFocus();
	if (fRuleItemList->CountItems() > 0)
		fRuleItemList->Select(0L);
	else
	{
		BAlert *alert = new BAlert("Filer","It appears that there aren't any rules for "
											"organizing files. Would you like Filer to "
											"add some basic ones for you?","No","Yes");
		if (alert->Go() == 1)
		{
			FilerRule *rule = new FilerRule();
			
			// NOTE: If actions 
			rule->AddTest(MakeTest("Type","is","text/plain"));
			rule->AddAction(MakeAction("Move it to…","/boot/home/Documents"));
			rule->SetDescription("Store text files in my Documents folder");
			AddRule(rule);
			
			rule = new FilerRule();
			rule->AddTest(MakeTest("Type","is","application/pdf"));
			rule->AddAction(MakeAction("Move it to…","/boot/home/Documents"));
			rule->SetDescription("Store PDF files in my Documents folder");
			AddRule(rule);
			
			rule = new FilerRule();
			rule->AddTest(MakeTest("Type","starts with","image/"));
			rule->AddAction(MakeAction("Move it to…","/boot/home/Pictures"));
			rule->SetDescription("Store pictures in my Pictures folder");
			AddRule(rule);
			
			rule = new FilerRule();
			rule->AddTest(MakeTest("Type","starts with","video/"));
			rule->AddAction(MakeAction("Move it to…","/boot/home/Videos"));
			rule->SetDescription("Store movie files in my Videos folder");
			AddRule(rule);
			
			rule = new FilerRule();
			rule->AddTest(MakeTest("Name","ends with",".zip"));
			rule->AddAction(MakeAction("Terminal command…","unzip %FULLPATH% -d /boot/home/Desktop"));
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


PrefsWindow::~PrefsWindow(void)
{
	delete fRuleList;
}


bool
PrefsWindow::QuitRequested(void)
{
	if (fChanges)
		SaveRules(fRuleList);
	MakeEmpty();
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}


void
PrefsWindow::MessageReceived(BMessage *msg)
{
	switch(msg->what)
	{
		case M_SHOW_ADD_WINDOW:
		{
			BRect frame(Frame());
			frame.right = frame.left + 400;
			frame.bottom = frame.top + 300;
			frame.OffsetBy(20,20);
			
			RuleEditWindow *rulewin = new RuleEditWindow(frame,NULL);
			rulewin->Show();
			break;
		}
		case M_SHOW_EDIT_WINDOW:
		{
			BRect frame(Frame());
			frame.right = frame.left + 400;
			frame.bottom = frame.top + 300;
			frame.OffsetBy(20,20);
			
			FilerRule *rule = fRuleList->ItemAt(fRuleItemList->CurrentSelection());
				
			RuleEditWindow *rulewin = new RuleEditWindow(frame,rule);
			rulewin->Show();
			break;
		}
		case M_ADD_RULE:
		{
			fChanges = true;
			FilerRule *item;
			if (msg->FindPointer("item",(void**)&item) == B_OK)
				AddRule(item);
			break;
		}
		case M_REMOVE_RULE:
		{
			fChanges = true;
			if (fRuleItemList->CurrentSelection() >= 0)
				RemoveRule((RuleItem*)fRuleItemList->ItemAt(fRuleItemList->CurrentSelection()));
			break;
		}
		case M_UPDATE_RULE:
		{
			fChanges = true;
			FilerRule *rule;
			if (msg->FindPointer("item",(void**)&rule) == B_OK)
			{
				int64 id;
				if (msg->FindInt64("id",&id) != B_OK)
					debugger("Couldn't find update ID");
				
				for (int32 i = 0; i < fRuleList->CountItems(); i++)
				{
					FilerRule *oldrule = fRuleList->ItemAt(i);
					if (oldrule->GetID() == id)
					{
						*oldrule = *rule;
						RuleItem *item = (RuleItem*)fRuleItemList->ItemAt(i);
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
			
			if (fRuleItemList->CountItems() > 1)
			{
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
			BWindow::MessageReceived(msg);
			break;
	}
}


void
PrefsWindow::FrameResized(float width, float height)
{
	float x = fAddButton->Frame().right + ((fRemoveButton->Frame().left - 
											fAddButton->Frame().right) / 2.0);
	fEditButton->MoveTo(x - (fEditButton->Bounds().Width() / 2.0),
						fAddButton->Frame().top);
}


void
PrefsWindow::AddRule(FilerRule *rule)
{
	fRuleList->AddItem(rule);
	fRuleItemList->AddItem(new RuleItem(rule));
	
	if (fRuleItemList->CurrentSelection() < 0)
		fRuleItemList->Select(0L);
}


void
PrefsWindow::RemoveRule(RuleItem *item)
{
	// Select a new rule (if there is one) before removing the old one. BListView simply drops
	// the selection if the selected item is removed. What a pain in the neck. :/
	int32 itemindex = fRuleItemList->IndexOf(item);
	int32 selection = fRuleItemList->CurrentSelection();
	if (itemindex == selection && fRuleItemList->CountItems() > 1)
	{
		if (selection == fRuleItemList->CountItems() - 1)
			selection--;
		else
			selection++;
		fRuleItemList->Select(selection);
	}
	
	fRuleItemList->RemoveItem(item);
	
	FilerRule *rule = item->Rule();
	fRuleList->RemoveItem(rule);
	delete item;
	
	if (fRuleItemList->CountItems() <= 0)
	{
		fEditButton->SetEnabled(false);
		fRemoveButton->SetEnabled(false);
	}
	
	if (fRuleItemList->CountItems() < 2)
	{
		fMoveUpButton->SetEnabled(false);
		fMoveDownButton->SetEnabled(false);
	}
}


void
PrefsWindow::MakeEmpty(void)
{
	for (int32 i = fRuleItemList->CountItems() - 1; i >= 0; i--)
	{
		RuleItem *item = (RuleItem*)fRuleItemList->RemoveItem(i);
		delete item;
	}
}
