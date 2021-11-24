/*
	FilerRule.h: The main filing class for Filer. It holds a list of both test
					conditions and one or more actions to be performed should the
					conditions be met.
	Released under the MIT license.
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Contributed by Owen Pan <owen.pan@yahoo.com>, 2017
*/

#ifndef FILER_RULE_H
#define FILER_RULE_H

#include <Archivable.h>
#include <Entry.h>
#include <Message.h>
#include <String.h>

#include "ObjectList.h"

typedef enum
{
	FILER_RULE_ALL = 0,
	FILER_RULE_ANY
} filer_rule_mode;


class FilerRule : public BArchivable
{
public:
								FilerRule();
								FilerRule(FilerRule& rule);
								FilerRule(BMessage* data);
								~FilerRule();

	virtual status_t			Archive(BMessage* into, bool deep = true);
	static FilerRule*	Instantiate(BMessage* archive);

			void				SetRuleMode(const filer_rule_mode& mode);
			filer_rule_mode		GetRuleMode() const { return fMode; }

			const char*			GetDescription() const;
			void				SetDescription(const char* desc);

			void				AddTest(BMessage* item, const int32& index = -1);
			BMessage*			RemoveTest(const int32& index);
			BMessage*			TestAt(const int32& index);
			int32				CountTests() const;

			void				AddAction(BMessage* action, const int32& index = -1);
			BMessage*			RemoveAction(const int32& index);
			BMessage*			ActionAt(const int32& index);
			int32				CountActions() const;

			void				MakeEmpty();
	virtual	void				PrintToStream();
			FilerRule&			operator=(FilerRule& from);
			
			int64				GetID() const { return fID; }
			bool				Disabled() const { return fDisabled; }
			void				Disabled(bool disabled) { fDisabled = disabled; }
			void				Toggle() { fDisabled = !fDisabled; }

private:
	BObjectList<BMessage>*		fTestList;
	BObjectList<BMessage>*		fActionList;
	filer_rule_mode				fMode;
	BString						fDescription;
	int64						fID;
	bool						fDisabled;
};

#endif	// FILER_RULE_H
