/*
 * Copyright 2017 Haiku, Inc.
 * Distributed under the terms of the MIT License.
 *
 * Author:
 *			Owen Pan <owen.pan@yahoo.com>
 */

#ifndef FOLDER_PATH_VIEW_H
#define FOLDER_PATH_VIEW_H

#include <Entry.h>
#include <PopUpMenu.h>
#include <StringView.h>

class FolderPathView : public BStringView
{
	void	AttachedToWindow();
	void	MessageReceived(BMessage* msg);
	void	MouseMoved(BPoint where, uint32 transit, const BMessage* dragMsg);
	void	MouseUp(BPoint where);

	void	_OpenFolder();

	entry_ref	fFolderRef;
	entry_ref	fFileRef;
	BPopUpMenu*	fPopupMenu;

public:
			FolderPathView(const BString& path, const entry_ref& ref);
			~FolderPathView() { delete fPopupMenu; }
};

#endif	// FOLDER_PATH_VIEW_H
