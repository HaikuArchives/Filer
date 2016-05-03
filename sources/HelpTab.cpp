/*
 * Copyright 2016. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#include <ControlLook.h>
#include <IconUtils.h>
#include <LayoutBuilder.h>
#include <Resources.h>

#include "FilerDefs.h"
#include "HelpTab.h"
#include "main.h"

IconView::IconView()
	:
	BView("iconview", B_WILL_DRAW | B_SUPPORTS_LAYOUT),
	fIcon(NULL)
{
	GetIcon();
}


IconView::~IconView()
{
}


void
IconView::Draw(BRect updateRect)
{
	if (fIcon == NULL)
		return;

	SetDrawingMode(B_OP_ALPHA);
	SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
	DrawBitmapAsync(fIcon, BPoint(0, 0));
}


void
IconView::AttachedToWindow()
{
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	BView::AttachedToWindow();
}


BBitmap*
IconView::Icon()
{
	return fIcon;
}


BSize
IconView::MinSize()
{
	return BSize(64.0, 64.0);
}


BSize
IconView::MaxSize()
{
	return BSize(64.0, 64.0);
}


void
IconView::GetIcon()
{
	BResources* resources = BApplication::AppResources();

	if (resources != NULL) {
		size_t size;
		const uint8* data
			= (const uint8*)resources->LoadResource(B_VECTOR_ICON_TYPE,
		 		"BEOS:ICON", &size);
		fIcon = new BBitmap(BRect(0, 0, 64, 64), 0, B_RGBA32);
		if (fIcon != NULL) {
			if (data == NULL
				|| BIconUtils::GetVectorIcon(data, size, fIcon)
				!= B_OK) {
					delete fIcon;
					fIcon = NULL;
			}
		}
	}
}


HelpTab::HelpTab()
	:
	BView("Help", B_SUPPORTS_LAYOUT)
{
	// Icon
	fIconView = new IconView();

	// About info
	fName = new BStringView("name", "Filer");
	BFont font;
	fName->GetFont(&font);
	font.SetFace(B_BOLD_FACE);
	font.SetSize(font.Size() * 2.0);
	fName->SetFont(&font, B_FONT_FAMILY_AND_STYLE | B_FONT_SIZE
		| B_FONT_FLAGS);
	fName->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));

	fVersion = new BStringView("version", "v1.1.0");
	fVersion->GetFont(&font);
	font.SetFace(B_REGULAR_FACE);
	font.SetSize(font.Size() * 0.9);
	fVersion->SetFont(&font, B_FONT_FAMILY_AND_STYLE | B_FONT_SIZE
		| B_FONT_FLAGS);	
	fVersion->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));
	
	fCopyright1 = new BStringView("copy1", "Copyright 2008, DarkWyrm");
	fCopyright2 = new BStringView("copy2", "Copyright 2016, Humdinger");
	fCopyright1->SetFont(&font, B_FONT_FAMILY_AND_STYLE | B_FONT_SIZE
		| B_FONT_FLAGS);
	fCopyright2->SetFont(&font, B_FONT_FAMILY_AND_STYLE | B_FONT_SIZE
		| B_FONT_FLAGS);
	fCopyright1->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));
	fCopyright2->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));

	fInfo = new BTextView("info");
	fInfo->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	fInfo->MakeEditable(false);
	fInfo->SetWordWrap(true);
	fInfo->SetStylable(true);
	fInfo->SetText("Filer is an automatic file organizer. It takes the "
		"files it's opened with or that are dropped on it and moves, "
		"renames, copies or does all sorts of other things with them "
		"according to rules created by the user.");

	// work-around for misbehaving BTextView: the other GUI elements treat it
	// as it were only one line high.
	float minHeight, maxHeight, prefHeight;
	fInfo->GetHeightForWidth(300.0, &minHeight, &maxHeight, &prefHeight);
	fInfo->SetExplicitMinSize(BSize(30.0, minHeight));
	fInfo->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));
	fInfo->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_TOP));

	// Buttons
	fHelpButton = new BButton("help", "Help on rules",
		new BMessage(MSG_HELP));
	fHelpButton->SetTarget(be_app);

	fDocsButton = new BButton("docs", "User documentation",
		new BMessage(MSG_DOCS));
	fDocsButton->SetTarget(be_app);

	// Laying it all out
	static const float spacing = be_control_look->DefaultItemSpacing();
	BLayoutBuilder::Group<>(this, B_HORIZONTAL)
		.SetInsets(B_USE_WINDOW_INSETS)
		.AddGroup(B_VERTICAL, 0)
			.SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_TOP))
			.SetInsets(0, spacing / 2, 0, 0)
			.Add(fIconView)
		.End()
		.AddGroup(B_VERTICAL, 0)
			.Add(fName)
			.Add(fVersion)
			.Add(fCopyright1)
			.Add(fCopyright2)
			.AddStrut(spacing)
			.Add(fInfo, 0)
			.AddStrut(spacing)
			.AddGroup(B_HORIZONTAL)
				.AddGlue()
				.Add(fHelpButton)
				.Add(fDocsButton)
				.AddGlue()
			.End()
		.AddGlue(100.0)
		.End();

	SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));
}


HelpTab::~HelpTab()
{
}
