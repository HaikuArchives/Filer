/*
	TestView.h: view to display and edit settings for Filer tests
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/

#ifndef TESTVIEW_H
#define TESTVIEW_H

#include <MenuField.h>
#include <PopUpMenu.h>

#include "AddRemoveButtons.h"
#include "AutoTextControl.h"

class TestView : public BView
{
public:
				TestView(const char* name, BMessage* test = NULL,
					const int32& flags = B_WILL_DRAW);
				~TestView();

	void 		AttachedToWindow();
	void		MessageReceived(BMessage* msg);

	BMessage*	GetTest() const;

	void		SetRemoveEnabled(bool isEnabled)
					{ fAddRemoveButtons->SetRemoveEnabled(isEnabled); }
	
private:
	BPopUpMenu*	TestMenu() const;
	bool		SetTest(BMessage* msg);
	void		SetMode(const char* mode);
	const char*	GetValue();
	
	BMenu*		AddMenuSorted(BMenu* parent, const char* name) const;
	BMenu*		GetMenu(BMenu* parent, const char* name) const;
	
	BMenuField*	fTestField;
	BMenuField*	fModeField;
	
	AutoTextControl*	fValueBox;
	AddRemoveButtons*	fAddRemoveButtons;
	
	BMessage*	fTest;
	BMessage	fTestTypes;
	
};
 
#endif	// TESTVIEW_H
