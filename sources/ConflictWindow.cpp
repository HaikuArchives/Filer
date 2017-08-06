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
#include <DateTimeFormat.h>
#include <Directory.h>
#include <LayoutBuilder.h>

#include <private/shared/StringForSize.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ConflictWindow"

#define CREATE_TRANSLATED_VIEW(str, var, view) \
	text = B_TRANSLATE_COMMENT(str ": %" #var "%", \
		"Don't translate the variable %" #var "%"); \
	text.ReplaceFirst("%" #var "%", var); \
	BStringView* view = new BStringView("", text)

static const unsigned kReplace = 'RPLC';
static const unsigned kSkip = 'SKIP';


ConflictWindow::ConflictWindow(const char* file, const char* src,
	const char* dest, const char* desc)
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
	info->SetAlignment(B_ALIGN_CENTER);

	BString text;

	CREATE_TRANSLATED_VIEW("File name", file, fileName);

	CREATE_TRANSLATED_VIEW("Source folder", src, srcFolder);
	BStringView* srcInfo = _CreateAttrView(file, src);

	CREATE_TRANSLATED_VIEW("Target folder", dest, destFolder);
	BStringView* destInfo = _CreateAttrView(file, dest);

	CREATE_TRANSLATED_VIEW("Rule name", desc, ruleDesc);

	BButton* replace = new BButton("", B_TRANSLATE("Replace"),
		new BMessage(kReplace));
	BButton* skip = new BButton("", B_TRANSLATE("Skip"), new BMessage(kSkip));

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_BIG_INSETS)
		.AddGroup(B_VERTICAL, 0)
			.Add(info)
			.Add(fileName)
			.Add(srcFolder)
			.Add(srcInfo)
			.Add(destFolder)
			.Add(destInfo)
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

	BString text("[ ");
	text << str << "; " << size << " ]";
	BStringView* view = new BStringView("", text);
	view->SetAlignment(B_ALIGN_CENTER);

	return view;
}
