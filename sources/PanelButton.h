/*
 * Copyright 2017 Haiku, Inc.
 * Distributed under the terms of the MIT License.
 *
 * Author:
 *			Owen Pan <owen.pan@yahoo.com>
 */

#ifndef PANEL_BUTTON_H
#define PANEL_BUTTON_H

#include <Bitmap.h>
#include <Button.h>
#include <FilePanel.h>

#include "RuleRunner.h"
#include "TypedRefFilter.h"

class PanelButton : public BButton
{
	int32				fPanelCount;
	BBitmap*			fIcon;

	BFilePanel**		fPanels;
	TypedRefFilter**	fFilters;

public:
			PanelButton(uint32 what, int32 nPanels, float height);
			~PanelButton();

	void	CreatePanel(int8 which, const BView* target, uint32 flavors,
				const char* fileType, uint32 filters, const NamePair pairs[]);
	void	ShowPanel(int8 which) { fPanels[which]->Show(); }
	bool	PanelExists(int8 which) { return fPanels[which] != NULL; }
};

#endif	// PANEL_BUTTON_H
