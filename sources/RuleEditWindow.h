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


class RuleEditWindow : public BWindow
{
public:
				RuleEditWindow(FilerRule* rule, BHandler* caller);
				~RuleEditWindow();

	void		MessageReceived(BMessage* msg);

//	FilerRule*	Rule();

	void		AppendTest(BMessage* test);
	void		RemoveTest();

	void		AppendAction(BMessage* action);
	void		RemoveAction();
private:
	void		SendRuleMessage();

	AutoTextControl*	fDescriptionBox;

	BBox*			fTestGroup;
	BBox*			fActionGroup;

	BGroupLayout*	fTestGroupLayout;
	BGroupLayout*	fActionGroupLayout;

	BButton*		fOK;
	BButton*		fCancel;
	BButton*		fAddTest;
	BButton*		fRemoveTest;
	BButton*		fAddAction;
	BButton*		fRemoveAction;
	BButton*		fHelp;

	int64			fOriginalID;
	BHandler*		fCaller;

	BList			fTestList;
	BList			fActionList;
};

#endif	// RULE_EDIT_WINDNOW_H
