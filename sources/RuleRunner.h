/*
	RuleRunner.h: class to handle running test and actions for the rules
	Released under the MIT license.
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Contributed by: Humdinger <humdingerb@gmail.com>, 2016
*/

#ifndef RULERUNNER_H
#define RULERUNNER_H

#include "FilerRule.h"
#include "ObjectList.h"

#include <Entry.h>
#include <Message.h>

struct NamePair
{
	const char* const english;
	const char* const locale;
};

extern const NamePair sTestTypes[];
extern const NamePair sModeTypes[];
extern const NamePair sActions[];

enum
{
	TEST_TYPE_NULL = 0,
	TEST_TYPE_STRING,
	TEST_TYPE_NUMBER,
	TEST_TYPE_DATE,
	TEST_TYPE_ANY
};

// Pos-non-zero status value to specify checking more rules after match
#define CONTINUE_TESTS 1

class RuleRunner
{
public:
						RuleRunner();
						~RuleRunner();

	static	void		GetTestTypes(BMessage& msg);
	static	status_t	GetCompatibleModes(int8 testtype, BMessage& msg);
	static	status_t	GetCompatibleModes(const int32& type, BMessage& msg);
	static	void		GetModes(BMessage& msg);
	static	void		GetActions(BMessage& msg);

//	static	BString		GetEditorTypeForTest(const char* testname);

	static	int32		GetDataTypeForTest(int8 testtype);
	static	int32		GetDataTypeForMode(int8 modetype);

			bool		IsMatch(const BMessage& test, const entry_ref& ref);
			status_t	RunAction(const BMessage& test, entry_ref& ref);
			status_t	RunRule(FilerRule* rule, entry_ref& ref);
};

status_t	LoadRules(BObjectList<FilerRule>* ruleList);
status_t	SaveRules(const BObjectList<FilerRule>* ruleList);
int8		AttributeTestType();
bool		ActionHasTarget(int8 type);
bool		SetTextForType(BString& text, int8 type, const entry_ref& ref,
				bool isTest);
void		AddDefaultRules(BObjectList<FilerRule>* ruleList);

#endif	// RULERUNNER_H
