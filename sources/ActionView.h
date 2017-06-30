/*
	ActionView.h: View for adjusting settings for an individual Filer action
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/

#ifndef ACTIONVIEW_H
#define ACTIONVIEW_H

#include <MenuField.h>
#include <PopUpMenu.h>

#include "AddRemoveButtons.h"
#include "AutoTextControl.h"

class ActionView : public BView
{
public:
					ActionView(const char* name, BMessage* test = NULL,
							const int32& flags = B_WILL_DRAW);
					~ActionView();

	void 			AttachedToWindow();
	void			MessageReceived(BMessage* msg);

	BMessage*		GetAction() const;

	void			SetRemoveEnabled(bool isEnabled)
						{ fAddRemoveButtons->SetRemoveEnabled(isEnabled); }

private:
	BPopUpMenu*		ActionMenu() const;
	void			SetAction(int8 type);
	
	BMenuField*		fActionField;
	
	AutoTextControl*	fValueBox;
	AddRemoveButtons*	fAddRemoveButtons;
	
	BMessage*		fAction;
	BMessage		fActions;
};

#endif	// ACTIONVIEW_H
