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
#include <LayoutBuilder.h>
#include <String.h>
#include <StringView.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ConflictWindow"

static const unsigned kReplace = 'RPLC';
static const unsigned kSkip = 'SKIP';


ConflictWindow::ConflictWindow(const char* file, const char* dest,
	const char* src, const char* desc)
	:
	BWindow(BRect(0, 0, 0, 0), B_TRANSLATE("Filer: Conflict"),
		B_FLOATING_WINDOW_LOOK, B_FLOATING_ALL_WINDOW_FEEL,
		B_NOT_CLOSABLE | B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_AVOID_FOCUS |
		B_AUTO_UPDATE_SIZE_LIMITS),
	fDoAll(new BCheckBox("", B_TRANSLATE("Do this for all files"), NULL)),
	fReplace(false),
	fSem(create_sem(0, ""))
{
	BStringView* info = new BStringView("",
		"File already exists in target folder.");

	BString text(B_TRANSLATE_COMMENT("File name: %file%",
		"Don't translate the variable %file%"));
	text.ReplaceFirst("%file%", file);
	BStringView* fileName = new BStringView("", text);

	text = B_TRANSLATE_COMMENT("Source folder: %src%",
		"Don't translate the variable %src%");
	text.ReplaceFirst("%src%", src);
	BStringView* srcFolder = new BStringView("", text);

	text = B_TRANSLATE_COMMENT("Target folder: %dest%",
		"Don't translate the variable %dest%");
	text.ReplaceFirst("%dest%", dest);
	BStringView* destFolder = new BStringView("", text);

	text = B_TRANSLATE_COMMENT("Rule name: %desc%",
		"Don't translate the variable %desc%");
	text.ReplaceFirst("%desc%", desc);
	BStringView* ruleDesc = new BStringView("", text);

	BButton* replace = new BButton("", B_TRANSLATE("Replace"),
		new BMessage(kReplace));
	BButton* skip = new BButton("", B_TRANSLATE("Skip"), new BMessage(kSkip));

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_BIG_INSETS)
		.AddGroup(B_VERTICAL, 0)
			.Add(info)
			.AddStrut(5)
			.Add(fileName)
			.Add(srcFolder)
			.Add(destFolder)
			.Add(ruleDesc)
			.End()
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
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
