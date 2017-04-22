#ifndef REFSTORAGE_H
#define REFSTORAGE_H

#include <Entry.h>
#include <ListView.h>
#include <String.h>

extern BList gRefStructList;
extern BLocker gRefLock;
extern const char gPrefsPath[];


class RefStorage
{
public:
				RefStorage(const entry_ref& ref);
	void		SetData(const entry_ref& ref);

	entry_ref	ref;
	node_ref	nref;
	bool		doAll;
	bool		replace;
};

status_t LoadFolders(BListView* folderList = NULL);
status_t ReloadFolders();
status_t SaveFolders(const BListView* folderList = NULL);

#endif	// REFSTORAGE_H
