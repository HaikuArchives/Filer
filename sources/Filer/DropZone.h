/*
 * Copyright 2016. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */
#ifndef DROPZONE_H
#define DROPZONE_H

#include <View.h>

const pattern stripePattern = {0xcc, 0x66, 0x33, 0x99, 0xcc, 0x66, 0x33, 0x99};
	
class DropZone : public BView
{
public:
					DropZone();
					~DropZone();

	virtual void	Draw(BRect rect);
	void			MessageReceived(BMessage* message);
};


#endif // DROPZONE_H
