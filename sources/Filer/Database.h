/*
	Database.h: Convenience functions for work with the SQLite3 database
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/
#ifndef DATABASE_H
#define DATABASE_H

#include <String.h>

BString 		EscapeIllegalCharacters(const char *string);
BString 		DeescapeIllegalCharacters(const char *string);

void 			DBCommand(CppSQLite3DB &db, const char *command, 
							const char *functionname);
CppSQLite3Query	DBQuery(CppSQLite3DB &db, const char *query, 
							const char *functionname);

#endif
