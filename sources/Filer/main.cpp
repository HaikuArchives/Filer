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

#include "main.h"
#include "FilerRule.h"
#include "PrefsWindow.h"
#include "RuleRunner.h"

// Created upon startup instead of when spawning a RuleEditWindow for
// better performance
BMessage gArchivedTypeMenu;

// Original def in TestView.cpp
#define M_TYPE_CHOSEN 'tych'

App::App(void)
 :	BApplication("application/x-vnd.dw-Filer"),
 	fRefList(NULL),
 	fRuleList(NULL),
 	fPrefsWin(NULL),
 	fQuitRequested(false)
{
	fRefList = new BObjectList<entry_ref>(20,true);
	fRuleList = new BObjectList<FilerRule>(20,true);
	
//	SetupTypeMenu();

	LoadRules(fRuleList);
}


App::~App(void)
{
	delete fRefList;
	delete fRuleList;
}


void
App::MessageReceived(BMessage *msg)
{
	switch(msg->what)
	{
		default:
			BApplication::MessageReceived(msg);
			break;
	}
}


void
App::RefsReceived(BMessage *msg)
{
	entry_ref tempRef;
	int32 i = 0;
	while (msg->FindRef("refs",i, &tempRef) == B_OK)
	{
		BEntry entry(&tempRef);
		if (entry.Exists())
		{
			entry_ref *ref = new entry_ref(tempRef);
			entry.GetRef(ref);
			fRefList->AddItem(ref);
		}
		else
			printf("Couldn't find file %s\n",tempRef.name);
		i++;
	}
	
	if (msg->FindRef("refs", &tempRef) == B_OK && fRefList->CountItems() == 0)
	{
		printf("No files given could be processed. Exiting.\n");
		fQuitRequested = true;
	}
}


void
App::ArgvReceived(int32 argc, char **argv)
{
	for (int32 i = 1; i < argc; i++)
	{
		BEntry entry(argv[i]);
		if (entry.Exists())
		{
			entry_ref *ref = new entry_ref;
			entry.GetRef(ref);
			fRefList->AddItem(ref);
		}
		else
			printf("Couldn't find file %s\n",argv[i]);
	}
	
	if (argc > 1 && fRefList->CountItems() == 0)
	{
		printf("No files given could be processed. Exiting.\n");
		fQuitRequested = true;
	}
}


void
App::ReadyToRun(void)
{
	if (fRefList->CountItems() > 0 || fQuitRequested)
	{
		for (int32 i = 0; i < fRefList->CountItems(); i++)
		{
			entry_ref ref = *fRefList->ItemAt(i);
			FileRef(ref);
		}
		PostMessage(B_QUIT_REQUESTED);
	}
	else
	{
		fPrefsWin = new PrefsWindow();
		fPrefsWin->Show();
	}
}


void
App::FileRef(entry_ref ref)
{
	RuleRunner runner;
	
	for (int32 i = 0; i < fRuleList->CountItems(); i++)
	{
		FilerRule * rule = fRuleList->ItemAt(i);
		runner.RunRule(rule,ref);
	}
}


void
App::SetupTypeMenu(void)
{
	BPopUpMenu *menu = new BPopUpMenu("Type");
	
	
	// Read in the types in the MIME database which have extra attributes
	// associated with them
	
	BMimeType mime;
	BMessage supertypes, types, info, attr;
	BString supertype;
	
	BMimeType::GetInstalledSupertypes(&supertypes);
	
	int32 index = 0;
	while (supertypes.FindString("super_types",index,&supertype) == B_OK)
	{
		index++;
		
		BMenu *submenu = new BMenu(supertype.String());
		menu->AddItem(submenu);
	
		BMimeType::GetInstalledTypes(supertype.String(),&types);
		
		BString string;
		int32 typeindex = 0;
		
		while (types.FindString("types",typeindex,&string) == B_OK)
		{
			typeindex++;
			
			mime.SetTo(string.String());
			
			char attrTypeName[B_MIME_TYPE_LENGTH];
			mime.GetShortDescription(attrTypeName);
			
			BString supertype = string.String();
			supertype.Truncate(supertype.FindFirst("/"));
			
			BMenuItem *item = menu->FindItem(supertype.String());
			BMenu *submenu = item->Submenu();
			if (!submenu->FindItem(attrTypeName))
			{
				BMessage *msg = new BMessage(M_TYPE_CHOSEN);
				msg->AddString("type",string.String());
				msg->AddString("typename",attrTypeName);
				submenu->AddItem(new BMenuItem(attrTypeName,msg));
			}
		}
	}

	menu->Archive(&gArchivedTypeMenu);
	delete menu;
}


int
main()
{
	App *app = new App;
	app->Run();
	delete app;
	return 0;
}
