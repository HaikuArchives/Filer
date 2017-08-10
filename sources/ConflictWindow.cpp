/*
 * Copyright 2017 Haiku, Inc.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *			Owen Pan <owen.pan@yahoo.com>
 *			Humdinger <humdingerb@gmail.com>
 */


#include "ConflictWindow.h"

#include <Application.h>
#include <Button.h>
#include <Catalog.h>
#include <DateTimeFormat.h>
#include <Directory.h>
#include <LayoutBuilder.h>
#include <NodeInfo.h>
#include <Roster.h>


#include <private/shared/StringForSize.h>

#include "FolderPathView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ConflictWindow"

static const uint32 kReplace = 'RPLC';
static const uint32 kSkip = 'SKIP';


static BStringView*
CreateLightString(const char* str)
{
	BStringView* view = new BStringView("", str);

	view->SetHighColor(mix_color(ui_color(B_PANEL_BACKGROUND_COLOR),
		ui_color(B_PANEL_TEXT_COLOR), 192));

	return view;
}


ConflictWindow::ConflictWindow(const char* srcFolder, const entry_ref& srcFile,
	const char* destFolder, const entry_ref& destFile, const char* desc)
	:
	BWindow(BRect(0, 0, 0, 0), B_TRANSLATE("Filer: Conflict"),
		B_FLOATING_WINDOW_LOOK, B_FLOATING_ALL_WINDOW_FEEL,
		B_NOT_CLOSABLE | B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_AVOID_FOCUS
			| B_AUTO_UPDATE_SIZE_LIMITS),
	fFile(srcFile.name),
	fDoAll(new BCheckBox("", B_TRANSLATE("Do this for all files"), NULL)),
	fReplace(false),
	fSem(create_sem(0, "")),
	fStripeView(NULL)
{
	BBitmap icon = _GetIcon(32 * icon_layout_scale());
	fStripeView = new StripeView(icon);

	BStringView* info = new BStringView("",
		B_TRANSLATE("A file with that name already exists in the target folder."));

	BButton* replace = new BButton("", B_TRANSLATE("Replace"),
		new BMessage(kReplace));
	BButton* skip = new BButton("", B_TRANSLATE("Skip"), new BMessage(kSkip));

	BLayoutBuilder::Group<>(this, B_HORIZONTAL, B_USE_ITEM_SPACING)
		.Add(fStripeView)
		.AddGroup(B_VERTICAL)
			.SetInsets(0, B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING)
			.Add(info)
			.AddStrut(B_USE_HALF_ITEM_SPACING)
			.AddGrid(B_USE_HALF_ITEM_SPACING, 0)
				.Add(_CreateLabelView(B_TRANSLATE("File name")), 0, 0)
				.Add(CreateLightString(fFile), 1, 0)
				.Add(BSpaceLayoutItem::CreateVerticalStrut(B_USE_HALF_ITEM_SPACING),
					0, 1)
				.Add(_CreateLabelView(B_TRANSLATE("Source folder")), 0, 2)
				.Add(new FolderPathView(srcFolder, srcFile), 1, 2)
				.Add(_CreateAttrView(srcFolder), 1, 3)
				.Add(BSpaceLayoutItem::CreateVerticalStrut(B_USE_HALF_ITEM_SPACING),
					0, 4)
				.Add(_CreateLabelView(B_TRANSLATE("Target folder")), 0, 5)
				.Add(new FolderPathView(destFolder, destFile), 1, 5)
				.Add(_CreateAttrView(destFolder), 1, 6)
				.Add(BSpaceLayoutItem::CreateVerticalStrut(B_USE_HALF_ITEM_SPACING),
					0, 7)
				.Add(_CreateLabelView(B_TRANSLATE("Rule name")), 0, 8)
				.Add(CreateLightString(desc), 1, 8)
				.End()
			.AddStrut(B_USE_HALF_ITEM_SPACING)
			.AddGroup(B_HORIZONTAL)
				.AddGlue()
				.Add(skip)
				.Add(replace)
				.AddGlue()
				.End()
			.Add(fDoAll)
			.End()
		.End();
}


bool
ConflictWindow::Go(bool& doAll)
{
	CenterOnScreen();
	Show();

	while (acquire_sem(fSem) == B_INTERRUPTED);
		// The current thread will be blocked here because the semaphore was
		// created with a thread count of 0. Interrupt signals from the thread
		// that spawned the current thread are to be ignored.

	doAll = fDoAll->Value() == B_CONTROL_ON;

	bool flag = fReplace;
		// save the flag before it's destroyed by Quit() below

	if (Lock()) Quit();

	return flag;
}


void
ConflictWindow::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case kReplace:
			fReplace = true;
		case kSkip:
			delete_sem(fSem);
				// unblock the thread that was blocked in Go()
			break;
		default:
			BWindow::MessageReceived(msg);
	}
}


BStringView*
ConflictWindow::_CreateLabelView(const char* label)
{
	BString text(label);
	text += ':';

	BStringView* view = new BStringView("", text);
	view->SetAlignment(B_ALIGN_RIGHT);

	return view;
}


BStringView*
ConflictWindow::_CreateAttrView(const char* folder)
{
	BDirectory dir(folder);
	if (dir.InitCheck() != B_OK)
		return NULL;

	struct stat attr;
	if (dir.GetStatFor(fFile, &attr) != B_OK)
		return NULL;

	BString str;
	BDateTimeFormat fmt;
	if (fmt.Format(str, attr.st_mtime, B_LONG_DATE_FORMAT, B_MEDIUM_TIME_FORMAT)
		!= B_OK)
		return NULL;

	char size[128];
	string_for_size(attr.st_size, size, sizeof(size));

	str.Prepend("  •  ");
	str.Prepend(size);

	return CreateLightString(str);
}


BBitmap
ConflictWindow::_GetIcon(int32 iconSize)
{
	BBitmap icon(BRect(0, 0, iconSize - 1, iconSize - 1), 0, B_RGBA32);
	team_info teamInfo;
	get_team_info(B_CURRENT_TEAM, &teamInfo);
	app_info appInfo;
	be_roster->GetRunningAppInfo(teamInfo.team, &appInfo);
	BNodeInfo::GetTrackerIcon(&appInfo.ref, &icon, icon_size(iconSize));
	return icon;
}
