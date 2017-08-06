/*
 * Copyright 2008, 2016. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Authors:
 *  DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
 *	Humdinger, humdingerb@gmail.com
 *	Owen Pan <owen.pan@yahoo.com>, 2017
 */

#include "AutoFilerTab.h"

#include <stdio.h>
#include <stdlib.h>

#include <Catalog.h>
#include <ControlLook.h>
#include <FindDirectory.h>
#include <LayoutBuilder.h>
#include <Path.h>
#include <Roster.h>
#include <SeparatorView.h>
#include <StringView.h>

#include "FilerDefs.h"
#include "RefStorage.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "AutoFilerTab"

static const char* const kStartAutoFiler = B_TRANSLATE("Start AutoFiler");
static const char* const kStopAutoFiler = B_TRANSLATE("Stop AutoFiler");


AutoFilerTab::AutoFilerTab()
	:
	BView(B_TRANSLATE("AutoFiler"), B_SUPPORTS_LAYOUT),
	fDirtySettings(false)
{
	_BuildLayout();
	LoadFolders(fFolderList);

//	fRefFilter = new TypedRefFilter("application/x-vnd.Be-directory",
//		B_DIRECTORY_NODE);
	fRefFilter = new TypedRefFilter("", B_DIRECTORY_NODE);

	BMessage panelMsg(MSG_FOLDER_CHOSEN);
	fFilePanel = new BFilePanel(B_OPEN_PANEL, NULL, NULL,
		B_DIRECTORY_NODE, true, &panelMsg, fRefFilter);
	fFilePanel->Window()->SetTitle(B_TRANSLATE("AutoFiler: Add folders"));
	fFilePanel->SetButtonLabel(B_DEFAULT_BUTTON,
		B_TRANSLATE_COMMENT("Add", "Used as button in a file panel"));

	fFolderList->MakeFocus();
	if (fFolderList->CountItems() > 0)
		fFolderList->Select(0L);
}


AutoFilerTab::~AutoFilerTab()
{
	delete fRefFilter;
	delete fFilePanel;
}


void
AutoFilerTab::_BuildLayout()
{
	BStringView* folderLabel = new BStringView("folderlabel",
		B_TRANSLATE("Automatically run Filer on the contents of these folders:"));

	fFolderList = new AutoFilerList("rulelist", this);
	fScrollView = new BScrollView("listscroll", fFolderList,
		B_FRAME_EVENTS | B_WILL_DRAW, false, true);
	fFolderList->SetSelectionMessage(new BMessage(MSG_FOLDER_SELECTED));

	fAutorunBox = new BCheckBox("autorunbox",
		B_TRANSLATE("Run AutoFiler on system startup"),
		new BMessage(MSG_AUTOFILER_AUTORUN));

	fStartStop = new BButton("startstop", kStartAutoFiler,
		new BMessage(MSG_STARTSTOP_AUTOFILER));

	BPath path;
	find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	path.Append(gPrefsPath);

	BNode node(path.Path());
	bool autorun = false;
	if (node.InitCheck() == B_OK) {
		bool tmpbool;
		if (node.ReadAttr("autorun", B_BOOL_TYPE, 0, (void*)&tmpbool,
				sizeof(bool)) > 0)
			autorun = tmpbool;
	}
	if (autorun)
		fAutorunBox->SetValue(B_CONTROL_ON);

	fAddButton = new BButton("addbutton", "+",
		new BMessage(MSG_SHOW_ADD_PANEL));

	fRemoveButton = new BButton("removebutton", kEnDash,
		new BMessage(MSG_REMOVE_FOLDER));
	fRemoveButton->SetEnabled(false);

	float height;
	fStartStop->GetPreferredSize(NULL, &height);

	BSize size(height, height);
	fAddButton->SetExplicitSize(size);
	fRemoveButton->SetExplicitSize(size);

	static const float spacing = be_control_look->DefaultItemSpacing();
	BLayoutBuilder::Group<>(this, B_HORIZONTAL, B_USE_DEFAULT_SPACING)
		.SetInsets(spacing)
		.AddGroup(B_VERTICAL)
			.AddGroup(B_HORIZONTAL)
				.Add(fAutorunBox)
				.Add(new BSeparatorView(B_VERTICAL))
				.Add(fStartStop)
				.End()
			.Add(new BSeparatorView(B_HORIZONTAL))
			.Add(folderLabel)
			.AddGroup(B_HORIZONTAL)
				.Add(fScrollView)
				.AddGroup(B_VERTICAL)
					.Add(fAddButton)
					.Add(fRemoveButton)
					.AddGlue()
					.End()
				.End()
			.End()
		.End();

	fAddButton->SetToolTip(B_TRANSLATE("Add folder"));
	fRemoveButton->SetToolTip(B_TRANSLATE("Remove folder"));
}


void
AutoFilerTab::AttachedToWindow()
{
	fAutorunBox->SetTarget(this);
	fStartStop->SetTarget(this);
	fAddButton->SetTarget(this);
	fRemoveButton->SetTarget(this);

	fFolderList->SetTarget(this);
	fFilePanel->SetTarget(this);

	BMessenger messenger(this);
	if (fFolderList->CountItems() > 0) {
		BMessage msg(MSG_FOLDER_SELECTED);
		messenger.SendMessage(&msg);
	}
	BMessage msg(MSG_UPDATE_LABEL);
	messenger.SendMessage(&msg);
	fRunner	= new BMessageRunner(this, &msg, 0.5 * 6000000); // x * seconds

	BView::AttachedToWindow();
}


void
AutoFilerTab::DetachedFromWindow()
{
	if (!fDirtySettings)
		return;
	
	// save autorun value
	bool autorun = (fAutorunBox->Value() == B_CONTROL_ON);
	
	BPath path;
	find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	path.Append(gPrefsPath);

	BNode node(path.Path());
	if (node.InitCheck() == B_OK) {
		node.WriteAttr("autorun", B_BOOL_TYPE, 0,
			(void*)&autorun, sizeof(bool));
	}
	ToggleAutorun(autorun);
}


void
AutoFilerTab::MessageReceived(BMessage* msg)
{
	if (msg->WasDropped()) {
		BMessenger msgr(this);
		entry_ref tempRef;
		int32 i = 0;
		while (msg->FindRef("refs", i, &tempRef) == B_OK)
		{
			BEntry entry(&tempRef);
			if (entry.Exists()) {
				BMessage newMsg(MSG_FOLDER_CHOSEN);
				newMsg.AddRef("refs", &tempRef);
				msgr.SendMessage(&newMsg);
			} else
				printf("Couldn't find file %s\n",tempRef.name);
			i++;
		}
	}
	switch (msg->what)
	{
		case MSG_AUTOFILER_AUTORUN:
		{
			ToggleAutorun(fAutorunBox->Value() == B_CONTROL_ON);
			break;
		}
		case MSG_STARTSTOP_AUTOFILER:
		{
			ToggleAutoFiler();
			break;
		}
		case MSG_UPDATE_LABEL:
		{
			UpdateAutoFilerLabel();
			break;
		}
		case MSG_SHOW_ADD_PANEL:
		{
			fFilePanel->Show();
			break;
		}
		case MSG_REMOVE_FOLDER:
		{
			int32 selection = fFolderList->CurrentSelection();
			if (selection < 0)
				break;

			BStringItem* item = (BStringItem*)fFolderList->RemoveItem(selection);
			delete item;

			int32 count = fFolderList->CountItems();
			fFolderList->Select((selection > count - 1) ? count - 1 : selection);

			UpdateAutoFilerFolders();
			break;
		}
		case MSG_FOLDER_SELECTED:
		{
			int32 selection = fFolderList->CurrentSelection();
			bool value = (selection >= 0);

			fRemoveButton->SetEnabled(value);
			break;
		}
		case MSG_FOLDER_CHOSEN:
		{
			entry_ref ref;
			bool added = false;
			int32 count = fFolderList->CountItems();

			for (int32 i = 0; msg->FindRef("refs", i, &ref) == B_OK; i++) {
				BEntry entry(&ref);
				BPath path;
				entry.GetPath(&path);
				BString newpath(path.Path());

				int32 index = 0;
				int compare = 1;

				while (index < count) {
					BString str = ((BStringItem*) fFolderList->ItemAt(index))->Text();
					int iCompare = newpath.ICompare(str);

					if (iCompare < 0 || iCompare == 0 && (compare = newpath.Compare(str)) <= 0)
						break;

					index++;
				}

				if (compare == 0)
					continue;

				fFolderList->AddItem(new BStringItem(newpath), index);
				added = true;
				count++;
			}

			if (added) {
				fFolderList->Invalidate();
				UpdateAutoFilerFolders();
			}

			break;
		}
		default:
			BView::MessageReceived(msg);
	}
}


bool
AutoFilerTab::IsFolderUnique(BString newpath)
{
	if (fFolderList->IsEmpty())
		return true;

	bool unique = true;
	for (int i = 0; i < fFolderList->CountItems(); i++) {
		BStringItem* sItem = (BStringItem*)fFolderList->ItemAt(i);
		if (newpath.Compare(sItem->Text()) == 0)
			unique = false;
	}
	return unique;
}


void
AutoFilerTab::ToggleAutoFiler()
{
	if (be_roster->IsRunning(kAutoFilerSignature)) {
		BString command = "hey %app% quit";
		command.ReplaceFirst("%app%", kAutoFilerSignature);
		system(command.String());
		fStartStop->SetLabel(kStartAutoFiler);
	} else {
		be_roster->Launch(kAutoFilerSignature);
		fStartStop->SetLabel(kStopAutoFiler);
	}
}


void
AutoFilerTab::UpdateAutoFilerLabel()
{
	if (be_roster->IsRunning(kAutoFilerSignature))
		fStartStop->SetLabel(kStopAutoFiler);
	else
		fStartStop->SetLabel(kStartAutoFiler);
}


void
AutoFilerTab::UpdateAutoFilerFolders()
{
	SaveFolders(fFolderList);

	// if AutoFiler is running, tell it to refresh its folders
	if (be_roster->IsRunning(kAutoFilerSignature)) {
		BMessage msg(MSG_REFRESH_FOLDERS);
		BMessenger msgr(kAutoFilerSignature);
		msgr.SendMessage(&msg);
	}
}


void
AutoFilerTab::ToggleAutorun(bool autorun)
{
	fDirtySettings = true;
	if (autorun)
		EnableAutorun();
	else
		DisableAutorun();
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
