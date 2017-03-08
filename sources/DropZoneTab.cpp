/*
 * Copyright 2016. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#include <ControlLook.h>
#include <Dragger.h>
#include <Font.h>
#include <LayoutBuilder.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <Roster.h>
#include <View.h>
#include <private/interface/AboutWindow.h>

#include "DropZoneTab.h"
#include "FilerDefs.h"
#include "main.h"
#include "ReplicantWindow.h"

void
DropZone::_Init()
{
	fLabel1 = new BStringView("label1", " Filer ");
	fLabel2 = new BStringView("label2", " Dropzone ");

	BFont font;
	fLabel1->GetFont(&font);
	font.SetFace(B_CONDENSED_FACE);
	font.SetSize(font.Size() * 1.5);
	fLabel1->SetFont(&font, B_FONT_FAMILY_AND_STYLE | B_FONT_SIZE
		| B_FONT_FLAGS);
	font.SetSize(font.Size() * 0.75);
	fLabel2->SetFont(&font, B_FONT_FAMILY_AND_STYLE | B_FONT_SIZE
		| B_FONT_FLAGS);

	fLabel1->SetAlignment(B_ALIGN_CENTER);
	fLabel2->SetAlignment(B_ALIGN_CENTER);

	return;
}


DropZone::DropZone(bool replicatable)
	:
	BView("Filer dropzone", B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE),
	fReplicated(false)
{
	SetViewColor(B_TRANSPARENT_COLOR);

	_Init();

	if (replicatable) {
		// Dragger
		BRect rect(Bounds());
		rect.left = rect.right - 7;
		rect.top = rect.bottom - 7;
		BDragger* dragger = new BDragger(rect, this,
			B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
		dragger->SetExplicitMinSize(BSize(7, 7));
		dragger->SetExplicitMaxSize(BSize(7, 7));
		dragger->SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT, B_ALIGN_BOTTOM));

		// Layout
		BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
			.AddGroup(B_VERTICAL, 3)
				.AddGlue()
				.AddStrut(1)
				.Add(fLabel1)
				.Add(fLabel2)
				.AddGlue()
			.End()
			.Add(dragger, 0.01);
	} else {
		BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
			.AddGroup(B_VERTICAL, 3)
				.AddGlue()
				.AddStrut(1)
				.Add(fLabel1)
				.Add(fLabel2)
				.AddGlue()
			.End();
	}
}


DropZone::DropZone(BMessage* archive)
	:
	BView(archive),
	fReplicated(true)
{
	_Init();
}


DropZone::~DropZone()
{
}


BArchivable*
DropZone::Instantiate(BMessage* data)
{
	if (!validate_instantiation(data, "Filer"))
		return NULL;

	return new DropZone(data);
}


status_t
DropZone::Archive(BMessage* archive, bool deep) const
{
	BView::Archive(archive, deep);

	archive->AddString("add_on", kFilerSignature);
	archive->AddString("class", "Filer");
		
	archive->PrintToStream();

	return B_OK;
}


void
DropZone::Draw(BRect rect)
{
	BRect bounds = Bounds();
	if (!fReplicated) {
		SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
		FillRect(bounds);
	}

	SetDrawingMode(B_OP_ALPHA);
	SetLowColor(0, 0, 0, 0);
	SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),
		B_DARKEN_2_TINT));

	FillRect(bounds, B_SOLID_LOW);
	StrokeRect(bounds);
	FillRect(bounds.InsetBySelf(3, 3), stripePattern);

	BView::Draw(rect);
}


void
DropZone::MessageReceived(BMessage* msg)
{
	if (msg->WasDropped()) {
		msg->what = B_REFS_RECEIVED;
		be_roster->Launch(kFilerSignature, msg);
		return;
	}
	switch (msg->what)
	{
		case MSG_OPEN_FILER:
		{
			be_roster->Launch(kFilerSignature);
			break;
		}
		case B_ABOUT_REQUESTED:
		{
			BAboutWindow* about = new BAboutWindow("Filer", kFilerSignature);
			about->AddDescription(
				"Filer is an automatic file organizer. It takes the "
				"files it's opened with or that are dropped on it and moves, "
				"renames, copies or does all sorts of other things with them "
				"according to rules created by the user.");
			about->AddCopyright(2008, "DarkWyrm");
			about->AddCopyright(2016, "Humdinger, Pete Goodeve");
			about->AddCopyright(2017, "Owen Pan");
			about->Show();
		}
		default:
		{
			BView::MessageReceived(msg);
			break;
		}
	}
}


void
DropZone::MouseDown(BPoint position)
{
	uint32 buttons = 0;
	if (Window() != NULL && Window()->CurrentMessage() != NULL)
		buttons = Window()->CurrentMessage()->FindInt32("buttons");

	if (buttons == B_SECONDARY_MOUSE_BUTTON)
		ShowPopUpMenu(ConvertToScreen(position));

	BView::MouseDown(position);
}


void
DropZone::ShowPopUpMenu(BPoint screen)
{
	if (!fReplicated)
		return;

	BPopUpMenu* menu = new BPopUpMenu("PopUpMenu", this);

	BMenuItem* item = new BMenuItem("Open Filer" B_UTF8_ELLIPSIS,
		new BMessage(MSG_OPEN_FILER));
	menu->AddItem(item);

	menu->SetTargetForItems(this);
	menu->Go(screen, true, true, true);
}


DropZoneTab::DropZoneTab()
	:
	BView("Dropzone", B_SUPPORTS_LAYOUT)
{
	BStringView* zoneLabel = new BStringView("zonelabel",
		"Drag and drop the files to be processed below.");
	zoneLabel->SetAlignment(B_ALIGN_CENTER);
	fDropzone = new DropZone(false);

	fRepliButton = new BButton("replibutton",
		"Replicate dropzone" B_UTF8_ELLIPSIS, new BMessage(MSG_REPLICATE));

	static const float spacing = be_control_look->DefaultItemSpacing();
	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(spacing)
		.Add(zoneLabel)
		.Add(fDropzone)
		.Add(fRepliButton);
}


DropZoneTab::~DropZoneTab()
{
}


void
DropZoneTab::AttachedToWindow()
{
	fRepliButton->SetTarget(this);

	BView::AttachedToWindow();
}


void
DropZoneTab::MessageReceived(BMessage* msg)
{
	switch (msg->what)
	{
		case MSG_REPLICATE:
		{
			ReplicantWindow* replicantWindow
				= new ReplicantWindow(Window()->Frame());
			replicantWindow->Show();
			break;
		}
		default:
		{
			BView::MessageReceived(msg);
			break;
		}
	}
}
