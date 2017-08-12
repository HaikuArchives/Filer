/*
 * Copyright 2017 Haiku, Inc.
 * Distributed under the terms of the MIT License.
 *
 * Author:
 *			Owen Pan <owen.pan@yahoo.com>
 */

#include "ConflictWindow.h"
#include "ContextPopUp.h"
#include "FilerDefs.h"
#include "FolderPathView.h"

#include <Catalog.h>
#include <Cursor.h>
#include <MenuItem.h>
#include <NodeInfo.h>
#include <Path.h>
#include <Roster.h>
#include <Window.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "FolderPathView"

static const uint32 kOpenFolder = 'Opfd';
static const uint32 kOpenFile = 'Opfl';


FolderPathView::FolderPathView(const BString& path, const entry_ref& ref)
	:
	BStringView("", path),
	fFileRef(ref),
	fShowingPopUpMenu(false)
{
	SetHighUIColor(B_LINK_TEXT_COLOR);
	get_ref_for_path(path, &fFolderRef);
}


FolderPathView::~FolderPathView()
{
}


void
FolderPathView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case MSG_POPUP_CLOSED:
		{
			fShowingPopUpMenu = false;
			break;
		}
		case kOpenFolder:
		{
			_OpenFolder();
			break;
		}
		case kOpenFile:
		{
			be_roster->Launch(&fFileRef);
			break;
		}
		default:
			BStringView::MessageReceived(msg);
	}
}


void
FolderPathView::MouseMoved(BPoint where, uint32 transit, const BMessage* msg)
{
	switch (transit) {
		case B_ENTERED_VIEW:
		{
			const BCursor cursor(B_CURSOR_ID_FOLLOW_LINK);
			SetViewCursor(&cursor);

			SetHighUIColor(B_LINK_HOVER_COLOR);
			Invalidate();
			break;
		}
		case B_EXITED_VIEW:
		{
			const BCursor cursor(B_CURSOR_ID_SYSTEM_DEFAULT);
			SetViewCursor(&cursor);

			SetHighUIColor(B_LINK_TEXT_COLOR);
			Invalidate();
		}
	}
}


void
FolderPathView::MouseDown(BPoint position)
{
	uint32 buttons = 0;
	if (Window() != NULL && Window()->CurrentMessage() != NULL)
		buttons = Window()->CurrentMessage()->FindInt32("buttons");

	if ((buttons & B_SECONDARY_MOUSE_BUTTON) != 0) {
		_ShowPopUpMenu(ConvertToScreen(position));
		return;
	}
	BStringView::MouseDown(position);
}


void
FolderPathView::MouseUp(BPoint where)
{
	if (Bounds().Contains(where))
		_OpenFolder();
}


void
FolderPathView::_OpenFolder()
{
	BEntry selected(&fFileRef);
	if (!selected.Exists())
		return;

	// Ask Tracker to open the containing folder and select the
	// file inside it.
	BMessenger trackerMessenger("application/x-vnd.Be-TRAK");

	if (trackerMessenger.IsValid()) {
		BMessage selectionCommand(B_REFS_RECEIVED);
		status_t status = selectionCommand.AddRef("refs", &fFolderRef);

		node_ref selectedRef;
		if (selected.GetNodeRef(&selectedRef) == B_OK) {
			selectionCommand.AddData("nodeRefToSelect", B_RAW_TYPE,
				(void*)&selectedRef, sizeof(node_ref));
		}
		trackerMessenger.SendMessage(&selectionCommand);

		snooze(200000);	// wait 0.2 sec for large folders to populate
		BMessage select('Tsel');
		select.AddRef("refs", &fFileRef);
		trackerMessenger.SendMessage(&select);
	}
}


void
FolderPathView::_ShowPopUpMenu(BPoint screen)
{
	if (fShowingPopUpMenu)
		return;

	ContextPopUp* menu = new ContextPopUp("PopUpMenu", this);
	BMessage* msg = NULL;
	BMenuItem* item;

	msg = new BMessage(kOpenFile);
	item = new BMenuItem(B_TRANSLATE("Open file"), msg);
	menu->AddItem(item);

	msg = new BMessage(kOpenFolder);
	item = new BMenuItem(B_TRANSLATE("Open folder"), msg);
	menu->AddItem(item);

	menu->SetTargetForItems(this);
	menu->Go(screen, true, true, true);
	fShowingPopUpMenu = true;
}
