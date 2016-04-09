/*
	AutoFiler: Watcher daemon which executes the Filer on monitored folders
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/
#ifndef MAIN_H
#define MAIN_H

#include <Application.h>
#include <Message.h>

class App : public BApplication
{
public:
			App(void);
	void	MessageReceived(BMessage *msg);

private:
	void	HandleNodeMonitoring(BMessage *msg);
	void	StartWatching(void);
	void	StopWatching(void);
};

#endif
