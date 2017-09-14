/*
	ActionView.cpp: View for adjusting settings for an individual Filer action
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
	Contributed by:
		Pete Goodeve, 2016
		Owen Pan <owen.pan@yahoo.com>, 2017
*/

#include "ActionView.h"

#include <Catalog.h>
#include <LayoutBuilder.h>
#include <Path.h>

#include "FilerDefs.h"
#include "RuleEditWindow.h"
#include "RuleRunner.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ActionView"


ActionView::ActionView(const char* name, BMessage* action, const int32& flags)
	:
	BView(name, flags | B_FRAME_EVENTS)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	RuleRunner::GetActions(fActions);

	fActionField = new BMenuField(NULL, ActionMenu());

	fValueBox = new AutoTextControl("valuebox", NULL, NULL, new BMessage());
	fValueBox->SetDivider(0);

	const float height = fValueBox->Bounds().Height();
	fPanelButton = new PanelButton(MSG_ACTION_PANEL, nActions, height);
	fAddRemoveButtons = new AddRemoveButtons(MSG_ADD_ACTION, MSG_REMOVE_ACTION,
		this, height);

	BLayoutBuilder::Group<>(this, B_HORIZONTAL, B_USE_HALF_ITEM_SPACING)
		.Add(fActionField, 0)
		.AddGroup(B_HORIZONTAL, 0)
			.Add(fPanelButton, 0)
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

	BString toolTip(B_TRANSLATE_COMMENT(
		"\%FILENAME\%\t\tFull file name\n"
		"\%EXTENSION\%\tJust the extension\n"
		"\%BASENAME\%\tFile name without extension\n"
		"\%FOLDER\%\t\tFull location of the folder which contains the file\n"
		"\%FULLPATH\%\t\tFull location of the file\n"
		"\%DATE\%\t\t\tCurrent date in the format MM-DD-YYYY\n"
		"\%EURODATE\%\t\tCurrent date in the format DD-MM-YYYY\n"
		"\%REVERSEDATE\%\tCurrent date in the format YYYY-MM-DD\n"
		"\%TIME\%\t\t\tCurrent time using 24-hour time\n"
		"\%ATTR:xxxx\%\t\tAn extended attribute of the file",
		"Tooltip, do not translate the %variables%"));
	fValueBox->SetToolTip(toolTip.String());
}


ActionView::~ActionView()
{
	delete fActionField;
	delete fPanelButton;
	delete fValueBox;
	delete fAddRemoveButtons;
}


void
ActionView::AttachedToWindow()
{
	fActionField->Menu()->SetTargetForItems(this);
	fPanelButton->SetTarget(this);
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
			if (msg->FindRef("refs", &ref) == B_OK) {
				BString text;
				if (SetTextForType(text, fType, ref, false))
					fValueBox->SetText(text);
			}
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
			if (!fPanelButton->PanelExists(fType)) {
				const char* fileType;
				switch (fType) {
					case ACTION_ARCHIVE:
						fileType = kArchiveMime;
						break;
					case ACTION_COMMAND:
						fileType = kScriptMime;
						break;
					default:
						fileType = "";
				}

				uint32 flavor = B_FILE_NODE;
				if (fType != ACTION_COMMAND)
					flavor |= B_DIRECTORY_NODE;

				uint32 filter = B_DIRECTORY_NODE;
				if (fType != ACTION_MOVE && fType != ACTION_COPY)
					filter |= B_FILE_NODE | B_SYMLINK_NODE;

				fPanelButton->CreatePanel(fType, this, flavor, fileType, filter,
					sActions[fType].locale);
			}

			fPanelButton->ShowPanel(fType);
			break;
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


void
ActionView::SetVisibility(BView* view, bool show)
{
	if (show) {
		if (view->IsHidden())
			view->Show();
	} else if (!view->IsHidden())
		view->Hide();
}


void
ActionView::SetAction()
{
	fActionField->MenuItem()->SetLabel(sActions[fType].locale);

	bool show = ActionHasTarget(fType);
	SetVisibility(fValueBox, show);
	SetVisibility(fPanelButton, show);
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
