/*
 * Copyright 2016. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#include <File.h>
#include <FindDirectory.h>
#include <LayoutBuilder.h>
#include <Path.h>
#include <Screen.h>

#include "MainWindow.h"


MainWindow::MainWindow()
	:
	BWindow(BRect(), ("Filer"),
		B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS)
{
	_BuildLayout();

	LoadSettings();

	MoveTo(fPosition.left, fPosition.top);
	ResizeTo(fPosition.Width(), fPosition.Height());

	if (fPosition == BRect(-1, -1, -1, -1)) {
		CenterOnScreen();
		ResizeTo(350, 250);
	} else {
		// make sure window is on screen
		BScreen screen(this);
		if (!screen.Frame().InsetByCopy(10, 10).Intersects(Frame()))
			CenterOnScreen();
	}

	fTabView->Select(fTabSelection);
}


MainWindow::~MainWindow()
{
}


bool
MainWindow::QuitRequested()
{
	SaveSettings();

	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}


void
MainWindow::_BuildLayout()
{
	// The tabview
	fTabView = new BTabView("tabview", B_WIDTH_FROM_WIDEST);
	fTabView->SetBorder(B_NO_BORDER);
	fTabView->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	fDropZone = new DropZoneTab();
	fRules = new RuleTab();
	fAutoFiler = new AutoFilerTab();
	fHelp = new HelpTab();

	fTabView->AddTab(fDropZone);
	fTabView->AddTab(fRules);
	fTabView->AddTab(fAutoFiler);
	fTabView->AddTab(fHelp);

	// do the layouting
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.AddGroup(B_VERTICAL)
			.SetInsets(0, B_USE_DEFAULT_SPACING, 0, 0)
			.Add(fTabView)
		.End();

	fTabView->SetViewColor(B_TRANSPARENT_COLOR);
}


void
MainWindow::MessageReceived(BMessage* msg)
{
//	msg->PrintToStream();
	switch (msg->what)
	{
		default:
		{
			BWindow::MessageReceived(msg);
			break;
		}
	}
}

void
MainWindow::LoadSettings()
{
	fPosition.Set(-1, -1, -1, -1);
	fTabSelection = 0;

	BPath path;
	BMessage msg;

	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) == B_OK) {
		status_t ret = path.Append(kSettingsFolder);
		if (ret == B_OK) {
			path.Append(kSettingsFile);
			BFile file(path.Path(), B_READ_ONLY);

			if ((file.InitCheck() == B_OK) && (msg.Unflatten(&file) == B_OK)) {
				if (msg.FindRect("pos", &fPosition) != B_OK)
					fPosition.Set(-1, -1, -1, -1);
				if (msg.FindInt32("tab", &fTabSelection) != B_OK)
					fTabSelection = 0;
			}
		}
	}
}


void
MainWindow::SaveSettings()
{
	BRect pos = Frame();
	int32 tab = fTabView->Selection();
	if (pos == fPosition && tab == fTabSelection)
		return;

	BPath path;
	BMessage msg;

	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) < B_OK)
		return;
	status_t ret = path.Append(kSettingsFolder);

	if (ret == B_OK)
		ret = create_directory(path.Path(), 0777);

	if (ret == B_OK)
		path.Append(kSettingsFile);

	if (ret == B_OK) {
		BFile file(path.Path(), B_WRITE_ONLY | B_CREATE_FILE);
		ret = file.InitCheck();

		if (ret == B_OK) {
			msg.AddRect("pos", pos);
			msg.AddInt32("tab", tab);
			msg.Flatten(&file);
		}
	}
}
