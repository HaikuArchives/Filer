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
	frame.OffsetBy(290.0, 130.0);
	MoveTo(frame.LeftTop());

	DropZone* dropzone = new DropZone(true);
	dropzone->SetExplicitMinSize(BSize(70.0, 64.0));
	dropzone->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));

	static const float spacing = be_control_look->DefaultItemSpacing();
	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(spacing)
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
