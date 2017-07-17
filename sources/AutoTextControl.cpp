/*
	AutoTextControl.cpp: A BTextControl which notifies on each keypress
	Written by DarkWyrm <darkwyrm@earthlink.net>, Copyright 2007
	Released under the MIT license.
	Contributed by:
		Pete Goodeve
*/

#include "AutoTextControl.h"

#include <Entry.h>
#include <Path.h>
#include <PropertyInfo.h>
#include <String.h>
#include <Window.h>

#include <stdio.h>
#include <ctype.h>

#include "RuleEditWindow.h"
#include "RuleRunner.h"

#define MSG_TEXT_CHANGED 'txtc'

static property_info sProperties[] = {
	{ "CharacterLimit", { B_GET_PROPERTY, 0 }, { B_DIRECT_SPECIFIER, 0 },
		"Returns the maximum number of characters "
		"that the AutoTextControl will allow.",
		0, { B_INT32_TYPE }
	},
	
	{ "CharacterLimit", { B_SET_PROPERTY, 0 }, { B_DIRECT_SPECIFIER, 0},
		"Sets the maximum number of characters "
		"that the AutoTextControl will allow.",
		0, { B_INT32_TYPE }
	},
};

AutoTextControl::AutoTextControl(const char* name, const char* label,
	const char* text, BMessage* msg, uint32 flags)
	:
	BTextControl(name, label, text, msg, flags),
	fEmpty(false),
 	fFilter(NULL),
 	fCharLimit(0)
{
	SetFilter(new AutoTextControlFilter(this));
}


AutoTextControl::~AutoTextControl()
{
	if (Window())
		Window()->RemoveCommonFilter(fFilter);

	delete fFilter;
}


AutoTextControl::AutoTextControl(BMessage* data)
	:
	BTextControl(data)
{
	if (data->FindInt32("_charlimit", (int32*)&fCharLimit) != B_OK)
		fCharLimit = 0;
}


BArchivable*
AutoTextControl::Instantiate(BMessage* data)
{
	if (validate_instantiation(data, "AutoTextControl"))
		return new AutoTextControl(data);

	return NULL;
}


status_t
AutoTextControl::Archive(BMessage* data, bool deep) const
{
	status_t status = BTextControl::Archive(data, deep);

	if (status == B_OK)
		status = data->AddInt32("_charlimit", fCharLimit);

	if (status == B_OK)
		status = data->AddString("class", "AutoTextControl");

	return status;
}


status_t
AutoTextControl::GetSupportedSuites(BMessage* msg)
{
	msg->AddString("suites", "suite/vnd.DW-autotextcontrol");

	BPropertyInfo prop_info(sProperties);
	msg->AddFlat("messages", &prop_info);
	return BTextControl::GetSupportedSuites(msg);
}


BHandler*
AutoTextControl::ResolveSpecifier(BMessage* msg, int32 index,
	BMessage* specifier, int32 form, const char* property)
{
	return BControl::ResolveSpecifier(msg, index, specifier, form, property);
}


void
AutoTextControl::AttachedToWindow()
{
	BTextControl::AttachedToWindow();
	if (fFilter)
		Window()->AddCommonFilter(fFilter);

	if (IsEmptyAfterTrim(Text())) {
		fEmpty = true;
		MarkAsInvalid(true);
		static_cast<RuleEditWindow*>(Window())->UpdateEmptyCount(true);
	}

	SetModificationMessage(new BMessage(MSG_TEXT_CHANGED));
	SetTarget(this);
}


void
AutoTextControl::DetachedFromWindow()
{
	if (fFilter)
		Window()->RemoveCommonFilter(fFilter);

	if (IsEmptyAfterTrim(Text()))
		static_cast<RuleEditWindow*>(Window())->UpdateEmptyCount(false);

	BTextControl::DetachedFromWindow();
}


void
AutoTextControl::MessageReceived(BMessage* msg)
{
	if (msg->WasDropped()) {
		entry_ref ref;
		if (msg->FindRef("refs", &ref) != B_OK)
			return;

		int8 type;
		TestView* testView = dynamic_cast<TestView*>(Parent());
		bool isTest = testView != NULL;
		if (isTest)
			type = testView->GetType();
		else {
			ActionView* view = dynamic_cast<ActionView*>(Parent());
			if (view == NULL)
				return;

			type = view->GetType();
		}

		BString text;
		if (SetTextForType(text, type, ref, isTest)) {
			SetText(text.String());

			if (isTest && testView->GetDataType() == TEST_TYPE_NUMBER)
				testView->ResetUnit();
		}
	} else if (msg->what == MSG_TEXT_CHANGED) {
		bool empty = IsEmptyAfterTrim(Text());
		if (fEmpty != empty) {
			fEmpty = empty;
			MarkAsInvalid(empty);
			static_cast<RuleEditWindow*>(Window())->UpdateEmptyCount(empty);
		}
	} else
		BTextControl::MessageReceived(msg);
}


void
AutoTextControl::SetCharacterLimit(const uint32& limit)
{
	fCharLimit = limit;
}


uint32
AutoTextControl::GetCharacterLimit(const uint32& limit)
{
	return fCharLimit;
}


void
AutoTextControl::SetFilter(AutoTextControlFilter* filter)
{
	if (fFilter) {
		if (Window())
			Window()->RemoveCommonFilter(fFilter);
		delete fFilter;
	}

	fFilter = filter;
	if (Window())
		Window()->AddCommonFilter(fFilter);
}


AutoTextControlFilter::AutoTextControlFilter(AutoTextControl* box)
	:
	BMessageFilter(B_PROGRAMMED_DELIVERY, B_ANY_SOURCE, B_KEY_DOWN),
 	fBox(box),
 	fCurrentMessage(NULL)
{
}


AutoTextControlFilter::~AutoTextControlFilter()
{
}


filter_result
AutoTextControlFilter::Filter(BMessage* msg, BHandler** target)
{
	int32 rawchar, mod;
	msg->FindInt32("raw_char", &rawchar);
	msg->FindInt32("modifiers", &mod);

	BView* view = dynamic_cast<BView*>(*target);
	if (view != NULL || view->Name() != NULL && strcmp("_input_", view->Name()))
		return B_DISPATCH_MESSAGE;
	
	AutoTextControl* text = dynamic_cast<AutoTextControl*>(view->Parent());
	if (!text || text != fBox)
		return B_DISPATCH_MESSAGE;

	fCurrentMessage = msg;
	filter_result result = KeyFilter(rawchar, mod);
	fCurrentMessage = NULL;

	if (fBox->fCharLimit && result == B_DISPATCH_MESSAGE) {
		// See to it that we still allow shortcut keys
		if (mod & B_COMMAND_KEY)
			return B_DISPATCH_MESSAGE;

		// We don't use strlen() because it is not UTF-8 aware, which can
		// affect how many characters can be typed.
		if (isprint(rawchar) && 
				(uint32)BString(text->Text()).CountChars() == text->fCharLimit)
			return B_SKIP_MESSAGE;
	}

	return result;
}


filter_result
AutoTextControlFilter::KeyFilter(const int32& rawchar, const int32& mod)
{
	if (fBox)
		fBox->Invoke();

	return B_DISPATCH_MESSAGE;
}


bool
IsEmptyAfterTrim(const char* s)
{
	BString str(s);

	return str.Trim().IsEmpty();
}
