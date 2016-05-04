#ifndef REFSTORAGE_H
#define REFSTORAGE_H

#include <Entry.h>
#include <List.h>
#include <Locker.h>
#include <Node.h>
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
};

status_t LoadFolders();
status_t ReloadFolders();
status_t SaveFolders();

#endif	// REFSTORAGE_H
