/*
 * Copyright 2017 Haiku, Inc.
 * Distributed under the terms of the MIT License.
 *
 * Author:
 *			Owen Pan <owen.pan@yahoo.com>
 */

#ifndef CONFLICT_WINDOW_H
#define CONFLICT_WINDOW_H

#include <CheckBox.h>
#include <StringView.h>
#include <Window.h>

class ConflictWindow : public BWindow
{
	BCheckBox*	fDoAll;
	bool		fReplace;

	// The semaphore below is used to block the thread that created this
	// window. When the thread calls Go(), it will be blocked by the
	// semaphore. When the window is closed, the semaphore is deleted, and
	// the thread will be unblocked.
	sem_id	fSem;

	void			MessageReceived(BMessage* msg);
	BStringView*	_CreateAttrView(const char* file, const char* folder);

public:
			ConflictWindow(const char* file, const char* src, const char* dest,
				const char* desc);
	bool	Go(bool& doAll);
};

#endif	// CONFLICT_WINDOW_H
