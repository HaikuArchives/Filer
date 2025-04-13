/*
 * Copyright 2016. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */


#include <Catalog.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>

#include "DropZoneTab.h"
#include "ReplicantWindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ReplicantWindow"


ReplicantWindow::ReplicantWindow(BRect frame)
	:
	BWindow(BRect(0, 0, 120, 100), NULL,
	B_DOCUMENT_WINDOW_LOOK, B_FLOATING_APP_WINDOW_FEEL,
	B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS)
{
	CenterIn(frame);
	DropZone* dropzone = new DropZone(true);

	static const float spacing = be_control_look->DefaultItemSpacing();
	float labelWidth = be_plain_font->StringWidth(B_TRANSLATE_SYSTEM_NAME("Filer"));
	font_height fontHeight;
	be_plain_font->GetHeight(&fontHeight);
	float labelHeight = ceilf(fontHeight.ascent + fontHeight.descent);

	dropzone->SetExplicitMinSize(BSize(labelWidth + spacing * 2, labelHeight + spacing * 2));
	dropzone->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(spacing, spacing, spacing, spacing * 1.5)
		.Add(dropzone);
}


ReplicantWindow::~ReplicantWindow()
{
}


//bool
//ReplicantWindow::QuitRequested()
//{
//	be_app->PostMessage(B_QUIT_REQUESTED);
//	return true;
//}
