#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Window.h>
#include <Message.h>
#include <ListView.h>
#include <ScrollView.h>
#include <FilePanel.h>
#include <Button.h>
#include <Locker.h>


class MainWindow : public BWindow
{
public:
					MainWindow(BRect frame);
			void	MessageReceived(BMessage *msg);
			bool	QuitRequested(void);

private:
	BListView	*fFolderList;
	BButton		*fAddButton,
				*fChangeButton,
				*fRemoveButton;
};


#endif
