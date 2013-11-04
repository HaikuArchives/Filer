/*
	ActionView.h: View for adjusting settings for an individual Filer action
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/
#ifndef ACTIONVIEW_H
#define ACTIONVIEW_H

#include <View.h>
#include <Button.h>

class AutoTextControl;

#include <TextControl.h>

class ActionView : public BView
{
public:
					ActionView(const BRect &frame,const char *name,
							BMessage *test = NULL,
							const int32 &resize = B_FOLLOW_LEFT | B_FOLLOW_TOP,
							const int32 &flags = B_WILL_DRAW);
					~ActionView(void);
	void 			AttachedToWindow(void);
	BRect			GetPreferredSize(void);
	void			ResizeToPreferred(void);
	void			MessageReceived(BMessage *msg);
	BMessage *	GetAction(void) const;
	
private:
	void			ShowActionMenu(void);
	void			SetAction(const char *name
);
	
	BButton			*fActionButton;
	
	AutoTextControl	*fValueBox;
	
	BMessage		*fAction;
	BMessage		fActions;
};

#endif
