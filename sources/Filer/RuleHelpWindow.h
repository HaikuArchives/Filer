/*
	RuleHelpWindow.h: Quick-and-dirty help display control
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/
#ifndef RULE_HELP_WINDOW_H
#define RULE_HELP_WINDOW_H

#include <Window.h>
#include <TextView.h>

class RuleHelpWindow : public BWindow
{
public:
	RuleHelpWindow(BRect frame);

private:
	BTextView	*fTextView;
};

#endif
