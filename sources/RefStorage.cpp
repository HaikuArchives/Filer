/*
	Released under the MIT license.
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Contributed by:
		Humdinger <humdingerb@gmail.com>, 2016
		Owen Pan <owen.pan@yahoo.com>, 2017
*/

#include <Autolock.h>
#include <File.h>
#include <FindDirectory.h>
#include <Path.h>

#include "RefStorage.h"

BList gRefStructList;
BLocker gRefLock;

const char gPrefsPath[] = "Filer/AutoFilerFolders";

RefStorage::RefStorage(const entry_ref& fileref)
	:
	doAll(false),
	replace(false)
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
LoadFolders(BListView* folderList)
{
	BPath path;
	find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	path.Append(gPrefsPath);

	if (!BEntry(path.Path()).Exists())
		return B_OK;

	if (folderList == NULL) {
		BAutolock autolock(&gRefLock);
		if (!autolock.IsLocked())
			return B_BUSY;
	}

	BFile file(path.Path(), B_READ_ONLY);
	status_t status = file.InitCheck();
	if (status != B_OK)
		return status;

	BMessage msg;
	status = msg.Unflatten(&file);
	if (status != B_OK)
		return status;

	int32 i;
	int32 count = 0;

	// try to load folders in the new paths format
	BString str;
	for (i = 0; msg.FindString("path", i, &str) == B_OK; i++) {
		BEntry entry(str.String());
		if (entry.InitCheck() != B_OK || !entry.Exists()
			|| !entry.IsDirectory())
			continue;

		if (folderList == NULL) {
			entry_ref ref;
			if (entry.GetRef(&ref) != B_OK)
				continue;

			gRefStructList.AddItem(new RefStorage(ref));
		} else
			folderList->AddItem(new BStringItem(str));

		count++;
	}

	if (i > 0) {
		// at least one data field in paths format was found

		if (count < i)
			SaveFolders(folderList);

		return B_OK;
	}

	// folders are not in paths format, so load them in refs format

	count = 0;

	entry_ref ref;
	for (i = 0; msg.FindRef("refs", i, &ref) == B_OK; i++) {
		BEntry entry(&ref);
		if (entry.InitCheck() != B_OK || !entry.Exists()
			|| !entry.IsDirectory())
			continue;

		if (folderList == NULL)
			gRefStructList.AddItem(new RefStorage(ref));
		else
			folderList->AddItem(new BStringItem(BPath(&ref).Path()));

		count++;
	}

	if (count > 0)
		SaveFolders(folderList);	// save folders in the new paths format

	return B_OK;
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
SaveFolders(const BListView* folderList)
{
	int32 count;

	if (folderList == NULL) {
		BAutolock autolock(&gRefLock);
		if (!autolock.IsLocked())
			return B_BUSY;

		count = gRefStructList.CountItems();
	} else
		count = folderList->CountItems();

	BMessage msg;

	for (int32 i = 0; i < count; i++)
		if (folderList == NULL) {
			RefStorage* refholder = (RefStorage*)gRefStructList.ItemAt(i);
			BPath path(&refholder->ref);
			msg.AddString("path", path.Path());
		} else
			msg.AddString("path", ((BStringItem*) folderList->ItemAt(i))->Text());

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
