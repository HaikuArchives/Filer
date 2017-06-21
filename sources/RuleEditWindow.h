/*
	RuleEditWindow.h: Rule editor class (duh)
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/
#ifndef RULE_EDIT_WINDNOW_H
#define RULE_EDIT_WINDNOW_H

#include <Box.h>
#include <Button.h>
#include <LayoutBuilder.h>
#include <Window.h>

#include "ActionView.h"
#include "AutoTextControl.h"
#include "FilerRule.h"
#include "TestView.h"


class RuleEditWindow : public BWindow
{
public:
				RuleEditWindow(FilerRule* rule, BHandler* caller);

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
	BButton*		fHelp;

	TestView*		fTestView;
	ActionView*		fActionView;

	int64			fOriginalID;
	BHandler*		fCaller;

	BList			fTestList;
	BList			fActionList;
};

#endif	// RULE_EDIT_WINDNOW_H
