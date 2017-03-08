/*
 * Copyright 2016. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#ifndef HELPTAB_H
#define HELPTAB_H

#include <Bitmap.h>
#include <Button.h>
#include <StringView.h>
#include <TextView.h>
#include <View.h>

class IconView : public BView {
public:
						IconView();
						~IconView();

	void 				AttachedToWindow();
	virtual	void		Draw(BRect updateRect);
	virtual BSize		MinSize();
	virtual BSize		MaxSize();
			
private:
			void		GetIcon();
			BBitmap*	Icon();

			BBitmap*	fIcon;
};


class HelpTab : public BView
{
public:
					HelpTab();
					~HelpTab();
private:
	IconView*		fIconView;
	BStringView*	fName;
	BStringView*	fVersion;
	BStringView*	fCopyright1;
	BStringView*	fCopyright2;
	BStringView*	fCopyright3;
	BTextView*		fInfo;
	
	BButton*		fHelpButton;
	BButton*		fDocsButton;
};

#endif // HELPTAB_H
