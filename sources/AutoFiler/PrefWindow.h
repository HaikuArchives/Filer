#ifndef PREFWIN_H
#define PREFWIN_H

#include <Window.h>
#include <ScrollView.h>
#include <ListView.h>
#include <Button.h>
#include <Message.h>
#include <FilePanel.h>
#include <CheckBox.h>

class TypedRefFilter;

class PrefWindow : public BWindow
{
public:
					PrefWindow(void);
					~PrefWindow(void);
			bool	QuitRequested(void);
			void	MessageReceived(BMessage *msg);
			void	FrameResized(float width, float height);

private:
			BRect	LoadFrame(void);
			void	SaveFrame(void);
			void	EnableAutorun(void);
			void	DisableAutorun(void);
			
	BListView		*fFolderList;
	BScrollView		*fScrollView;
	
	BButton			*fAddButton,
					*fChangeButton,
					*fRemoveButton;
					
	BFilePanel		*fFilePanel;
	TypedRefFilter	*fRefFilter;
	
	BCheckBox		*fAutorunBox;
};

#endif
