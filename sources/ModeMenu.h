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
#include <String.h>

class ModeMenu : public BPopUpMenu
{
			BString		fLabel;
	const	BMenuItem*	fTest;
	const	BView*		fTarget;

public:
			ModeMenu(const BMenuItem* item, const BView* view);
	bool	AddDynamicItem(add_state state);
};

#endif	// MODE_MENU_H
