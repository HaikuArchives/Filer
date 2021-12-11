/*
 * Copyright 2008, DarkWyrm <darkwyrm@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "FSUtils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Alert.h>
#include <Catalog.h>
#include <Directory.h>
#include <Errors.h>
#include <File.h>
#include <OS.h>
#include <Path.h>
#include <String.h>
#include <Volume.h>

#include <fs_attr.h>


#define COPY_BUFFER_SIZE 1024000

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "FSUtils"


status_t
CheckCopiable(BEntry* src, BEntry* dest)
{
	// Checks to see if we can copy the src to dest.
	if (!src || !dest)
		return B_ERROR;

	if (!src->Exists())
		return B_ENTRY_NOT_FOUND;

	// Ensure that when we want the destination directory, that is exactly
	// what we're working with. If we've been given an entry which is a file,
	// extract the destination directory.
	BEntry destdir;		
	if (dest->IsDirectory())
		destdir = *dest;
	else
		dest->GetParent(&destdir);

	// check existence of target directory
	if (!destdir.Exists())
		return B_NAME_NOT_FOUND;

	// check space
	entry_ref ref;
	off_t src_bytes;

	dest->GetRef(&ref);
	BVolume dvolume(ref.device);
	src->GetSize(&src_bytes);

	if (src_bytes > dvolume.FreeBytes())
		return B_DEVICE_FULL;

	// check permissions
	if (dvolume.IsReadOnly())
		return B_READ_ONLY;

	// check existing name
	BPath path;
	destdir.GetPath(&path);

	char name[B_FILE_NAME_LENGTH];
	src->GetName(name);
	
	BString newpath = path.Path();
	newpath += name;

	BFile file;
	if (file.SetTo(newpath.String(),
		B_READ_WRITE | B_FAIL_IF_EXISTS) == B_FILE_EXISTS) {
		// We have an existing file, so query the user what to do.
		status_t returncode;

		newpath = B_TRANSLATE_COMMENT("The file '%name%' already exists. Do you"
			" want to replace it?", "Don't translate the variable %name%");
		newpath.ReplaceAll("%name%", name);

		BAlert* alert = new BAlert(B_TRANSLATE("Error"), newpath.String(),
			B_TRANSLATE("Replace file"), B_TRANSLATE("Skip file"),
			B_TRANSLATE("Abort"));
		returncode = alert->Go();
		switch (returncode)
		{
			case 0:
				return FS_CLOBBER;
			case 1:
				return FS_SKIP;
			default:
				return B_CANCELED;
		}
	}
	return B_OK;
}


status_t
CopyFile(BEntry* srcentry, BEntry* destentry, bool clobber)
{
	if (!srcentry || !destentry)
		return B_ERROR;

	if (!destentry->IsDirectory())
		return B_ERROR;

	entry_ref ref;
	srcentry->GetRef(&ref);

	BPath srcpath;
	srcentry->GetPath(&srcpath);

	BString srcstring(srcpath.Path());
	srcstring.ReplaceAll("'", "'\\''");

	BPath destpath;
	destentry->GetPath(&destpath);

	BString deststring(destpath.Path());
	deststring.ReplaceAll("'", "'\\''");

	BString command("copyattr -r -d ");
	command << "'" << srcstring << "' '" << deststring << "/'";
	int code = system(command.String());

	return code;
}


status_t
MoveFile(BEntry* srcentry, BEntry* destentry, bool clobber)
{
	if (!srcentry || !destentry)
		return B_ERROR;

	if (!destentry->IsDirectory())
		return B_ERROR;

	BDirectory destDir(destentry);
	status_t ret = destDir.InitCheck();
	if (ret != B_OK)
		return ret;

	char destLeaf[B_FILE_NAME_LENGTH] = {'\0'};
	srcentry->GetName(destLeaf);
	if (destLeaf == NULL)
		return B_ERROR;

	return srcentry->MoveTo(&destDir, destLeaf, clobber);
}


#if 0
const char* GetValidName(BEntry* entry)
{
	// given a particular location, this will (1) check to see if said entry
	// exists and if it does, generates a filename which will work complete with
	// the full path. The entry is also set to this new, valid path

	BPath path;
	entry->GetPath(&path);

	if (entry->Exists()) {
		// separate into path and leaf
		char leafbase[B_FILE_NAME_LENGTH];
		char newpath[B_PATH_NAME_LENGTH];
		strcpy(leafbase, path.Leaf());
		path.GetParent(&path);

		int32 attempt = 1;
		
		do {
			if (attempt > 1) {
				sprintf(newpath, "%s/%s copy %ld", path.Path(),leafbase,
					attempt);
			} else
				sprintf(newpath, "%s/%s copy", path.Path(),leafbase);

			entry->SetTo(newpath);

			attempt++;
		} while (entry->Exists());

		return newpath;
	}

	return path.Path();
}
#endif
