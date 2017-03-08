/*
 * Copyright 2017 Haiku, Inc.
 * Distributed under the terms of the MIT License.
 *
 * Author:
 *			Owen Pan <owen.pan@yahoo.com>
 */

#ifndef CONFLICT_WINDOW_H
#define CONFLICT_WINDOW_H

#include <Entry.h>
#include <Message.h>

#include <CheckBox.h>
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

	void	MessageReceived(BMessage* msg);

public:
			ConflictWindow(const char* file);
	bool	Go(bool& doAll);
};

#endif	// CONFLICT_WINDOW_H
