/*
 * Copyright 2015. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#ifndef REPLICANT_WINDOW_H
#define REPLICANT_WINDOW_H

#include <Size.h>
#include <Window.h>

#include <stdio.h>

class ReplicantWindow : public BWindow {
public:
					ReplicantWindow(BRect frame);
	virtual			~ReplicantWindow();

//	bool			QuitRequested();

};

#endif // REPLICANT_WINDOW_H
