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

enum 
{
	M_REPLICATE = 'repl'
};


DropZoneTab::DropZoneTab()
	:
	BView("Drop zone", B_SUPPORTS_LAYOUT)
{
	_BuildLayout();
}


DropZoneTab::~DropZoneTab()
{
}


void
DropZoneTab::_BuildLayout()
{
	BStringView* zoneLabel = new BStringView("zonelabel",
		"Drag and drop the files to be processed below.");
	zoneLabel->SetAlignment(B_ALIGN_CENTER);
	fDropzone = new BView("dropzone", B_SUPPORTS_LAYOUT);
	
	fRepliButton = new BButton("replibutton",
		"Replicate!", new BMessage(M_REPLICATE));

	static const float spacing = be_control_look->DefaultItemSpacing();
	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(spacing)
		.Add(zoneLabel)
		.Add(fDropzone)
		.Add(fRepliButton);
}


void
DropZoneTab::AttachedToWindow()
{
//	SetFlags(Flags() | B_FULL_UPDATE_ON_RESIZE | B_NAVIGABLE);

	fRepliButton->SetTarget(this);
//	fDropzone->SetTarget(this);

	BView::AttachedToWindow();
}


void
DropZoneTab::MessageReceived(BMessage *message)
{
	switch (message->what)
	{
		case M_REPLICATE:
		{
			break;
		}
		default:
			BView::MessageReceived(message);
	}
}
