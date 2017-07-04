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


ModeMenu::ModeMenu(const BMenuItem* item, const TestView* view)
	:
	BPopUpMenu(""),
	fTest(item),
	fTarget(view),
	fDataType(view->GetDataType())
{
}


bool
ModeMenu::AddDynamicItem(add_state state)
{
	if (state != B_INITIAL_ADD)
		return false;

	int32 dataType = fTarget->GetDataType();
	if (fDataType == dataType)
		return false;

	fDataType = dataType;
	RemoveItems(0, CountItems(), true);

	BMessage modes;
	if (RuleRunner::GetCompatibleModes(fDataType, modes) != B_OK)
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
