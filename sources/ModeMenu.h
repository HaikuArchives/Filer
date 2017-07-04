/*
 * Copyright 2017 Haiku, Inc.
 * Distributed under the terms of the MIT License.
 *
 * Author:
 *			Owen Pan <owen.pan@yahoo.com>
 */

#ifndef MODE_MENU_H
#define MODE_MENU_H

#include <PopUpMenu.h>

#include "TestView.h"

class ModeMenu : public BPopUpMenu
{
			int32		fDataType;
	const	BMenuItem*	fTest;
	const	TestView*	fTarget;

public:
			ModeMenu(const BMenuItem* item, const TestView* view);
	bool	AddDynamicItem(add_state state);
};

#endif	// MODE_MENU_H
