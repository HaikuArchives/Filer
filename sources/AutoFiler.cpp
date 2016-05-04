/*
	AutoFiler: Watcher daemon which executes the Filer on monitored folders
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/

#include <NodeMonitor.h>
#include <Roster.h>
#include <String.h>

#include "AutoFiler.h"
#include "FilerDefs.h"
#include "RefStorage.h"

App::App()
	:
	BApplication(kAutoFilerSignature)
{
	LoadFolders();
	StartWatching();
}


void
App::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		case MSG_REFRESH_FOLDERS:
		{
			StopWatching();
			ReloadFolders();
			StartWatching();
			break;
		}
		case B_NODE_MONITOR:
		{
			HandleNodeMonitoring(msg);
			break;
		}
		default:
			BApplication::MessageReceived(msg);
			break;
	}
}


void
App::StartWatching()
{
	gRefLock.Lock();
	
	for (int32 i = 0; i < gRefStructList.CountItems(); i++)
	{
		RefStorage* refholder = (RefStorage*)gRefStructList.ItemAt(i);
		watch_node(&refholder->nref, B_WATCH_ALL, this);
	}
	
	gRefLock.Unlock();
}


void
App::HandleNodeMonitoring(BMessage* msg)
{
	int32 op;
	msg->FindInt32("opcode", &op);
	
	switch (op)
	{
		case B_ENTRY_CREATED:
		{
			BString name;
			entry_ref ref;
			
			msg->FindInt32("device", &ref.device);
			msg->FindInt64("directory", &ref.directory);
			msg->FindString("name", &name);
			ref.set_name(name.String());
			
			BMessage args(B_REFS_RECEIVED);
			args.AddRef("refs", &ref);
			be_roster->Launch(kFilerSignature, &args);
			break;
		}
		case B_ENTRY_MOVED:
		{
			// We only care if we're monitoring the "to" directory because
			// the Filer doesn't care about files that aren't there anymore
			node_ref nref;
			msg->FindInt32("device", &nref.device);
			msg->FindInt64("to directory", &nref.node);
			
			gRefLock.Lock();
			
			bool match = false;
			for (int32 i = 0; i < gRefStructList.CountItems(); i++)
			{
				RefStorage* refholder = (RefStorage*)gRefStructList.ItemAt(i);
				if (nref == refholder->nref)
				{
					match = true;
					break;
				}
			}
			
			gRefLock.Unlock();
			
			if (match)
			{
				BString name;
				entry_ref ref;
				msg->FindString("name", &name);
				ref.device = nref.device;
				ref.directory = nref.node;
				ref.set_name(name.String());
				
				BMessage args(B_REFS_RECEIVED);
				args.AddRef("refs", &ref);
				be_roster->Launch(kFilerSignature, &args);
			}
			break;
		}
		case B_STAT_CHANGED:
		case B_ATTR_CHANGED:
		{
			node_ref nref;
			msg->FindInt32("device", &nref.device);
			msg->FindInt64("node", &nref.node);
			
			gRefLock.Lock();
			
			bool match = false;
			entry_ref ref;
			for (int32 i = 0; i < gRefStructList.CountItems(); i++)
			{
				RefStorage* refholder = (RefStorage*)gRefStructList.ItemAt(i);
				if (nref == refholder->nref)
				{
					ref = refholder->ref;
					match = true;
					break;
				}
			}
			
			gRefLock.Unlock();
			
			if (match)
			{
				BMessage args(B_REFS_RECEIVED);
				args.AddRef("refs", &ref);
				be_roster->Launch(kFilerSignature, &args);
			}
			break;
		}
		default:
			break;
	}
}


void
App::StopWatching()
{
	stop_watching(this);
}


int
main(int argc, char** argv)
{
	App* app = new App;
	app->Run();
	delete app;
	
	return 0;
}
