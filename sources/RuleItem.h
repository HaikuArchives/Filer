/*
	RuleItem.h: BStringItem which has a reference to its corresponding rule
	Released under the MIT license.
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Contributed by Owen Pan <owen.pan@yahoo.com>, 2017
*/

#ifndef RULEITEM_H
#define RULEITEM_H

#include <Entry.h>
#include <ListItem.h>
#include <ListView.h>

class FilerRule;

class RuleItem : public BStringItem
{
public:
				RuleItem(FilerRule* item);

	void		DrawItem(BView* owner, BRect frame, bool complete = false);
	void		SetRule(FilerRule* rule);
	FilerRule*	Rule();

private:
	FilerRule*	fRule;
};

#endif	// RULEITEM_H
