/*
	FSUtils.cpp: Utility classes for basic filesystem operations
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/
#include <OS.h>
#include <Volume.h>
#include <Directory.h>
#include <File.h>
#include <Entry.h>
#include <Path.h>
#include <Alert.h>
#include <stdio.h>
#include <stdlib.h>
#include <fs_attr.h>
#include <String.h>
#include <string.h>
#include <Errors.h>
#include "FSUtils.h"

#define COPY_BUFFER_SIZE 1024000

status_t CheckCopiable(BEntry *src,BEntry *dest)
{
	// Checks to see if we can copy the src to dest.
	if(!src || !dest)
		return B_ERROR;
	
	if(!src->Exists())
		return B_FILE_NOT_FOUND;

	// Ensure that when we want the destination directory, that is exactly
	// what we're working with. If we've been given an entry which is a file,
	// extract the destination directory.
	BEntry destdir;		
	if(dest->IsDirectory())
		destdir=*dest;
	else
		dest->GetParent(&destdir);
	
	// check existence of target directory
	if(!destdir.Exists())
		return B_NAME_NOT_FOUND;
		
	// check space
	entry_ref ref;
	off_t src_bytes;


	dest->GetRef(&ref);
	BVolume dvolume(ref.device);
	src->GetSize(&src_bytes);

	if(src_bytes>dvolume.FreeBytes())
		return B_DEVICE_FULL;
	
	// check permissions
	if(dvolume.IsReadOnly())
		return B_READ_ONLY;

	// check existing name
	BPath path;
	destdir.GetPath(&path);
	

	char name[B_FILE_NAME_LENGTH];
	src->GetName(name);
	
	BString newpath=path.Path();
	newpath+=name;

	BFile file;
	if(file.SetTo(newpath.String(),B_READ_WRITE | B_FAIL_IF_EXISTS)==B_FILE_EXISTS)
	{
		// We have an existing file, so query the user what to do.
		status_t returncode;

		newpath="The file ";
		newpath+=name;
		newpath+=" exists. Do you want to replace it?";
		
		BAlert *alert=new BAlert("Error",newpath.String(),"Replace File",
		 "Skip File", "Stop");
		returncode=alert->Go();
		switch(returncode)
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

status_t CopyFile(BEntry *srcentry, BEntry *destentry, bool clobber)
{
	if(!srcentry || !destentry)
		return B_ERROR;
	
	if (!destentry->IsDirectory())
		return B_ERROR;
		
	entry_ref ref;
	srcentry->GetRef(&ref);
	
	BPath srcpath;
	srcentry->GetPath(&srcpath);
	
	BString srcstring(srcpath.Path());
	srcstring.CharacterEscape("'",'\\');
	
	BPath destpath;
	destentry->GetPath(&destpath);
	
	BString deststring(destpath.Path());
	deststring.CharacterEscape("'",'\\');
	
	BString command("copyattr -r -d ");
	command << "'" << srcstring << "' '" << deststring << "/'";
	int code = system(command.String());
	
	if (!code)
	{
		entry_ref ref;
		srcentry->GetRef(&ref);
		
		deststring << "/" << ref.name;
		return srcentry->SetTo(deststring.String());
	}
		
	return code;
}

status_t MoveFile(BEntry *srcentry,BEntry *destentry, bool clobber)
{
	if(!srcentry || !destentry)
		return B_ERROR;
	
	if (!destentry->IsDirectory())
		return B_ERROR;
	
	BPath srcpath;
	srcentry->GetPath(&srcpath);
	
	BString srcstring(srcpath.Path());
	srcstring.CharacterEscape("'",'\\');
	
	BPath destpath;
	destentry->GetPath(&destpath);
	
	BString deststring(destpath.Path());
	deststring.CharacterEscape("'",'\\');
	
	BString command("mv ");
	if (clobber)
		command << "-f ";
	command << "'" << srcstring << "' '" << deststring << "/'";
	int code = system(command.String());

	if (!code)
	{
		entry_ref ref;
		srcentry->GetRef(&ref);
		
		deststring << "/" << ref.name;
		return srcentry->SetTo(deststring.String());
	}
		
	return code;
}

const char *GetValidName(BEntry *entry)
{
	// given a particular location, this will (1) check to see if said entry
	// exists and if it does, generates a filename which will work complete with
	// the full path. The entry is also set to this new, valid path
	
	BPath path;
	entry->GetPath(&path);

	if(entry->Exists())
	{
		// separate into path and leaf
		char leafbase[B_FILE_NAME_LENGTH];
		char newpath[B_PATH_NAME_LENGTH];
		strcpy(leafbase, path.Leaf());
		path.GetParent(&path);
		
		int32 attempt=1;
		
		do
		{
			if(attempt>1)
				sprintf(newpath, "%s/%s copy %ld", path.Path(),leafbase, attempt);
			else
				sprintf(newpath, "%s/%s copy", path.Path(),leafbase);

			entry->SetTo(newpath);

			attempt++;
		} while (entry->Exists());
		
		return newpath;
	}
	
	return path.Path();
}

bool IsFilenameChar(char c)
{
//	const char validstring[]="1234567890-_ ~.,+=!@#$%^&[]{}";
	const char validstring[]="1234567890-_~.,+=!@#$%^&[]{}";
	if( (c>64 && c<91) || (c>96 && c<123))
		return true;
	int validlen=strlen(validstring);
	for(int i=0;i<validlen;i++)
	{
		if(c==validstring[i])
			return true;
	}
	return false;
}

int charcmp(char c1, char c2)
{
	// Case-sensitive character compare
	if(c1<c2)
		return -1;
	else
		if(c2<c1)
			return 1;
	return 0;
}

int charncmp(char c1, char c2)
{
	if(c1>96 && c1<123)
		c1-=32;
	if(c2>96 && c2<123)
		c2-=32;

	if(c1<c2)
		return -1;
	else
		if(c2<c1)
			return 1;
	return 0;
}
