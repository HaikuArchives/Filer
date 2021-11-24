/*
	FilerRule.cpp: The main filing class for Filer. It holds a list of both test
					conditions and one or more actions to be performed should the
					conditions be met.
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/

#include "FilerRule.h"

static int64 sIDCounter = 0;

FilerRule::FilerRule()
	:
	fTestList(NULL),
 	fActionList(NULL),
	fMode(FILER_RULE_ALL),
	fDisabled(false)
{
	fTestList = new BObjectList<BMessage>(20, true);
	fActionList = new BObjectList<BMessage>(20, true);

	fID = sIDCounter++;
}


FilerRule::FilerRule(FilerRule& rule)
	:
	fTestList(NULL),
 	fActionList(NULL),
	fMode(FILER_RULE_ALL),
	fDisabled(false)
{
	fTestList = new BObjectList<BMessage>(20, true);
	fActionList = new BObjectList<BMessage>(20, true);

	fID = sIDCounter++;

	*this = rule;
}


FilerRule::FilerRule(BMessage* data)
{
	fTestList = new BObjectList<BMessage>(20, true);
	fActionList = new BObjectList<BMessage>(20, true);

	fDisabled = data->GetBool("_disabled", true);
	data->FindString("_desc", &fDescription);

	int8 mode = 0;
	data->FindInt8("_rulemode", &mode);
	fMode = (filer_rule_mode)mode;

	BMessage test;
	for (int i = 0; data->FindMessage("test", i, &test) == B_OK; i++)
		fTestList->AddItem(new BMessage(test));

	BMessage action;
	for (int i = 0; data->FindMessage("action", i, &action) == B_OK; i++)
		fActionList->AddItem(new BMessage(action));

	fID = sIDCounter++;
}


FilerRule::~FilerRule()
{
	delete fTestList;
	delete fActionList;
}


status_t
FilerRule::Archive(BMessage* into, bool deep)
{
	status_t ret = BArchivable::Archive(into, deep);

	into->AddString("_desc", fDescription);
	into->AddBool("_disabled", fDisabled);
	into->AddInt8("_rulemode", fMode);

	for (int i = 0; i < fTestList->CountItems(); i++)
		into->AddMessage("test", fTestList->ItemAt(i));
	for (int i = 0; i < fActionList->CountItems(); i++)
		into->AddMessage("action", fActionList->ItemAt(i));

	return ret;
}


FilerRule*
FilerRule::Instantiate(BMessage* archive)
{
	if (!validate_instantiation(archive, "FilerRule"))
		return NULL;
	return new FilerRule(archive);
}


void
FilerRule::SetRuleMode(const filer_rule_mode& mode)
{
	fMode = mode;
}


const char*
FilerRule::GetDescription() const
{
	return fDescription.String();
}


void
FilerRule::SetDescription(const char* desc)
{
	fDescription = desc;
}


void
FilerRule::AddTest(BMessage* item, const int32& index)
{
	if (index < 0)
		fTestList->AddItem(item);
	else
		fTestList->AddItem(item, index);
}


BMessage*
FilerRule::RemoveTest(const int32& index)
{
	return fTestList->RemoveItemAt(index);
}


BMessage*
FilerRule::TestAt(const int32& index)
{
	return fTestList->ItemAt(index);
}


int32
FilerRule::CountTests() const
{
	return fTestList->CountItems();
}


void
FilerRule::AddAction(BMessage* item, const int32& index)
{
	if (index < 0)
		fActionList->AddItem(item);
	else
		fActionList->AddItem(item, index);
}


BMessage*
FilerRule::RemoveAction(const int32& index)
{
	return fActionList->RemoveItemAt(index);
}


BMessage*
FilerRule::ActionAt(const int32& index)
{
	return fActionList->ItemAt(index);
}


int32
FilerRule::CountActions() const
{
	return fActionList->CountItems();
}


void
FilerRule::MakeEmpty()
{
	fTestList->MakeEmpty();
	fActionList->MakeEmpty();
}


void
FilerRule::PrintToStream()
{
	printf("Filer Rule '%s':\n", GetDescription());
	
	for (int32 i = 0; i < fTestList->CountItems(); i++)
		fTestList->ItemAt(i)->PrintToStream();

	for (int32 i = 0; i < fActionList->CountItems(); i++)
		fActionList->ItemAt(i)->PrintToStream();
}


FilerRule&
FilerRule::operator=(FilerRule& from)
{
	MakeEmpty();
	for (int32 i = 0; i < from.CountTests(); i++)
	{
		BMessage* test = from.TestAt(i);
		if (test)
			AddTest(new BMessage(*test));
	}

	for (int32 i = 0; i < from.CountActions(); i++)
	{
		BMessage* action = from.ActionAt(i);
		if (action)
			AddAction(new BMessage(*action));
	}

	SetDescription(from.GetDescription());
	SetRuleMode(from.GetRuleMode());

	return *this;
}

