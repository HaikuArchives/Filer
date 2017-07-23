/*
	ActionView.cpp: View for adjusting settings for an individual Filer action
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
	Contributed by:
		Pete Goodeve, 2016
		Owen Pan <owen.pan@yahoo.com>, 2017
*/

#include "ActionView.h"

#include <Application.h>
#include <IconUtils.h>
#include <LayoutBuilder.h>
#include <Path.h>
#include <Resources.h>

#include "FilerDefs.h"
#include "RuleEditWindow.h"
#include "RuleRunner.h"


ActionView::ActionView(const char* name, BMessage* action, const int32& flags)
	:
	BView(name, flags | B_FRAME_EVENTS)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	RuleRunner::GetActions(fActions);

	fFilePanels = new BFilePanel*[nActions];
	fRefFilters = new TypedRefFilter*[nActions];
	for (int32 i = 0; i < nActions; i++) {
		fFilePanels[i] = NULL;
		fRefFilters[i] = NULL;
	}

	fActionField = new BMenuField(NULL, ActionMenu());
	fIconButton = new BButton(NULL, new BMessage(MSG_ACTION_PANEL));

	fValueBox = new AutoTextControl("valuebox", NULL, NULL, new BMessage());
	fValueBox->SetDivider(0);

	size_t size;
	const uint8* data = static_cast<const uint8*>(BApplication::AppResources()->
							LoadResource(B_VECTOR_ICON_TYPE, 1, &size));

	const float boxHeight = fValueBox->Bounds().Height();
	const float height = boxHeight - 12;
	fIcon = new BBitmap(BRect(0, 0, height, height), 0, B_RGBA32);

	BIconUtils::GetVectorIcon(data, size, fIcon);
	fIconButton->SetIcon(fIcon);

	fAddRemoveButtons = new AddRemoveButtons(MSG_ADD_ACTION, MSG_REMOVE_ACTION,
		this, boxHeight);

	BLayoutBuilder::Group<>(this, B_HORIZONTAL, B_USE_HALF_ITEM_SPACING)
		.Add(fActionField, 0)
		.AddGroup(B_HORIZONTAL, 0)
			.Add(fIconButton, 0)
			.Add(fValueBox)
			.End()
		.AddStrut(0)
		.Add(fAddRemoveButtons)
		.End();

	fValueBox->SetText("");

	if (action != NULL && action->FindInt8("type", &fType) == B_OK) {
		BString str;
		if (action->FindString("value", &str) == B_OK)
			fValueBox->SetText(str.String());
	} else if (fActions.FindInt8("actions", 0, &fType) != B_OK)
		fType = 0;

	SetAction();

	BString toolTip(
		"\%FILENAME\%\t\tFull file name\n"
		"\%EXTENSION\%\tJust the extension\n"
		"\%BASENAME\%\tFile name without extension\n"
		"\%FOLDER\%\t\tFull location of the folder which contains the file\n"
		"\%FULLPATH\%\t\tFull location of the file\n"
		"\%DATE\%\t\t\tCurrent date in the format MM-DD-YYYY\n"
		"\%EURODATE\%\t\tCurrent date in the format DD-MM-YYYY\n"
		"\%REVERSEDATE\%\tCurrent date in the format YYYY-MM-DD\n"
		"\%TIME\%\t\t\tCurrent time using 24-hour time\n"
		"\%ATTR:xxxx\%\t\tAn extended attribute of the file");
	fValueBox->SetToolTip(toolTip.String());
}


ActionView::~ActionView()
{
	for (int32 i = 0; i < nActions; i++) {
		delete fFilePanels[i];
		delete fRefFilters[i];
	}
	delete[] fFilePanels;
	delete[] fRefFilters;

	delete fActionField;
	delete fIcon;
	delete fIconButton;
	delete fValueBox;
	delete fAddRemoveButtons;
}


void
ActionView::AttachedToWindow()
{
	fActionField->Menu()->SetTargetForItems(this);
	fIconButton->SetTarget(this);
	fValueBox->SetTarget(this);
}


void
ActionView::MessageReceived(BMessage* msg)
{
	switch (msg->what)
	{
		case B_REFS_RECEIVED:
		{
			entry_ref ref;
			if (msg->FindRef("refs", &ref) == B_OK)
				fValueBox->SetText(fType == ACTION_RENAME ? ref.name
					: BPath(&ref).Path());
			break;
		}
		case MSG_ACTION_CHOSEN:
		{
			int8 type;
			if (msg->FindInt8("type", &type) == B_OK) {
				fType = type;
				bool wasHidden = fValueBox->IsHidden();
				SetAction();
				bool isHidden = fValueBox->IsHidden();
				if (wasHidden != isHidden
					&& IsEmptyAfterTrim(fValueBox->Text()))
					static_cast<RuleEditWindow*>(Window())->
						UpdateEmptyCount(wasHidden && !isHidden);
			}
			break;
		}
		case MSG_ACTION_PANEL:
		{
			BFilePanel*& panel = fFilePanels[fType];
			if (panel == NULL) {
				char* filetype;
				switch (fType) {
					case ACTION_ARCHIVE:
						filetype = "application/zip";
						break;
					case ACTION_COMMAND:
						filetype = "text/plain";
						break;
					default:
						filetype = "";
				}

				uint32 flags = B_DIRECTORY_NODE;
				if (fType == ACTION_RENAME || fType == ACTION_ARCHIVE
					|| fType == ACTION_COMMAND)
					flags |= B_FILE_NODE;

				TypedRefFilter*& filter = fRefFilters[fType];
				filter = new TypedRefFilter(filetype, flags);

				if (fType == ACTION_RENAME || fType == ACTION_COMMAND)
					flags = B_FILE_NODE;

				BMessenger msgr(this);
				panel = new BFilePanel(B_OPEN_PANEL, &msgr, NULL, flags, false,
					NULL, filter);
				panel->Window()->SetTitle(sActions[fType].locale);
			}

			panel->Show();
			break;
		}
		default:
			BView::MessageReceived(msg);
	}
}


BMessage*
ActionView::GetAction() const
{
	BMessage* action = new BMessage;
	BString str(fValueBox->Text());

	action->AddInt8("type", fType);
	action->AddString("value", str.Trim());

	return action;
}


static void
setVisibility(BControl* control, bool show)
{
	if (show) {
		if (control->IsHidden())
			control->Show();
	} else if (!control->IsHidden())
			control->Hide();
}


void
ActionView::SetAction()
{
	fActionField->MenuItem()->SetLabel(sActions[fType].locale);

	bool show = ActionHasTarget(fType);
	setVisibility(fValueBox, show);
	setVisibility(fIconButton, show);
}


BPopUpMenu*
ActionView::ActionMenu() const
{
	BPopUpMenu* menu = new BPopUpMenu("");

	int8 type;
	for (int32 i = 0; fActions.FindInt8("actions", i, &type) == B_OK; i++) {
		BMessage* msg = new BMessage(MSG_ACTION_CHOSEN);
		msg->AddInt8("type", type);
		menu->AddItem(new BMenuItem(sActions[type].locale, msg));
	}

	return menu;
}
