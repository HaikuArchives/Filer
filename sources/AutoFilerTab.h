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

	void			UpdateAutoFilerFolders();
	void			ToggleAutorun(bool autorun);
	void			EnableAutorun();
	void			DisableAutorun();

	bool			IsFolderUnique(BString newpath);
	void			ToggleAutoFiler();
	void			UpdateAutoFilerLabel();
	
	BListView*		fFolderList;
	BScrollView*	fScrollView;

	BCheckBox*		fAutorunBox;
	BButton*		fStartStop;
	
	BButton*		fAddButton;
	BButton*		fRemoveButton;
					
	BFilePanel*		fFilePanel;
	TypedRefFilter*	fRefFilter;
	BMessageRunner*	fRunner;

	bool			fDirtySettings;
};

#endif // AUTOFILERTAB_H
