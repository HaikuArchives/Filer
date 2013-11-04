/*
	FSUtils.h: Utility classes for basic filesystem operations
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/
#ifndef FSUTILS_H_
#define FSUTILS_H_

#include <Errors.h>

#define FS_CLOBBER 'fscl'
#define FS_SKIP 'fssk'

status_t CheckCopiable(BEntry *src, BEntry *dest);
status_t CopyFile(BEntry *src,BEntry *dest, bool clobber);
status_t MoveFile(BEntry *src,BEntry *dest, bool clobber);
const char *GetValidName(BEntry *entry);
bool IsFilenameChar(char c);
int charcmp(char c1, char c2);
int charncmp(char c1, char c2);
#endif