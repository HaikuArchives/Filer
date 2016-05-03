/*
	ActionView.h: View for adjusting settings for an individual Filer action
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/

#ifndef ACTIONVIEW_H
#define ACTIONVIEW_H

#include <Button.h>
#include <View.h>

class AutoTextControl;

#include <TextControl.h>

class ActionView : public BView
{
public:
					ActionView(const BRect& frame, const char* name,
							BMessage* test = NULL,
							const int32& resize = B_FOLLOW_LEFT | B_FOLLOW_TOP,
							const int32& flags = B_WILL_DRAW);
					~ActionView();

	void 			AttachedToWindow();
	BRect			GetPreferredSize();
	void			ResizeToPreferred();
	void			MessageReceived(BMessage* msg);

	BMessage*		GetAction() const;

private:
	void			ShowActionMenu();
	void			SetAction(const char* name);
	
	BButton*		fActionButton;
	
	AutoTextControl*	fValueBox;
	
	BMessage*		fAction;
	BMessage		fActions;
};

#endif	// ACTIONVIEW_H
