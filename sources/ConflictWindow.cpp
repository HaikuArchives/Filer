/*
 * Copyright 2017 Haiku, Inc.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *			Owen Pan <owen.pan@yahoo.com>
 *			Humdinger <humdingerb@gmail.com>
 */


#include "ConflictWindow.h"

#include <Button.h>
#include <Catalog.h>
#include <Cursor.h>
#include <DateTimeFormat.h>
#include <Directory.h>
#include <LayoutBuilder.h>

#include <private/shared/StringForSize.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ConflictWindow"

static const unsigned kReplace = 'RPLC';
static const unsigned kSkip = 'SKIP';


class FolderView : public BStringView
{
	void	MouseMoved(BPoint point, uint32 transit, const BMessage* dragMsg);

public:
			FolderView(const BString& path);
};


FolderView::FolderView(const BString& path)
	:
	BStringView("", path)
{
}


void
FolderView::MouseMoved(BPoint point, uint32 transit, const BMessage* dragMsg)
{
	switch (transit) {
		case B_ENTERED_VIEW:
		{
			const BCursor cursor(B_CURSOR_ID_FOLLOW_LINK);
			SetViewCursor(&cursor);
			break;
		}
		case B_EXITED_VIEW:
		{
			const BCursor cursor(B_CURSOR_ID_SYSTEM_DEFAULT);
			SetViewCursor(&cursor);
			break;
		}
	}
}


ConflictWindow::ConflictWindow(const char* file, const char* src,
	const char* dest, const char* desc)
	:
	BWindow(BRect(0, 0, 0, 0), B_TRANSLATE("Filer: Conflict"),
		B_FLOATING_WINDOW_LOOK, B_FLOATING_ALL_WINDOW_FEEL,
		B_NOT_CLOSABLE | B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_AVOID_FOCUS
			| B_AUTO_UPDATE_SIZE_LIMITS),
	fDoAll(new BCheckBox("", B_TRANSLATE("Do this for all files"), NULL)),
	fReplace(false),
	fSem(create_sem(0, ""))
{
	BStringView* info = new BStringView("",
		"File already exists in target folder.");

	BButton* replace = new BButton("", B_TRANSLATE("Replace"),
		new BMessage(kReplace));
	BButton* skip = new BButton("", B_TRANSLATE("Skip"), new BMessage(kSkip));

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_BIG_INSETS)
		.Add(info)
		.AddGrid(B_USE_HALF_ITEM_SPACING, 0)
			.Add(_CreateLabelView(B_TRANSLATE("File name")), 0, 0)
			.Add(new BStringView("", file), 1, 0)
			.Add(_CreateLabelView(B_TRANSLATE("Source folder")), 0, 1)
			.Add(new FolderView(src), 1, 1)
			.Add(_CreateAttrView(file, src), 1, 2)
			.Add(_CreateLabelView(B_TRANSLATE("Target folder")), 0, 3)
			.Add(new FolderView(dest), 1, 3)
			.Add(_CreateAttrView(file, dest), 1, 4)
			.Add(_CreateLabelView(B_TRANSLATE("Rule name")), 0, 5)
			.Add(new BStringView("", desc), 1, 5)
			.End()
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(skip)
			.Add(replace)
			.AddGlue()
			.End()
		.Add(fDoAll)
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

	return new BStringView("", text);
}


BStringView*
ConflictWindow::_CreateAttrView(const char* file, const char* folder)
{
	BDirectory dir(folder);
	if (dir.InitCheck() != B_OK)
		return NULL;

	struct stat attr;
	if (dir.GetStatFor(file, &attr) != B_OK)
		return NULL;

	BString str;
	BDateTimeFormat fmt;
	if (fmt.Format(str, attr.st_mtime, B_SHORT_DATE_FORMAT, B_SHORT_TIME_FORMAT)
		!= B_OK)
		return NULL;

	char size[128];
	string_for_size(attr.st_size, size, sizeof(size));
	str << "; " << size;

	BStringView* view = new BStringView("", str);

	BFont font;
	view->GetFont(&font);
	font.SetFace(B_ITALIC_FACE);
	view->SetFont(&font);

	return view;
}
