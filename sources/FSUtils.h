/*
 * Copyright 2008, DarkWyrm <darkwyrm@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef FSUTILS_H_
#define FSUTILS_H_

#include <Entry.h>
#include <Errors.h>

#define FS_CLOBBER 'fscl'
#define FS_SKIP 'fssk'

status_t	CheckCopiable(BEntry* src, BEntry* dest);
status_t	CopyFile(BEntry* src, BEntry* dest, bool clobber);
status_t	MoveFile(BEntry* src, BEntry* dest, bool clobber);

const char*	GetValidName(BEntry* entry);

#endif	// FSUTILS_H_
