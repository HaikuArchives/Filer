/*
	AutoFiler: Watcher daemon which executes the Filer on monitored folders
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/

#ifndef AUTOFILER_H
#define AUTOFILER_H

#include <Application.h>
#include <Message.h>

class App : public BApplication
{
public:
			App();
	void	MessageReceived(BMessage* msg);

private:
	void	HandleNodeMonitoring(BMessage* msg);
	void	StartWatching();
	void	StopWatching();
};

#endif	// AUTOFILER_H
