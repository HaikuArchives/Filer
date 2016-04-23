/*
 * Copyright 2008, 2016. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *  DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
 *	Humdinger, humdingerb@gmail.com
 */

#ifndef AUTOFILERTAB_H
#define AUTOFILERTAB_H

#include <Button.h>
#include <CheckBox.h>
#include <FilePanel.h>
#include <ListView.h>
#include <ScrollView.h>
#include <View.h>

class TypedRefFilter;

class AutoFilerTab : public BView
{
public:
					AutoFilerTab();
					~AutoFilerTab();

	virtual void	AttachedToWindow();
	virtual void	DetachedFromWindow();
	void			MessageReceived(BMessage* msg);
	
private:
	void			_BuildLayout();

	void			EnableAutorun();
	void			DisableAutorun();
	
	BListView*		fFolderList;
	BScrollView*	fScrollView;

	BCheckBox*		fAutorunBox;
	
	BButton*		fAddButton;
	BButton*		fEditButton;
	BButton*		fRemoveButton;
					
	BFilePanel*		fFilePanel;
	TypedRefFilter*	fRefFilter;
};

#endif // AUTOFILERTAB_H
