/*
	RuleItem.cpp: BStringItem which has a reference to its corresponding rule
	Released under the MIT license.
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Contributed by Owen Pan <owen.pan@yahoo.com>, 2017
*/

#include "RuleItem.h"
#include "FilerRule.h"


RuleItem::RuleItem(FilerRule* item)
	:
	BStringItem("")
{
	SetRule(item);
}


void
RuleItem::DrawItem(BView* owner, BRect frame, bool complete)
{
	if (fRule->Disabled()) {
		rgb_color color;
		float tint, light, dark;

		if (IsSelected()) {
			color = ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR);
			light = B_LIGHTEN_1_TINT;
			dark = B_DARKEN_1_TINT;
		} else {
			color = ui_color(B_LIST_ITEM_TEXT_COLOR);
			light = B_LIGHTEN_2_TINT;
			dark = B_DARKEN_2_TINT;
		}

		tint = color.red + color.green + color.blue > 128 * 3 ? dark : light;
		owner->SetHighColor(tint_color(color, tint));
	}

	BStringItem::DrawItem(owner, frame, complete);
}


void
RuleItem::SetRule(FilerRule* rule)
{
	fRule = rule;

	if (fRule)
		SetText(rule->GetDescription());
	else
		SetText("BUG: Invalid rule");
}


FilerRule*
RuleItem::Rule()
{
	return fRule;
}
