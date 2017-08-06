/*
 * Copyright 2015. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#include "ContextPopUp.h"
#include "FilerDefs.h"

ContextPopUp::ContextPopUp(const char* name, BMessenger target)
	:
	BPopUpMenu(name, false, false),
	fTarget(target)
{
	SetAsyncAutoDestruct(true);
}


ContextPopUp::~ContextPopUp()
{
	fTarget.SendMessage(MSG_POPUP_CLOSED);
}
