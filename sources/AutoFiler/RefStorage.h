#ifndef REFSTORAGE_H
#define REFSTORAGE_H

#include <Entry.h>
#include <Node.h>
#include <List.h>
#include <Locker.h>
#include <String.h>

extern BList gRefStructList;
extern BLocker gRefLock;
extern const char gPrefsPath[];

enum
{
	M_REFRESH_FOLDERS = 'rffl'
};

class RefStorage
{
public:
				RefStorage(const entry_ref &ref);
	void		SetData(const entry_ref &ref);
	
	entry_ref	ref;
	node_ref	nref;
};

status_t LoadFolders(void);
status_t ReloadFolders(void);
status_t SaveFolders(void);

#endif
