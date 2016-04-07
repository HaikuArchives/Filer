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

enum
{
	TEST_TYPE_NULL = 0,
	TEST_TYPE_STRING,
	TEST_TYPE_NUMBER,
	TEST_TYPE_DATE,
	TEST_TYPE_ANY
};


class RuleRunner
{
public:
						RuleRunner(void);
						~RuleRunner(void);
	
	static	void		GetTestTypes(BMessage &msg);
	static	status_t	GetCompatibleModes(const char *testtype, BMessage &msg);
	static	status_t	GetCompatibleModes(const int32 &type, BMessage &msg);
	static	void		GetModes(BMessage &msg);
	static	void		GetActions(BMessage &msg);
	
	static	BString		GetEditorTypeForTest(const char *testname);
	
	static	int32		GetDataTypeForTest(const char *testname);
	static	int32		GetDataTypeForMode(const char *modename);
	
			bool		IsMatch(const BMessage &test, const entry_ref &ref);
			status_t	RunAction(const BMessage &test, entry_ref &ref);
			status_t	RunRule(FilerRule *rule, entry_ref &ref);
};

status_t LoadRules(BObjectList<FilerRule> *ruleList);
status_t SaveRules(BObjectList<FilerRule> *ruleList);


// Some convenience functions. Deleting the returned BMessage is the
// responsibility of the caller
BMessage * MakeTest(const char *name,const char *mode, const char *value,
					const char *mimeType = NULL, const char *typeName = NULL,
					const char *attrType = NULL, const char *attrName = NULL);
BMessage * MakeAction(const char *name,const char *value);

#endif
