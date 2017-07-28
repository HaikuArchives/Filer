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
#include "PanelButton.h"

class ActionView : public BView
{
public:
					ActionView(const char* name, BMessage* test = NULL,
							const int32& flags = B_WILL_DRAW);
					~ActionView();

	void 			AttachedToWindow();
	void			MessageReceived(BMessage* msg);

	BMessage*		GetAction() const;
	int8			GetType() const { return fType; }

	void			SetRemoveEnabled(bool isEnabled)
						{ fAddRemoveButtons->SetRemoveEnabled(isEnabled); }

	static void		SetVisibility(BView* view, bool show);

private:
	BPopUpMenu*		ActionMenu() const;
	void			SetAction();

	BMenuField*		fActionField;
	PanelButton*	fPanelButton;

	AutoTextControl*	fValueBox;
	AddRemoveButtons*	fAddRemoveButtons;

	int8			fType;
	BMessage		fActions;
};

#endif	// ACTIONVIEW_H
