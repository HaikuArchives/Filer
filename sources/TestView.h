/*
	TestView.h: view to display and edit settings for Filer tests
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/

#ifndef TESTVIEW_H
#define TESTVIEW_H

#include <MenuField.h>
#include <PopUpMenu.h>
#include <String.h>

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
	int8		GetType() const { return fType; }
	int32		GetDataType() const { return fDataType; }

	void		ResetUnit();
	void		SetRemoveEnabled(bool isEnabled)
					{ fAddRemoveButtons->SetRemoveEnabled(isEnabled); }

private:
	BPopUpMenu*	TestMenu() const;
	void		SetTest();
	void		SetMode();
	void		SetUnit();
	void		FindAttribute(const BMessage* msg);
	const char*	GetValue();

	BMenu*		AddMenuSorted(BMenu* parent, const char* name) const;
	BMenu*		GetMenu(BMenu* parent, const char* name) const;

	BMenuField*	fTestField;
	BMenuField*	fModeField;
	BMenuField*	fUnitField;

	AutoTextControl*	fValueBox;
	AddRemoveButtons*	fAddRemoveButtons;

	BMessage	fTestTypes;

	int8		fType;
	int8		fMode;
	int8		fUnit;
	int32		fDataType;

	BString		fAttrType;
	BString		fAttrName;
	BString		fMimeType;
	BString		fTypeName;

	const char	fDecimalMark;
};

#endif	// TESTVIEW_H
