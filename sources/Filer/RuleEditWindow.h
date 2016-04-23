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
				RuleEditWindow(BRect& rect, FilerRule* rule);
				~RuleEditWindow();

	void		MessageReceived(BMessage* msg);

//	FilerRule*	Rule();

	void		AppendTest(BMessage* test);
	void		RemoveTest();

	void		AppendAction(BMessage* action);
	void		RemoveAction();
private:
	BRect		GetPreferredSize() const;
	void		SendRuleMessage();

	AutoTextControl*	fDescriptionBox;

	BBox*			fTestGroup;
	BBox*			fActionGroup;

	BButton*		fOK;
	BButton*		fCancel;
	BButton*		fAddTest;
	BButton*		fRemoveTest;
	BButton*		fAddAction;
	BButton*		fRemoveAction;
	BButton*		fHelp;

	int64			fOriginalID;

	BList			fTestList;
	BList			fActionList;
};

#endif	// RULE_EDIT_WINDNOW_H
