/*
 * Copyright 2016. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#include <Messenger.h>

#include "main.h"
#include "DropZone.h"


DropZone::DropZone()
	:
	BView("dropzone", B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE)
{
}


DropZone::~DropZone()
{
}


void
DropZone::Draw(BRect rect)
{
	SetDrawingMode(B_OP_ALPHA);

	SetHighColor(tint_color(ViewColor(), B_DARKEN_2_TINT));
	SetLowColor(0, 0, 0, 0);

	BRect bounds = Bounds();
	StrokeRect(bounds);
	FillRect(bounds.InsetBySelf(3, 3), stripePattern);

	BView::Draw(rect);
}


void
DropZone::MessageReceived(BMessage* message)
{
	if (message->WasDropped()) {
		BMessenger messenger(be_app);
		message->what = B_REFS_RECEIVED;
		messenger.SendMessage(message);
	}

	BView::MessageReceived(message);
}
