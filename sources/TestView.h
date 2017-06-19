/*
	TestView.h: view to display and edit settings for Filer tests
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/
#ifndef TESTVIEW_H
#define TESTVIEW_H

#include <Menu.h>
#include <MenuField.h>
#include <Message.h>
#include <PopUpMenu.h>
#include <View.h>

class AutoTextControl;

class TestView : public BView
{
public:
				TestView(const char* name, BMessage* test = NULL,
					const int32& flags = B_WILL_DRAW);
				~TestView();

	void 		AttachedToWindow();
	void		MessageReceived(BMessage* msg);
	BMessage*	GetTest() const;
	
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
	
	BMessage*	fTest;
	BMessage	fTestTypes;
	
};
 
#endif	// TESTVIEW_H
