/*
 * Copyright 2016. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#include <ControlLook.h>
#include <LayoutBuilder.h>
#include <StringView.h>
#include <View.h>

#include "DropZoneTab.h"
#include "main.h"


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
DropZone::MessageReceived(BMessage* msg)
{
	if (msg->WasDropped()) {
		BMessenger messenger(be_app);
		msg->what = B_REFS_RECEIVED;
		messenger.SendMessage(msg);
	}

	BView::MessageReceived(msg);
}


DropZoneTab::DropZoneTab()
	:
	BView("Drop zone", B_SUPPORTS_LAYOUT)
{
	BStringView* zoneLabel = new BStringView("zonelabel",
		"Drag and drop the files to be processed below.");
	zoneLabel->SetAlignment(B_ALIGN_CENTER);
	fDropzone = new DropZone();

	static const float spacing = be_control_look->DefaultItemSpacing();
	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(spacing)
		.Add(zoneLabel)
		.Add(fDropzone);
}


DropZoneTab::~DropZoneTab()
{
}
