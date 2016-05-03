/*
 * Copyright 2016. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#ifndef DROPZONETAB_H
#define DROPZONETAB_H

#include <Button.h>
#include <StringView.h>
#include <View.h>

const pattern stripePattern = {0xcc, 0x66, 0x33, 0x99, 0xcc, 0x66, 0x33, 0x99};

class DropZone : public BView
{
public:
							DropZone(bool replicatable = true);
							DropZone(BMessage* data);
							~DropZone();

	static 	BArchivable* 	Instantiate(BMessage* archive);
	virtual status_t 		Archive(BMessage* data, bool deep = true) const;

	virtual void			Draw(BRect rect);
	void					MessageReceived(BMessage* msg);

	void					_Init();

private:
	bool					fReplicated;
	BStringView*			fLabel1;
	BStringView* 			fLabel2;
};


class DropZoneTab : public BView
{
public:
					DropZoneTab();
					~DropZoneTab();

	virtual void	AttachedToWindow();
	void			MessageReceived(BMessage* msg);

	
private:
	DropZone*		fDropzone;
	BButton*		fRepliButton;
};

#endif // DROPZONETAB_H
