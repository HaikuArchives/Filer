/*
	PrefsWindow.h: Window class to show and edit settings for the Filer
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/
#ifndef PREFS_WIN_H
#define PREFS_WIN_H

#include <Window.h>
#include <ListView.h>
#include <Button.h>
#include "ObjectList.h"

class RuleItem;
class FilerRule;

#define M_CLOSE_APP 'clar'


class PrefsWindow : public BWindow
{
public:
			PrefsWindow(void);
			~PrefsWindow(void);
	bool	QuitRequested(void);
	void	MessageReceived(BMessage *msg);
	void	FrameResized(float width, float height);
	
private:
	void	AddRule(FilerRule *rule);
	void	RemoveRule(RuleItem *item);
	void	MakeEmpty(void);
	
	BObjectList<FilerRule>	*fRuleList;
	
	BListView	*fRuleItemList;
	
	BButton		*fAddButton,
				*fEditButton,
				*fRemoveButton,
				*fMoveUpButton,
				*fMoveDownButton;
				
	BScrollView	*fScrollView;
	bool		fChanges;
};

#endif
