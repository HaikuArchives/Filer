/*
 * Copyright 2008, 2016. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *  DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
 *	Humdinger, humdingerb@gmail.com
 */
#ifndef DROPZONETAB_H
#define DROPZONETAB_H

#include <Button.h>
#include <View.h>

class DropZoneTab : public BView
{
public:
					DropZoneTab();
					~DropZoneTab();

	virtual void	AttachedToWindow();
	void			MessageReceived(BMessage* message);
	
private:
	void			_BuildLayout();

	BButton*		fRepliButton;
	BView*			fDropzone;
};


#endif // DROPZONETAB_H
