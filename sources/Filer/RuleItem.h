/*
	RuleItem.h: BStringItem which has a reference to its corresponding rule
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/

#ifndef RULEITEM_H
#define RULEITEM_H

#include <Entry.h>
#include <ListItem.h>

class FilerRule;

class RuleItem : public BStringItem
{
public:
				RuleItem(FilerRule* item);

	void		SetRule(FilerRule* rule);
	FilerRule*	Rule();

private:
	FilerRule*	fRule;
};

#endif	// RULEITEM_H
