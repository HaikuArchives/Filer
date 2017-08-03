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
#include <Catalog.h>

#define ADD_REMVOE_BUTTONS_CONTEXT "AddRemoveButtons"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT ADD_REMVOE_BUTTONS_CONTEXT


class AddRemoveButtons : public BView
{
	BButton*	fAdd;
	BButton*	fRemove;

public:
			AddRemoveButtons(uint32 add, uint32 remove, BView* target,
				float height, float spacing = 0,
				const char* addLabel = B_TRANSLATE_MARK("Add"),
				const char* removeLabel = B_TRANSLATE_MARK("Remove"));
			~AddRemoveButtons();

	void	SetTarget(const BView* target);
	void	SetRemoveEnabled(bool isEnabled) { fRemove->SetEnabled(isEnabled); }
};

#endif	// ADD_REMOVE_BUTTONS_H
