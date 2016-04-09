/*
	Released under the MIT license.
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Contributed by: Humdinger <humdingerb@gmail.com>, 2016
*/
#include <Application.h>
#include <File.h>
#include <FindDirectory.h>
#include <Messenger.h>
#include <Node.h>
#include <Path.h>
#include <Roster.h>
#include <StringView.h>
#include <SymLink.h>
#include <TypeConstants.h>

#include "PrefWindow.h"
#include "RefStorage.h"
#include "TypedRefFilter.h"

#include <stdio.h>


const BRect kDefaultFrame(100,100,500,400);

enum 
{
	M_SHOW_ADD_PANEL = 'shaw',
	M_SHOW_EDIT_PANEL = 'shew',
	M_REMOVE_FOLDER = 'rmfl',
	M_FOLDER_SELECTED = 'flsl',
	M_FOLDER_CHOSEN = 'flch'
};

PrefWindow::PrefWindow(void)
 :	BWindow(LoadFrame(),"AutoFiler Settings",B_TITLED_WINDOW,
 			B_ASYNCHRONOUS_CONTROLS)
{
	LoadFolders();
	
	AddShortcut('a',B_COMMAND_KEY,new BMessage(M_SHOW_ADD_PANEL));
	AddShortcut('c',B_COMMAND_KEY,new BMessage(M_SHOW_EDIT_PANEL));
	
	BView *top = new BView(Bounds(),"top",B_FOLLOW_ALL,B_WILL_DRAW);
	top->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(top);
	
	BStringView *folderLabel = new BStringView(BRect(10,5,11,6),"folderlabel",
		"Automatically run Filer on the contents of these folders:");
	folderLabel->ResizeToPreferred();
	top->AddChild(folderLabel);
	
	BRect rect(Bounds().InsetByCopy(10,10));
	rect.top = folderLabel->Frame().bottom + 5;
	rect.right -= B_V_SCROLL_BAR_WIDTH;
	
	fFolderList = new BListView(rect,"rulelist",B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL);
	fScrollView = new BScrollView("listscroll",fFolderList,
												B_FOLLOW_ALL,0,true,true);
	fScrollView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	top->AddChild(fScrollView);
	fFolderList->SetSelectionMessage(new BMessage(M_FOLDER_SELECTED));
	fFolderList->SetInvocationMessage(new BMessage(M_SHOW_EDIT_PANEL));
	fScrollView->ScrollBar(B_HORIZONTAL)->SetRange(0.0,0.0);
	
	fAutorunBox = new BCheckBox(BRect(0,0,1,1),"autorunbox","Run AutoFiler on system startup",
								new BMessage, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	fAutorunBox->ResizeToPreferred();
	fAutorunBox->MoveTo((Bounds().Width() - fAutorunBox->Bounds().Width()) / 2.0,
						Bounds().bottom - fAutorunBox->Bounds().Height() - 10.0);
	// add as child later

	BPath path;
	find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	path.Append(gPrefsPath);

	BNode node(path.Path());
	bool autorun = true;
	if (node.InitCheck() == B_OK)
	{
		bool tmpbool;
		if (node.ReadAttr("autorun",B_BOOL_TYPE,0,(void*)&tmpbool,sizeof(bool)) > 0)
			autorun = tmpbool;
	}
	if (autorun)
		fAutorunBox->SetValue(B_CONTROL_ON);
	
	fAddButton = new BButton(BRect(0,0,1,1),"addbutton","Add…",
									new BMessage(M_SHOW_ADD_PANEL),
									B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	fAddButton->ResizeToPreferred();
	fAddButton->MoveTo(10,fAutorunBox->Frame().top - 5.0 - fAddButton->Bounds().IntegerHeight());
	top->AddChild(fAddButton);
	
	fChangeButton = new BButton(BRect(0,0,1,1),"editbutton","Edit…",
									new BMessage(M_SHOW_EDIT_PANEL),
									B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	fChangeButton->ResizeToPreferred();
	fChangeButton->MoveTo((Bounds().Width() - fChangeButton->Bounds().Width()) / 2.0,
					fAddButton->Frame().top);
	top->AddChild(fChangeButton);
	fChangeButton->SetEnabled(false);
	
	
	fRemoveButton = new BButton(BRect(0,0,1,1),"removebutton","Remove",
									new BMessage(M_REMOVE_FOLDER),
									B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	fRemoveButton->ResizeToPreferred();
	fRemoveButton->MoveTo(Bounds().Width() - fRemoveButton->Bounds().Width() - 10,
					fAddButton->Frame().top);
	top->AddChild(fRemoveButton);
	fRemoveButton->SetEnabled(false);
	
	top->AddChild(fAutorunBox);
	
	fScrollView->ResizeTo(Bounds().Width() - 20.0, fAddButton->Frame().top - 10.0 - 
													fScrollView->Frame().top);
	
	float minwidth = (fRemoveButton->Bounds().Width() * 3.0) + 40;
	SetSizeLimits(minwidth, 30000, 200, 30000);
	
//	fRefFilter = new TypedRefFilter("application/x-vnd.Be-directory",B_DIRECTORY_NODE);
	fRefFilter = new TypedRefFilter("",B_DIRECTORY_NODE);
	fFilePanel = new BFilePanel(B_OPEN_PANEL,new BMessenger(this), NULL,B_DIRECTORY_NODE,false,
								NULL,fRefFilter);
	
	gRefLock.Lock();
	for (int32 i = 0; i < gRefStructList.CountItems(); i++)
	{
		RefStorage *refholder = (RefStorage*)gRefStructList.ItemAt(i);
		fFolderList->AddItem(new BStringItem(BPath(&refholder->ref).Path()));
	}
	gRefLock.Unlock();
	
	fFolderList->MakeFocus();
	if (fFolderList->CountItems() > 0)
		fFolderList->Select(0L);
}


PrefWindow::~PrefWindow(void)
{
	delete fFilePanel;
}


bool
PrefWindow::QuitRequested(void)
{
	SaveFolders();
	SaveFrame();
	
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
	
	
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}


void
PrefWindow::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		case M_SHOW_ADD_PANEL:
		{
			BMessage panelmsg(M_FOLDER_CHOSEN);
			fFilePanel->SetMessage(&panelmsg);
			fFilePanel->Show();
			break;
		}
		case M_SHOW_EDIT_PANEL:
		{
			int32 selection = fFolderList->CurrentSelection();
			if (selection < 0)
				break;
			
			BStringItem *item = (BStringItem*)fFolderList->ItemAt(selection);
			fFilePanel->SetPanelDirectory(item->Text());
			
			BMessage panelmsg(M_FOLDER_CHOSEN);
			panelmsg.AddInt32("index",selection);
			fFilePanel->SetMessage(&panelmsg);
			fFilePanel->Show();
			break;
		}
		case M_REMOVE_FOLDER:
		{
			int32 selection = fFolderList->CurrentSelection();
			if (selection < 0)
				break;
			
			BStringItem *item = (BStringItem*)fFolderList->RemoveItem(selection);
			delete item;
			
			gRefLock.Lock();
			RefStorage *refholder = (RefStorage*) gRefStructList.RemoveItem(selection);
			delete refholder;
			gRefLock.Unlock();
			
			break;
		}
		case M_FOLDER_SELECTED:
		{
			int32 selection = fFolderList->CurrentSelection();
			
			bool value = (selection >= 0);
			
			fChangeButton->SetEnabled(value);
			fRemoveButton->SetEnabled(value);
			
			break;
		}
		case M_FOLDER_CHOSEN:
		{
			int32 index;
			if (msg->FindInt32("index",&index) != B_OK)
				index = -1;
			
			entry_ref ref;
			if (msg->FindRef("refs",&ref) != B_OK)
				break;
			
			BStringItem *item = (BStringItem*) fFolderList->ItemAt(index);
			if (item)
			{
				gRefLock.Lock();
				RefStorage *refholder = (RefStorage*) gRefStructList.ItemAt(index);
				refholder->SetData(ref);
				gRefLock.Unlock();
			}
			else
			{
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
			BWindow::MessageReceived(msg);
	}
}


BRect
PrefWindow::LoadFrame(void)
{
	BRect frame(kDefaultFrame);
	
	BPath path;
	find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	path.Append(gPrefsPath);

	BNode node(path.Path());
	if (node.InitCheck() == B_OK)
	{
		BRect r;
		if (node.ReadAttr("windowframe",B_RECT_TYPE,0,(void*)&r,sizeof(BRect)) > 0)
			frame = r;
	}
	
	return frame;
}


void
PrefWindow::SaveFrame(void)
{
	BRect r(Frame());
	BPath path;
	find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	path.Append(gPrefsPath);

	BNode node(path.Path());
	if (node.InitCheck() == B_OK)
		node.WriteAttr("windowframe",B_RECT_TYPE,0,(void*)&r,sizeof(BRect));
}


void
PrefWindow::FrameResized(float width, float height)
{
	float x = fAddButton->Frame().right + ((fRemoveButton->Frame().left - 
											fAddButton->Frame().right) / 2.0);
	fChangeButton->MoveTo(x - (fChangeButton->Bounds().Width() / 2.0),
						fAddButton->Frame().top);
}


void
PrefWindow::EnableAutorun(void)
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
PrefWindow::DisableAutorun(void)
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

