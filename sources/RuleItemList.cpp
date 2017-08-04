/*
 * Copyright 2015-2016. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#include <Bitmap.h>
#include <Catalog.h>
#include <ControlLook.h>

#include "FilerDefs.h"
#include "RuleItem.h"
#include "RuleItemList.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ClipList"


RuleItemList::RuleItemList(const char* name, BHandler* caller)
	:
	BListView(name),
	fCaller(caller),
	fDropRect()
{
}


RuleItemList::~RuleItemList()
{
}


void
RuleItemList::AttachedToWindow()
{
	SetFlags(Flags() | B_FULL_UPDATE_ON_RESIZE | B_NAVIGABLE);

	BListView::AttachedToWindow();
}


void
RuleItemList::Draw(BRect rect)
{
	SetHighColor(ui_color(B_CONTROL_BACKGROUND_COLOR));

	BRect bounds(Bounds());
	BRect itemFrame = ItemFrame(CountItems() - 1);
	bounds.top = itemFrame.bottom;
	FillRect(bounds);

	BListView::Draw(rect);

	if (fDropRect.IsValid()) {
		SetHighColor(255, 0, 0, 255);
		StrokeRect(fDropRect);
	}
}


bool
RuleItemList::InitiateDrag(BPoint point, int32 dragIndex, bool wasSelected)
{
	RuleItem* sItem = dynamic_cast<RuleItem *> (ItemAt(CurrentSelection()));
	if (sItem == NULL) {
		// workaround for a timing problem (see Locale prefs)
		sItem = dynamic_cast<RuleItem *> (ItemAt(dragIndex));
		Select(dragIndex);
		if (sItem == NULL)
			return false;
	}
	BMessage message(MSG_RULE_DRAGGED);
	int32 index = CurrentSelection();
	message.AddInt32("index", index);

	BRect dragRect(0.0f, 0.0f, Bounds().Width(), sItem->Height());
	BBitmap* dragBitmap = new BBitmap(dragRect, B_RGB32, true);
	if (dragBitmap->IsValid()) {
		BView* view = new BView(dragBitmap->Bounds(), "helper", B_FOLLOW_NONE,
			B_WILL_DRAW);
		dragBitmap->AddChild(view);
		dragBitmap->Lock();

		sItem->DrawItem(view, dragRect);
		view->SetHighColor(0, 0, 0, 255);
		view->StrokeRect(view->Bounds());
		view->Sync();

		dragBitmap->Unlock();
	} else {
		delete dragBitmap;
		dragBitmap = NULL;
	}

	if (dragBitmap != NULL)
		DragMessage(&message, dragBitmap, B_OP_ALPHA, BPoint(0.0, 0.0));
	else
		DragMessage(&message, dragRect.OffsetToCopy(point), this);

	return true;
}

//
//void
//RuleItemList::MakeFocus(bool focused)
//{
//	BListView::MakeFocus(focused);
//
//	// only signal RuleItemList is focused when gaining focus
//	if (focused)
//		my_app->fMainWindow->SetHistoryActiveFlag(!focused);
//
//}


void
RuleItemList::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_RULE_DRAGGED:
		{
			int32 origIndex;
			int32 dropIndex;
			BPoint dropPoint;

			if (message->FindInt32("index", &origIndex) != B_OK)
				origIndex = CountItems() - 1; //
			dropPoint = message->DropPoint();
			dropIndex = IndexOf(ConvertFromScreen(dropPoint));
			if (dropIndex > origIndex)
				dropIndex = dropIndex - 1;
			if (dropIndex < 0)
				dropIndex = CountItems() - 1; // move to bottom

			BMessage msg;
			BMessenger msgr(fCaller);
			msg.what = MSG_RULE_MOVE;
			msg.AddInt32("origindex", origIndex);
			msg.AddInt32("dropindex", dropIndex);
			msgr.SendMessage(&msg, fCaller);
			break;
		}
//		case POPCLOSED:
//		{
//			fShowingPopUpMenu = false;
//			break;
//		}
		default:
		{
			BListView::MessageReceived(message);
			break;
		}
	}
}


//void
//RuleItemList::KeyDown(const char* bytes, int32 numBytes)
//{
//	switch (bytes[0]) {
//		case B_DELETE:
//		{
//			BMessage message(DELETE);
//			Looper()->PostMessage(&message);
//			break;
//		}
//		case B_LEFT_ARROW:
//		{
//			if (my_app->fMainWindow->Lock())
//				my_app->fMainWindow->fHistory->MakeFocus(true);
//			my_app->fMainWindow->Unlock();
//			break;
//		}
//		default:
//		{
//			BListView::KeyDown(bytes, numBytes);
//			break;
//		}
//	}
//}


//void
//RuleItemList::MouseDown(BPoint position)
//{
//	MakeFocus(true);
//
//	BRect bounds(Bounds());
//	BRect itemFrame = ItemFrame(CountItems() - 1);
//	bounds.top = itemFrame.bottom;
//	if (bounds.Contains(position))
//		return;
//
//	uint32 buttons = 0;
//	if (Window() != NULL && Window()->CurrentMessage() != NULL)
//		buttons = Window()->CurrentMessage()->FindInt32("buttons");
//
//	if ((buttons & B_SECONDARY_MOUSE_BUTTON) != 0) {
//		Select(IndexOf(position));
//		_ShowPopUpMenu(ConvertToScreen(position));
//		return;
//	}
//	BListView::MouseDown(position);
//}


void
RuleItemList::MouseUp(BPoint position)
{
	fDropRect = BRect(-1, -1, -1, -1);
	Invalidate();

	BListView::MouseUp(position);
}


void
RuleItemList::MouseMoved(BPoint where, uint32 transit, const BMessage* dragMessage)
{
	if (dragMessage != NULL) {
		switch (transit) {
			case B_ENTERED_VIEW:
			case B_INSIDE_VIEW:
			{
				int32 index = IndexOf(where);
				if (index < 0)
					index = CountItems();

				fDropRect = ItemFrame(index);
				if (fDropRect.IsValid()) {
					fDropRect.top = fDropRect.top -1;
					fDropRect.bottom = fDropRect.top + 1;
				} else {
					fDropRect = ItemFrame(index - 1);
					if (fDropRect.IsValid())
						fDropRect.top = fDropRect.bottom - 1;
					else {
						// empty view, show indicator at top
						fDropRect = Bounds();
						fDropRect.bottom = fDropRect.top + 1;
					}
				}
				Invalidate();
				break;
			}
			case B_EXITED_VIEW:
			{
				fDropRect = BRect(-1, -1, -1, -1);
				Invalidate();
				break;
			}
		}
	}
	BListView::MouseMoved(where, transit, dragMessage);
}


// #pragma mark - Member Functions

//
//void
//RuleItemList::_ShowPopUpMenu(BPoint screen)
//{
//	if (fShowingPopUpMenu)
//		return;
//
//	ContextPopUp* menu = new ContextPopUp("PopUpMenu", this);
//	BMessage* msg = NULL;
//
//	msg = new BMessage(PASTE_SPRUNGE);
//	BMenuItem* item = new BMenuItem(B_TRANSLATE("Paste to Sprunge.us"), msg, 'P');
//	menu->AddItem(item);
//
//	msg = new BMessage(EDIT_TITLE);
//	item = new BMenuItem(B_TRANSLATE("Edit title"), msg, 'E');
//	menu->AddItem(item);
//
//	msg = new BMessage(DELETE);
//	item = new BMenuItem(B_TRANSLATE("Remove"), msg);
//	menu->AddItem(item);
//
//	menu->SetTargetForItems(Looper());
//	menu->Go(screen, true, true, true);
//	fShowingPopUpMenu = true;
//}
