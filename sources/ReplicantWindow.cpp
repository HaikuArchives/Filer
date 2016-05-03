/*
 * Copyright 2016. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */


#include <ControlLook.h>
#include <LayoutBuilder.h>

#include "DropZoneTab.h"
#include "ReplicantWindow.h"


ReplicantWindow::ReplicantWindow(BRect frame)
	:
	BWindow(BRect(0, 0, 120, 100), ("Resize"),
		B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS)
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
