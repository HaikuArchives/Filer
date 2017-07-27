/*
 * Copyright 2017 Haiku, Inc.
 * Distributed under the terms of the MIT License.
 *
 * Author:
 *			Owen Pan <owen.pan@yahoo.com>
 */

#include "PanelButton.h"

#include <Application.h>
#include <IconUtils.h>
#include <Resources.h>
#include <Window.h>


PanelButton::PanelButton(uint32 what, int32 nPanels, float height)
	:
	BButton(NULL, new BMessage(what)),
	fPanelCount(nPanels)
{
	height -= 12;

	fPanels = new BFilePanel*[nPanels];
	fFilters = new TypedRefFilter*[nPanels];

	for (int32 i = 0; i < nPanels; i++) {
		fPanels[i] = NULL;
		fFilters[i] = NULL;
	}

	size_t size;
	const uint8* data = static_cast<const uint8*>(BApplication::AppResources()->
							LoadResource(B_VECTOR_ICON_TYPE, 1, &size));

	fIcon = new BBitmap(BRect(0, 0, height, height), 0, B_RGBA32);
	BIconUtils::GetVectorIcon(data, size, fIcon);

	SetIcon(fIcon);
}


PanelButton::~PanelButton()
{
	for (int32 i = 0; i < fPanelCount; i++) {
		delete fPanels[i];
		delete fFilters[i];
	}

	delete[] fPanels;
	delete[] fFilters;

	delete fIcon;
}


void
PanelButton::CreatePanel(int8 which, const BView* target, uint32 flavors,
	const char* fileType, uint32 filters, const NamePair pairs[])
{
	BFilePanel*& panel = fPanels[which];

	if (panel == NULL) {
		TypedRefFilter*& filter = fFilters[which];
		filter = new TypedRefFilter(fileType, filters);

		BMessenger msgr(target);
		panel = new BFilePanel(B_OPEN_PANEL, &msgr, NULL, flavors, false, NULL,
			filter);

		panel->Window()->SetTitle(pairs[which].locale);
	}
}
