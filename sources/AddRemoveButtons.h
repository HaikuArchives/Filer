/*
 * Copyright 2017 Haiku, Inc.
 * Distributed under the terms of the MIT License.
 *
 * Author:
 *			Owen Pan <owen.pan@yahoo.com>
 */

#ifndef ADD_REMOVE_BUTTONS_H
#define ADD_REMOVE_BUTTONS_H

#include <Button.h>

class AddRemoveButtons : public BView
{
	BButton*	fAdd;
	BButton*	fRemove;

public:
			AddRemoveButtons(uint32 add, uint32 remove, BView* target);
			~AddRemoveButtons();

	void	SetRemoveEnabled(bool isEnabled) { fRemove->SetEnabled(isEnabled); }
};

#endif	// ADD_REMOVE_BUTTONS_H
