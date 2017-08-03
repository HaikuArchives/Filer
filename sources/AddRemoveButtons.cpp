/*
 * Copyright 2017 Haiku, Inc.
 * Distributed under the terms of the MIT License.
 *
 * Author:
 *			Owen Pan <owen.pan@yahoo.com>
 */


#include "AddRemoveButtons.h"

#include <LayoutBuilder.h>

#include "FilerDefs.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT ADD_REMVOE_BUTTONS_CONTEXT


AddRemoveButtons::AddRemoveButtons(uint32 add, uint32 remove, BView* target,
	float height, float spacing, const char* addLabel, const char* removeLabel)
	:
	BView(NULL, B_WILL_DRAW)
{
	fAdd = new BButton("+");
	fRemove = new BButton(kEnDash);

	BMessage* msg = new BMessage(add);
	msg->AddPointer(kPointer, target);
	fAdd->SetMessage(msg);

	msg = new BMessage(remove);
	msg->AddPointer(kPointer, target);
	fRemove->SetMessage(msg);

	BSize size(height, height);
	fAdd->SetExplicitSize(size);
	fRemove->SetExplicitSize(size);

	BLayoutBuilder::Group<>(this, B_HORIZONTAL, spacing)
		.Add(fRemove)
		.Add(fAdd)
		.End();

	fAdd->SetToolTip(B_TRANSLATE(addLabel));
	fRemove->SetToolTip(B_TRANSLATE(removeLabel));
}


AddRemoveButtons::~AddRemoveButtons()
{
	delete fAdd;
	delete fRemove;
}


void
AddRemoveButtons::SetTarget(const BView* target)
{
	fAdd->SetTarget(target);
	fRemove->SetTarget(target);
}
