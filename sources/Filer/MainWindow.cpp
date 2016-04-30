/*
 * Copyright 2016. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#include <Catalog.h>
#include <ControlLook.h>
#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <FindDirectory.h>
#include <LayoutBuilder.h>
#include <Path.h>
#include <Roster.h>
#include <Screen.h>

#include "MainWindow.h"


MainWindow::MainWindow()
	:
	BWindow(BRect(50, 50, 400, 300), ("Filer"),
		B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS)
{
	_BuildLayout();
}


MainWindow::~MainWindow()
{
}


bool
MainWindow::QuitRequested()
{
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
