/*
 * Copyright 2016. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#ifndef DROPZONETAB_H
#define DROPZONETAB_H

#include <View.h>

#include "DropZone.h"

class DropZoneTab : public BView
{
public:
					DropZoneTab();
					~DropZoneTab();
	
private:
	DropZone*		fDropzone;
};


#endif // DROPZONETAB_H
