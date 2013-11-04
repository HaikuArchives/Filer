/*
	RuleHelpWindow.cpp: Quick-and-dirty help display control
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/
#include "RuleHelpWindow.h"
#include <Button.h>
#include <ScrollView.h>
#include <Application.h>
#include <Roster.h>
#include <Path.h>
#include <Entry.h>
#include <String.h>
#include <TranslationUtils.h>
#include <File.h>


RuleHelpWindow::RuleHelpWindow(BRect frame)
 : BWindow(frame, "Filer Rule Help", B_TITLED_WINDOW,
 			B_ASYNCHRONOUS_CONTROLS)
{
	BView *top = new BView(Bounds(),"top",B_FOLLOW_ALL, B_WILL_DRAW);
	AddChild(top);
	top->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	
	BRect r = Bounds().InsetByCopy(10,10);
	r.right -= B_V_SCROLL_BAR_WIDTH;
	
	fTextView = new BTextView(r,"text",r.OffsetToCopy(0,0).InsetByCopy(10,10),
							B_FOLLOW_ALL, B_WILL_DRAW | B_NAVIGABLE);
							
	BScrollView *scrollView = new BScrollView("scrollview",fTextView,
											B_FOLLOW_ALL,0,false,true);
	top->AddChild(scrollView);
	
	fTextView->MakeEditable(false);
	
	// Load the help file
	app_info ai;
	be_app->GetAppInfo(&ai);
	
	BEntry entry(&ai.ref);
	entry_ref ref;
	BEntry parent;
	
	entry.GetParent(&parent);
	parent.GetRef(&ref);
	BPath pathobj(&ref);
	
	BString filePath = pathobj.Path();
	filePath += "/FilerHelp.txt";
	
	BFile file(filePath.String(),B_READ_ONLY);
	if (file.InitCheck() == B_OK)
		BTranslationUtils::GetStyledText(&file,fTextView);
}
