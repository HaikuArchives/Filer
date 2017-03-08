/*
 * Copyright 2017 Haiku, Inc.
 * Distributed under the terms of the MIT License.
 *
 * Author:
 *			Owen Pan <owen.pan@yahoo.com>
 */

#include <String.h>

#include <Button.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>
#include <StringView.h>

#include "ConflictWindow.h"

static const unsigned kReplace = 'RPLC';
static const unsigned kSkip = 'SKIP';

ConflictWindow::ConflictWindow(const char* file)
	: BWindow(BRect(0, 0, 0, 0), "", B_BORDERED_WINDOW, B_WILL_ACCEPT_FIRST_CLICK | B_AUTO_UPDATE_SIZE_LIMITS)
	, fDoAll(new BCheckBox("", "Do this for all files", NULL))
	, fReplace(false)
	, fSem(create_sem(0, ""))
{
	const float spacing = be_control_look->DefaultItemSpacing();

	BString text(file);
	text += " already exists.";

	BStringView* info = new BStringView("", text.String());
	BButton* replace = new BButton("", "Replace", new BMessage(kReplace));
	BButton* skip = new BButton("", "Skip", new BMessage(kSkip));
	skip->MakeDefault(true);

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

	// The current thread will be blocked here because the semaphore was
	// created with a thread count of 0. Interrupt signals from the thread
	// that spawned the current thread are to be ignored.
	while (acquire_sem(fSem) == B_INTERRUPTED);

	doAll = fDoAll->Value() == B_CONTROL_ON;

	// save the flag before it's destroyed by Quit() below
	bool flag = fReplace;

	if (Lock()) Quit();

	return flag;
}

void
ConflictWindow::MessageReceived(BMessage* msg)
{
	switch (msg->what)
	{
		case kReplace:
			fReplace = true;
		case kSkip:
			// unblock the thread that was blocked in Go()
			delete_sem(fSem);
			break;
		default:
			BWindow::MessageReceived(msg);
	}
}
