/*
	Filer - an automatic rule-based file organizer
	
	Released under the MIT license.
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Contributed by:
		Owen Pan <owen.pan@yahoo.com>, 2017
*/

#ifndef MAIN_H
#define MAIN_H

#include <Application.h>
#include <Entry.h>
#include <Message.h>

#include "ObjectList.h"

class FilerRule;
class MainWindow;

class App : public BApplication
{
public:
					App();
					~App();

	void			MessageReceived(BMessage* msg);
	void			RefsReceived(BMessage* msg);
	void			ArgvReceived(int32 argc, char** argv);
	void			ReadyToRun();

	void			SetupTypeMenu();
	void			ShowHTML(const char* docfile);
	void			FileRef(entry_ref ref);

	bool			GetMatchSetting();
	void			ToggleMatchSetting();

	bool			DoAll() const { return fDoAll; }
	void			DoAll(bool doAll) { fDoAll = doAll; }
	bool			Replace() const { return fReplace; }
	void			Replace(bool replace) { fReplace = replace; }

private:
	void			LoadRuleSettings();
//	void			SaveRuleSettings();
	void			ProcessFiles();

	MainWindow*		fMainWin;
	
	bool			fQuitRequested;
	bool 			fMatchSetting;

	bool			fDoAll;
	bool			fReplace;

	BObjectList<entry_ref>*	fRefList;
	BObjectList<FilerRule>*	fRuleList;
};

#endif	// MAIN_H
