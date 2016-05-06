/*
 * Copyright 2008, 2016. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *  DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
 *	Humdinger, humdingerb@gmail.com
 */
#ifndef RULETAB_H
#define RULETAB_H

#include <Button.h>
#include <CheckBox.h>
#include <ListView.h>
#include <View.h>

#include "ObjectList.h"

class RuleItem;
class FilerRule;

class RuleTab : public BView
{
public:
					RuleTab();
					~RuleTab();

	virtual void	AttachedToWindow();
	void			MessageReceived(BMessage* message);
	
private:
	void			_BuildLayout();

	void			AddRule(FilerRule* rule);
	void			RemoveRule(RuleItem* item);
	void			MakeEmpty();

	BObjectList<FilerRule>*fRuleList;

	BCheckBox*		fMatchBox;

	BButton*		fAddButton;
	BButton*		fEditButton;
	BButton*		fRemoveButton;
	BButton*		fMoveUpButton;
	BButton*		fMoveDownButton;

	BListView*		fRuleItemList;
	BScrollView*	fScrollView;
	bool			fChanges;
};


#endif // RULETAB_H
