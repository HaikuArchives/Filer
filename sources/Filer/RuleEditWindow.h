/*
	RuleEditWindow.h: Rule editor class (duh)
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/
#ifndef RULE_EDIT_WINDNOW_H
#define RULE_EDIT_WINDNOW_H

#include <Box.h>
#include <Button.h>
#include <Window.h>
#include "ObjectList.h"

class AutoTextControl;
class FilerRule;
class TestView;

// Message defs used by other classes
enum
{
	M_ADD_RULE = 'adrl',
	M_UPDATE_RULE = 'uprl',
	M_FORCE_QUIT = 'frcq'
};

class RuleEditWindow : public BWindow
{
public:
				RuleEditWindow(BRect &rect, FilerRule *rule);
				~RuleEditWindow(void);
	void		MessageReceived(BMessage *msg);
	
//	FilerRule *	Rule(void);
	
	void		AppendTest(BMessage *test);
	void		RemoveTest(void);
	
	void		AppendAction(BMessage *action);
	void		RemoveAction(void);
	
private:
	BRect		GetPreferredSize(void) const;
	void		SendRuleMessage(void);
	
	AutoTextControl	*fDescriptionBox;
	
	BBox			*fTestGroup,
					*fActionGroup;
	
	BButton			*fOK,
					*fCancel,
					*fAddTest,
					*fRemoveTest,
					*fAddAction,
					*fRemoveAction,
					*fHelp;
	
	int64			fOriginalID;
	
	BList			fTestList,
					fActionList;
};

#endif
