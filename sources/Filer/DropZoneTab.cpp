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
