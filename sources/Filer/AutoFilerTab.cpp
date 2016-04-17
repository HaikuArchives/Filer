/*
 * Copyright 2008, 2016. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *  DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
 *	Humdinger, humdingerb@gmail.com
 */
#include <Alert.h>
#include <Application.h>
#include <ControlLook.h>
#include <FindDirectory.h>
#include <LayoutBuilder.h>
#include <Path.h>
#include <Roster.h>
#include <ScrollView.h>
#include <SeparatorView.h>
#include <StringView.h>
#include <View.h>

#include "AutoFilerTab.h"
#include "RefStorage.h"
#include "TypedRefFilter.h"

enum 
{
	M_SHOW_ADD_PANEL = 'shaw',
	M_SHOW_EDIT_PANEL = 'shew',
	M_REMOVE_FOLDER = 'rmfl',
	M_FOLDER_SELECTED = 'flsl',
	M_FOLDER_CHOSEN = 'flch'
};


AutoFilerTab::AutoFilerTab()
	:
	BView("AutoFiler", B_SUPPORTS_LAYOUT)
{
	LoadFolders();
	_BuildLayout();

//	fRefFilter = new TypedRefFilter("application/x-vnd.Be-directory",B_DIRECTORY_NODE);
	fRefFilter = new TypedRefFilter("", B_DIRECTORY_NODE);
	fFilePanel = new BFilePanel(B_OPEN_PANEL, new BMessenger(this), NULL,
		B_DIRECTORY_NODE, false, NULL, fRefFilter);
	BMessage panelMsg(M_FOLDER_CHOSEN);
	fFilePanel->SetMessage(&panelMsg);
	
	gRefLock.Lock();
	for (int32 i = 0; i < gRefStructList.CountItems(); i++)
	{
		RefStorage* refholder = (RefStorage*)gRefStructList.ItemAt(i);
		fFolderList->AddItem(new BStringItem(BPath(&refholder->ref).Path()));
	}
	gRefLock.Unlock();
	
	fFolderList->MakeFocus();
	if (fFolderList->CountItems() > 0)
		fFolderList->Select(0L);
}


AutoFilerTab::~AutoFilerTab()
{
	delete fFilePanel;
}


void
AutoFilerTab::_BuildLayout()
{
	BStringView* folderLabel = new BStringView("folderlabel",
		"Automatically run Filer on the contents of these folders:");
	
	fFolderList = new BListView("rulelist", B_SINGLE_SELECTION_LIST,
		B_WILL_DRAW	| B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE | B_NAVIGABLE);
	fScrollView = new BScrollView("listscroll", fFolderList,
		B_FRAME_EVENTS | B_WILL_DRAW, false, true);
	fFolderList->SetSelectionMessage(new BMessage(M_FOLDER_SELECTED));
	fFolderList->SetInvocationMessage(new BMessage(M_SHOW_EDIT_PANEL));

	fAutorunBox = new BCheckBox("autorunbox",
		"Run AutoFiler on system startup", new BMessage);

	BPath path;
	find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	path.Append(gPrefsPath);

	BNode node(path.Path());
	bool autorun = true;
	if (node.InitCheck() == B_OK) {
		bool tmpbool;
		if (node.ReadAttr("autorun", B_BOOL_TYPE, 0, (void*)&tmpbool,
				sizeof(bool)) > 0)
			autorun = tmpbool;
	}
	if (autorun)
		fAutorunBox->SetValue(B_CONTROL_ON);
	
	fAddButton = new BButton("addbutton", "Add" B_UTF8_ELLIPSIS,
		new BMessage(M_SHOW_ADD_PANEL));
	
	fEditButton = new BButton("editbutton", "Edit" B_UTF8_ELLIPSIS,
		new BMessage(M_SHOW_EDIT_PANEL));
	fEditButton->SetEnabled(false);
		
	fRemoveButton = new BButton("removebutton", "Remove",
		new BMessage(M_REMOVE_FOLDER));
	fRemoveButton->SetEnabled(false);

	static const float spacing = be_control_look->DefaultItemSpacing();
	BLayoutBuilder::Group<>(this, B_HORIZONTAL, B_USE_DEFAULT_SPACING)
		.SetInsets(spacing)
		.AddGroup(B_VERTICAL)
			.Add(fAutorunBox)
			.Add(new BSeparatorView(B_HORIZONTAL))
			.Add(folderLabel)
			.Add(fScrollView)
			.AddGroup(B_HORIZONTAL)
				.AddGlue()
				.Add(fAddButton)
				.Add(fEditButton)
				.Add(fRemoveButton)
				.AddGlue()
			.End()
		.End();
}


void
AutoFilerTab::AttachedToWindow()
{
//	SetFlags(Flags() | B_FULL_UPDATE_ON_RESIZE | B_NAVIGABLE);

	fAddButton->SetTarget(this);
	fEditButton->SetTarget(this);
	fRemoveButton->SetTarget(this);
	
	fFolderList->SetTarget(this);
	fFilePanel->SetTarget(this);

	if (fFolderList->CountItems() > 0) {
		BMessenger messenger(this);
		BMessage message(M_FOLDER_SELECTED);
		messenger.SendMessage(&message);
	}	
	BView::AttachedToWindow();
}


void
AutoFilerTab::DetachedFromWindow()
{
	printf("AutoFilerTab: QuitRequested()\n");
	SaveFolders();
	
	// save autorun value
	bool autorun = (fAutorunBox->Value() == B_CONTROL_ON);
	
	BPath path;
	find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	path.Append(gPrefsPath);

	BNode node(path.Path());
	if (node.InitCheck() == B_OK)
		node.WriteAttr("autorun",B_BOOL_TYPE,0,(void*)&autorun,sizeof(bool));
	
	if (autorun)
		EnableAutorun();
	else
		DisableAutorun();
	
	// if AutoFiler is running, tell it to refresh its folders
	if (be_roster->IsRunning("application/x-vnd.dw-AutoFiler"))
	{
		BMessage msg(M_REFRESH_FOLDERS);
		BMessenger msgr("application/x-vnd.dw-AutoFiler");
		msgr.SendMessage(&msg);
	}
}


void
AutoFilerTab::MessageReceived(BMessage *message)
{
	switch (message->what)
	{
		case M_SHOW_ADD_PANEL:
		{
			fFilePanel->Show();
			break;
		}
		case M_SHOW_EDIT_PANEL:
		{
			int32 selection = fFolderList->CurrentSelection();
			if (selection < 0)
				break;

			BStringItem* item = (BStringItem*)fFolderList->ItemAt(selection);
			fFilePanel->SetPanelDirectory(item->Text());

			BMessage panelMsg(M_FOLDER_CHOSEN);
			panelMsg.AddInt32("index", selection);
			fFilePanel->SetMessage(&panelMsg);
			fFilePanel->Show();
			break;
		}
		case M_REMOVE_FOLDER:
		{
			int32 selection = fFolderList->CurrentSelection();
			if (selection < 0)
				break;

			BStringItem* item = (BStringItem*)fFolderList->RemoveItem(selection);
			delete item;

			gRefLock.Lock();
			RefStorage* refholder = (RefStorage*)gRefStructList.RemoveItem(selection);
			delete refholder;
			gRefLock.Unlock();
			break;
		}
		case M_FOLDER_SELECTED:
		{
			int32 selection = fFolderList->CurrentSelection();
			bool value = (selection >= 0);

			fEditButton->SetEnabled(value);
			fRemoveButton->SetEnabled(value);
			break;
		}
		case M_FOLDER_CHOSEN:
		{
			int32 index;
			if (message->FindInt32("index", &index) != B_OK)
				index = -1;
			
			entry_ref ref;
			if (message->FindRef("refs", &ref) != B_OK)
				break;
			
			BStringItem* item = (BStringItem*)fFolderList->ItemAt(index);
			if (item) {
				gRefLock.Lock();
				RefStorage* refholder = (RefStorage*)gRefStructList.ItemAt(index);
				refholder->SetData(ref);
				gRefLock.Unlock();
			} else {
				item = new BStringItem("");
				fFolderList->AddItem(item);

				gRefLock.Lock();
				gRefStructList.AddItem(new RefStorage(ref));
				gRefLock.Unlock();
			}

			item->SetText(BPath(&ref).Path());
			fFolderList->Invalidate();
			break;
		}
		default:
			BView::MessageReceived(message);
	}
}


void
AutoFilerTab::EnableAutorun()
{
	BDirectory destDir;
	BPath destPath;
	find_directory(B_USER_SETTINGS_DIRECTORY, &destPath);

	status_t ret = destPath.Append("boot");
	if (ret == B_OK)
		ret = create_directory(destPath.Path(), 0777);

	ret = destPath.Append("launch");
	if (ret == B_OK)
		ret = create_directory(destPath.Path(), 0777);

	if (ret == B_OK) {
		destDir = BDirectory(destPath.Path());
		ret = destDir.InitCheck();
	}

	if (ret != B_OK)
		return;

	destPath.Append("AutoFiler");

	app_info info;
	BPath linkPath;
	be_roster->GetActiveAppInfo(&info);
	BEntry entry(&info.ref);

	entry.GetPath(&linkPath);
	linkPath.GetParent(&linkPath);
	linkPath.Append("AutoFiler");
	destDir.CreateSymLink(destPath.Path(), linkPath.Path(), NULL);
}


void
AutoFilerTab::DisableAutorun()
{
	BPath path;

	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) < B_OK)
		return;
	status_t ret = path.Append("boot/launch/AutoFiler");

	if (ret == B_OK) {
		BEntry entry(path.Path());
		entry.Remove();
	}
}
