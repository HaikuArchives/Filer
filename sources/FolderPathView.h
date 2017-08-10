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
	void	MessageReceived(BMessage* msg);
	void	MouseMoved(BPoint where, uint32 transit, const BMessage* dragMsg);
	void	MouseDown(BPoint position);

	void	MouseUp(BPoint where);

	void	_OpenFolder();
	void	_ShowPopUpMenu(BPoint screen);

	entry_ref	fFolderRef;
	entry_ref	fFileRef;
	bool		fShowingPopUpMenu;

public:
			FolderPathView(const BString& path, const entry_ref& ref);
			~FolderPathView();
};

#endif	// FOLDER_PATH_VIEW_H
