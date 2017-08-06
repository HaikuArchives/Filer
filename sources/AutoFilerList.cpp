/*
 * Copyright 2015-2017. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#include <Bitmap.h>
#include <Catalog.h>
#include <ControlLook.h>
#include <MenuItem.h>

#include "ContextPopUp.h"
#include "FilerDefs.h"
#include "FilerRule.h"
#include "MainWindow.h"
#include "RuleItem.h"
#include "AutoFilerList.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "RuleTab"


AutoFilerList::AutoFilerList(const char* name, BHandler* caller)
	:
	BListView(name),
	fCaller(caller)
{
}


AutoFilerList::~AutoFilerList()
{
}


void
AutoFilerList::AttachedToWindow()
{
	SetFlags(Flags() | B_FULL_UPDATE_ON_RESIZE | B_NAVIGABLE);

	BListView::AttachedToWindow();
}


void
AutoFilerList::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_POPUP_CLOSED:
		{
			fShowingPopUpMenu = false;
			break;
		}
		default:
		{
			BListView::MessageReceived(message);
			break;
		}
	}
}


void
AutoFilerList::KeyDown(const char* bytes, int32 numBytes)
{
	switch (bytes[0]) {
		case B_DELETE:
		{
			BMessage msg;
			BMessenger msgr(fCaller);
			msg.what = MSG_REMOVE_FOLDER;
			msgr.SendMessage(&msg, fCaller);
			break;
		}
		default:
		{
			BListView::KeyDown(bytes, numBytes);
			break;
		}
	}
}


void
AutoFilerList::MouseDown(BPoint position)
{
	BRect bounds(Bounds());
	BRect itemFrame = ItemFrame(CountItems() - 1);
	bounds.top = itemFrame.bottom;
	if (bounds.Contains(position))
		DeselectAll();

	uint32 buttons = 0;
	if (Window() != NULL && Window()->CurrentMessage() != NULL)
		buttons = Window()->CurrentMessage()->FindInt32("buttons");

	if ((buttons & B_SECONDARY_MOUSE_BUTTON) != 0) {
		Select(IndexOf(position));
		_ShowPopUpMenu(ConvertToScreen(position));
		return;
	}
	BListView::MouseDown(position);
}


void
AutoFilerList::MouseUp(BPoint position)
{
	Invalidate();

	BListView::MouseUp(position);
}


// #pragma mark - Member Functions


void
AutoFilerList::_ShowPopUpMenu(BPoint screen)
{
	if (fShowingPopUpMenu)
		return;

	ContextPopUp* menu = new ContextPopUp("PopUpMenu", this);
	BMessage* msg = NULL;
	BMenuItem* item;

	msg = new BMessage(MSG_SHOW_ADD_PANEL);
	item = new BMenuItem(B_TRANSLATE("Add"), msg);
	menu->AddItem(item);

	if (CurrentSelection() >= 0) {
		msg = new BMessage(MSG_REMOVE_FOLDER);
		item = new BMenuItem(B_TRANSLATE("Remove"), msg);
		menu->AddItem(item);
	}

	menu->SetTargetForItems(fCaller);
	menu->Go(screen, true, true, true);
	fShowingPopUpMenu = true;
}
