/*
	RuleItem.cpp: BStringItem which has a reference to its corresponding rule
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/
#include "RuleItem.h"
#include "FilerRule.h"


RuleItem::RuleItem(FilerRule *item)
 :	BStringItem("")
{
	SetRule(item);
}
	

void
RuleItem::SetRule(FilerRule *rule)
{
	fRule = rule;
	
	if (fRule)
		SetText(rule->GetDescription());
	else
		SetText("BUG:Invalid Rule");
}


FilerRule *
RuleItem::Rule(void)
{
	return fRule;
}

