/*
 * Copyright 2015-2016. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#ifndef AUTOFILERLIST_H
#define AUTOFILERLIST_H

#include <ListView.h>


class AutoFilerList : public BListView {
public:
					AutoFilerList(const char* name, BHandler* caller);
					~AutoFilerList();

	virtual void	AttachedToWindow();
	virtual	void	MessageReceived(BMessage* message);

	virtual	void	KeyDown(const char* bytes, int32 numBytes);
	void			MouseDown(BPoint position);
	void			MouseUp(BPoint position);

private:
	void			_ShowPopUpMenu(BPoint screen);

	bool			fShowingPopUpMenu;
	BHandler*		fCaller;
};

#endif // AUTOFILERLIST_H
