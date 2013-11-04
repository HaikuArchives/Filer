/*
	TestView.h: view to display and edit settings for Filer tests
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/
#ifndef TESTVIEW_H
#define TESTVIEW_H

#include <View.h>
#include <Menu.h>
#include <MenuItem.h>
#include <MenuField.h>
#include <Button.h>
#include <PopUpMenu.h>
#include <Message.h>

class AutoTextControl;

class TestView : public BView
{
public:
				TestView(const BRect &frame,const char *name,
						BMessage *test = NULL,
						const int32 &resize = B_FOLLOW_LEFT | B_FOLLOW_TOP,
						const int32 &flags = B_WILL_DRAW);
	void 		AttachedToWindow(void);
	BRect		GetPreferredSize(void);
	void		ResizeToPreferred(void);
	void		MessageReceived(BMessage *msg);
	BMessage *	GetTest(void) const;
	
private:

	void		SetupTestMenu(void);
	void		ShowModeMenu(void);
	bool		SetTest(BMessage *msg);
	void		SetMode(const char *mode);
	const char *GetValue(void);
	
	BMenu *		AddMenuSorted(BMenu *parent,const char *name);
	BMenu *		GetMenu(BMenu *parent,const char *name);
	
	BButton		*fTestButton,
				*fModeButton;
	
	BMessage	fArchivedTestMenu;
	
	AutoTextControl	*fValueBox;
	
	BMessage		*fTest;
	BMessage		fTestTypes;
	
};
 
#endif
