/*
	Filer - an automatic rule-based file organizer
	
	Released under the MIT license.
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Contributed by: Humdinger <humdingerb@gmail.com>, 2016
*/

#include <FindDirectory.h>
#include <MenuItem.h>
#include <Mime.h>
#include <Path.h>
#include <PopUpMenu.h>
#include <Roster.h>

#include "main.h"
#include "FilerDefs.h"
#include "FilerRule.h"
#include "MainWindow.h"
#include "RuleRunner.h"

// Created upon startup instead of when spawning a RuleEditWindow for
// better performance
BMessage gArchivedTypeMenu;


App::App()
	:
	BApplication(kFilerSignature),
 	fRefList(NULL),
 	fRuleList(NULL),
	fMainWin(NULL),
 	fQuitRequested(false)
{
	fRefList = new BObjectList<entry_ref>(20, true);
	fRuleList = new BObjectList<FilerRule>(20, true);
	
//	SetupTypeMenu();

	LoadRules(fRuleList);
}


App::~App()
{
	delete fRefList;
	delete fRuleList;
}


void
App::MessageReceived(BMessage* msg)
{
	switch (msg->what)
	{
		case MSG_HELP:
		{
			ShowHTML("documentation/Rule-Making Reference.html");
			break;
		}
		case MSG_DOCS:
		{
			ShowHTML("documentation/User Documentation.html");
			break;
		}
		default:
			BApplication::MessageReceived(msg);
			break;
	}
}


void
App::RefsReceived(BMessage* msg)
{
	entry_ref tempRef;
	int32 i = 0;
	while (msg->FindRef("refs",i, &tempRef) == B_OK)
	{
		BEntry entry(&tempRef);
		if (entry.Exists())
		{
			entry_ref* ref = new entry_ref(tempRef);
			entry.GetRef(ref);
			fRefList->AddItem(ref);
		} else
			printf("Couldn't find file %s\n",tempRef.name);
		i++;
	}
	
	if (msg->FindRef("refs", &tempRef) == B_OK && fRefList->CountItems() == 0)
	{
		printf("No files given could be processed. Exiting.\n");
		fQuitRequested = true;
	}
	ProcessFiles();
}


void
App::ArgvReceived(int32 argc, char** argv)
{
	for (int32 i = 1; i < argc; i++)
	{
		BEntry entry(argv[i]);
		if (entry.Exists()) {
			entry_ref* ref = new entry_ref;
			entry.GetRef(ref);
			fRefList->AddItem(ref);
		} else
			printf("Couldn't find file %s\n",argv[i]);
	}
	
	if (argc > 1 && fRefList->CountItems() == 0) {
		printf("No files given could be processed. Exiting.\n");
		fQuitRequested = true;
	}
}


void
App::ReadyToRun()
{
	if (fRefList->CountItems() > 0 || fQuitRequested) {
		ProcessFiles();
		PostMessage(B_QUIT_REQUESTED);
	} else {
		fMainWin = new MainWindow();
		fMainWin->Show();
	}
}


void
App::ProcessFiles()
{
	for (int32 i = 0; i < fRefList->CountItems(); i++)
	{
		entry_ref ref = *fRefList->ItemAt(i);
		FileRef(ref);
	}
}


void
App::ShowHTML(const char* docfile)
{
	app_info info;
	BPath path;
	be_roster->GetActiveAppInfo(&info);
	BEntry entry(&info.ref);

	entry.GetPath(&path);
	path.GetParent(&path);
	path.Append(docfile);

	entry = path.Path();
	entry_ref ref;
	entry.GetRef(&ref);
	be_roster->Launch(&ref);
}


void
App::FileRef(entry_ref ref)
{
	RuleRunner runner;
	
	for (int32 i = 0; i < fRuleList->CountItems(); i++)
	{
		FilerRule* rule = fRuleList->ItemAt(i);
		runner.RunRule(rule, ref);
	}
}


void
App::SetupTypeMenu()
{
	BPopUpMenu* menu = new BPopUpMenu("Type");

	// Read in the types in the MIME database which have extra attributes
	// associated with them

	BMimeType mime;
	BMessage supertypes, types, info, attr;
	BString supertype;

	BMimeType::GetInstalledSupertypes(&supertypes);

	int32 index = 0;
	while (supertypes.FindString("super_types", index, &supertype) == B_OK)
	{
		index++;

		BMenu* submenu = new BMenu(supertype.String());
		menu->AddItem(submenu);

		BMimeType::GetInstalledTypes(supertype.String(), &types);

		BString string;
		int32 typeindex = 0;

		while (types.FindString("types", typeindex, &string) == B_OK)
		{
			typeindex++;

			mime.SetTo(string.String());

			char attrTypeName[B_MIME_TYPE_LENGTH];
			mime.GetShortDescription(attrTypeName);

			BString supertype = string.String();
			supertype.Truncate(supertype.FindFirst("/"));

			BMenuItem* item = menu->FindItem(supertype.String());
			BMenu* submenu = item->Submenu();
			if (!submenu->FindItem(attrTypeName)) {
				BMessage* msg = new BMessage(MSG_TYPE_CHOSEN);
				msg->AddString("type", string.String());
				msg->AddString("typename", attrTypeName);
				submenu->AddItem(new BMenuItem(attrTypeName, msg));
			}
		}
	}

	menu->Archive(&gArchivedTypeMenu);
	delete menu;
}


int
main()
{
	App* app = new App;
	app->Run();
	delete app;
	return 0;
}