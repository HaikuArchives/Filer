/*
	AutoFiler: Watcher daemon which executes the Filer on monitored folders
	Released under the MIT license.
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Contributed by:
		Owen Pan <owen.pan@yahoo.com>, 2017
*/

#include <Locker.h>
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


static void
LaunchFiler(const entry_ref& ref)
{
	static uint32 launchTime = real_time_clock();

	RefStorage* refholder;
	team_id team;
	entry_ref dirRef;
	BEntry dir, file(&ref);
	BMessage msg(B_REFS_RECEIVED);

	file.GetParent(&dir);
	dir.GetRef(&dirRef);
	msg.AddRef("refs", &ref);

	gRefLock.Lock();

	for (int32 i = 0; i < gRefStructList.CountItems(); i++)
	{
		refholder = (RefStorage*) gRefStructList.ItemAt(i);

		if (refholder->ref == dirRef)
		{
			if (refholder->doAll && real_time_clock() > launchTime)
				refholder->doAll = false;

			msg.AddBool(kDoAll, refholder->doAll);
			msg.AddBool(kReplace, refholder->replace);
			break;
		}
	}

	be_roster->Launch(kFilerSignature, &msg, &team);

	if (!refholder->doAll)
	{
		BMessage reply;
		be_app_messenger.SetTo(NULL, team);

		if (be_app_messenger.SendMessage(MSG_AUTO_FILER, &reply) == B_OK)
		{
			reply.FindBool(kDoAll, &refholder->doAll);
			reply.FindBool(kReplace, &refholder->replace);
		}
	}

	launchTime = real_time_clock();

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

			LaunchFiler(ref);
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

				LaunchFiler(ref);
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
				LaunchFiler(ref);
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
	App app;
	app.Run();
	
	return 0;
}
