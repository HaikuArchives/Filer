/*
 * Copyright 2016. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <Application.h>
#include <Button.h>
#include <CheckBox.h>
#include <GroupLayoutBuilder.h>
#include <ScrollView.h>
#include <Size.h>
#include <TabView.h>
#include <Window.h>

#include "AutoFilerTab.h"
#include "DropZoneTab.h"
#include "FilerDefs.h"
#include "HelpTab.h"
#include "RuleTab.h"

#include <stdio.h>


class MainWindow : public BWindow {
public:
					MainWindow();
	virtual			~MainWindow();

	bool			QuitRequested();
	void			MessageReceived(BMessage* msg);

private:
	void			_BuildLayout();
	void			LoadSettings();
	void			SaveSettings();

	BRect			fPosition;
	int32			fTabSelection;

	BTabView*		fTabView;
	DropZoneTab*	fDropZone;
	RuleTab*		fRules;
	AutoFilerTab*	fAutoFiler;
	HelpTab*		fHelp;
};

#endif // MAIN_WINDOW_H
