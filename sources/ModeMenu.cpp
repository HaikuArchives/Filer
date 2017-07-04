/*
 * Copyright 2017 Haiku, Inc.
 * Distributed under the terms of the MIT License.
 *
 * Author:
 *			Owen Pan <owen.pan@yahoo.com>
 */


#include "ModeMenu.h"

#include <MenuItem.h>

#include "FilerDefs.h"
#include "RuleRunner.h"


ModeMenu::ModeMenu(const BMenuItem* item, const BView* view)
	:
	BPopUpMenu(""),
	fTest(item),
	fTarget(view)
{
}


bool
ModeMenu::AddDynamicItem(add_state state)
{
	if (state != B_INITIAL_ADD)
		return false;

	BString label(fTest->Label());
	if (label == fLabel)
		return false;

	fLabel = label;
	RemoveItems(0, CountItems(), true);

	BMessage modes;
	if (RuleRunner::GetCompatibleModes(GetTestType(label.String()), modes)
		!= B_OK)
		return false;

	int8 modetype;
	for (int32 i = 0; modes.FindInt8("modes", i, &modetype) == B_OK; i++) {
		BMessage* msg = new BMessage(MSG_MODE_CHOSEN);
		msg->AddInt8("mode", modetype);
		AddItem(new BMenuItem(sModeTypes[modetype].locale, msg));
	}

	SetTargetForItems(fTarget);
	return true;
}
