/*
	Released under the MIT license.
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Contributed by: Humdinger <humdingerb@gmail.com>, 2016
*/

#include <Autolock.h>
#include <File.h>
#include <FindDirectory.h>
#include <Message.h>
#include <Path.h>

#include "RefStorage.h"

BList gRefStructList;
BLocker gRefLock;

const char gPrefsPath[] = "Filer/AutoFilerFolders";

RefStorage::RefStorage(const entry_ref& fileref)
	: doAll(false), replace(false)
{
	SetData(fileref);
}


void
RefStorage::SetData(const entry_ref& fileref)
{
	BEntry entry(&fileref);
	if (entry.Exists()) {
		entry.GetRef(&ref);
		entry.GetNodeRef(&nref);
	}
}


status_t
LoadFolders()
{
	BPath path;
	find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	path.Append(gPrefsPath);

	if (!BEntry(path.Path()).Exists())
		return B_OK;

	BAutolock autolock(&gRefLock);
	if (!autolock.IsLocked())
		return B_BUSY;

	BMessage msg;

	BFile file(path.Path(), B_READ_ONLY);
	status_t status = file.InitCheck();
	if (status != B_OK)
		return status;

	status = msg.Unflatten(&file);
	if (status != B_OK)
		return status;

	entry_ref tempRef;
	int32 i = 0;
	while (msg.FindRef("refs", i, &tempRef) == B_OK)
	{
		i++;
		RefStorage* refholder = new RefStorage(tempRef);
		if (refholder)
			gRefStructList.AddItem(refholder);
	}
	return status;
}


status_t
ReloadFolders()
{
	BAutolock autolock(&gRefLock);
	if (!autolock.IsLocked())
		return B_BUSY;

	for (int32 i = 0; i < gRefStructList.CountItems(); i++)
	{
		RefStorage* refholder = (RefStorage*)gRefStructList.ItemAt(i);
		delete refholder;
	}
	gRefStructList.MakeEmpty();

	LoadFolders();

	return B_OK;
}


status_t
SaveFolders()
{
	BAutolock autolock(&gRefLock);
	if (!autolock.IsLocked())
		return B_BUSY;

	BMessage msg;

	for (int32 i = 0; i < gRefStructList.CountItems(); i++)
	{
		RefStorage* refholder = (RefStorage*)gRefStructList.ItemAt(i);
		msg.AddRef("refs", &refholder->ref);;
	}
	BPath path;
	find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	path.Append(gPrefsPath);

	BFile file(path.Path(),B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
	status_t status = file.InitCheck();
	if (status != B_OK)
		return status;

	status = msg.Flatten(&file);
	return status;
}
