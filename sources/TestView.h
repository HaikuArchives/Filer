/*
	TestView.h: view to display and edit settings for Filer tests
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/
#ifndef TESTVIEW_H
#define TESTVIEW_H

#include <Button.h>
#include <Menu.h>
#include <MenuItem.h>
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

	void 		AttachedToWindow();
	BRect		GetPreferredSize();
	void		ResizeToPreferred();
	void		MessageReceived(BMessage* msg);
	BMessage*	GetTest() const;
	
private:
	void		SetupTestMenu();
	void		ShowModeMenu();
	bool		SetTest(BMessage* msg);
	void		SetMode(const char* mode);
	const char*	GetValue();
	
	BMenu*		AddMenuSorted(BMenu* parent, const char* name);
	BMenu*		GetMenu(BMenu* parent, const char* name);
	
	BButton*	fTestButton;
	BButton*	fModeButton;
	
	BMessage	fArchivedTestMenu;
	
	AutoTextControl*	fValueBox;
	
	BMessage*	fTest;
	BMessage	fTestTypes;
	
};
 
#endif	// TESTVIEW_H
