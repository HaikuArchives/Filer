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
#include <ControlLook.h>
#include <LayoutBuilder.h>
#include <String.h>
#include <StringView.h>

static const unsigned kReplace = 'RPLC';
static const unsigned kSkip = 'SKIP';


ConflictWindow::ConflictWindow(const char* file)
	:
	BWindow(BRect(0, 0, 0, 0), "Filer: Conflict", B_FLOATING_WINDOW_LOOK, B_FLOATING_ALL_WINDOW_FEEL,
		B_NOT_CLOSABLE | B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_AVOID_FOCUS | B_AUTO_UPDATE_SIZE_LIMITS),
	fDoAll(new BCheckBox("", "Do this for all files", NULL)),
	fReplace(false),
	fSem(create_sem(0, ""))
{
	const float spacing = be_control_look->DefaultItemSpacing();

	BString text(file);
	text += " already exists.";

	BStringView* info = new BStringView("", text.String());
	BButton* replace = new BButton("", "Replace", new BMessage(kReplace));
	BButton* skip = new BButton("", "Skip", new BMessage(kSkip));

	BLayoutBuilder::Group<>(this, B_VERTICAL, spacing)
		.SetInsets(spacing, spacing, spacing, spacing)
		.Add(info)
		.AddGroup(B_HORIZONTAL, spacing)
			.SetInsets(spacing, spacing, spacing, spacing)
			.Add(skip)
			.Add(replace)
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
