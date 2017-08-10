/*
 * Copyright 2017 Haiku, Inc.
 * Distributed under the terms of the MIT License.
 *
 * Author:
 *			Owen Pan <owen.pan@yahoo.com>
 */


#include "FolderPathView.h"

#include <Catalog.h>
#include <Cursor.h>
#include <MenuItem.h>
#include <Roster.h>
#include <Window.h>

#include <private/shared/LongAndDragTrackingFilter.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "FolderPathView"

static const uint32 kMouseLongDown = 'Mold';
static const uint32 kOpenFolder = 'Opfd';
static const uint32 kOpenFile = 'Opfl';


FolderPathView::FolderPathView(const BString& path, const entry_ref& ref)
	:
	BStringView("", path),
	fFileRef(ref)
{
	SetHighUIColor(B_LINK_TEXT_COLOR);
	get_ref_for_path(path, &fFolderRef);

	fPopupMenu = new BPopUpMenu("", false, false);
	fPopupMenu->AddItem(new BMenuItem(B_TRANSLATE("Open folder"),
		new BMessage(kOpenFolder)));
	fPopupMenu->AddItem(new BMenuItem(B_TRANSLATE("Open file"),
		new BMessage(kOpenFile)));
}


void
FolderPathView::AttachedToWindow()
{
	AddFilter(new LongAndDragTrackingFilter(kMouseLongDown, 0));
	fPopupMenu->SetTargetForItems(this);
}


void
FolderPathView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case kMouseLongDown:
		{
			BPoint where;
			if (msg->FindPoint("where", &where) == B_OK)
				fPopupMenu->Go(ConvertToScreen(where), true);

			break;
		}
		case kOpenFolder:
			_OpenFolder();
			break;
		case kOpenFile:
			be_roster->Launch(&fFileRef);
			break;
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
FolderPathView::MouseUp(BPoint where)
{
	if (Bounds().Contains(where))
		_OpenFolder();
}


void
FolderPathView::_OpenFolder()
{
	BMessage msgRef(B_REFS_RECEIVED);
	msgRef.AddRef("refs", &fFolderRef);

	BMessage msgSel('Tsel');
	msgSel.AddRef("refs", &fFileRef);

	BList msgList(2);
	msgList.AddItem(&msgRef);
	msgList.AddItem(&msgSel);

	be_roster->Launch("application/x-vnd.Be-TRAK", &msgList);
}
