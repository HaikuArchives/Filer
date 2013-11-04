/*
	Database.cpp: Convenience functions for work with the SQLite3 database
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/
#include "CppSQLite3.h"
#include "Database.h"

static const char *sIllegalCharacters[] =
	{ "!","@","#","$","%","^","&","*","(",")","-","+","=","{","}","[","]","\\",
	  "|",";",":","'","\"","<",">",",",".","/","?","`","~"," ", NULL
	};
static const char *sReplacementCharacters[] =
	{ "£21£","£40£","£23£","£24£","£25£","£5e£","£26£","£2a£","£28£","£29£","£2d£",
	  "£2b£","£3d£","£7b£","£7d£","£5b£","£5d£","£5c£","£7c£","£3b£","£3a£","£27£",
	  "£22£","£3c£","£3e£","£2c£","£2e£","£2f£","£3f£","£60£","£7e£","£20£", NULL
	};
	
static const char *sIllegalWords[]=
	{ " select "," drop "," create "," delete "," where "," update "," order "," by ",
		" and "," or "," in "," between "," aliases "," join "," union "," alter ",
		" functions "," group "," into ", " view ", NULL };
static const char *sReplacementWords[]=
	{ " ¥select "," ¥drop "," ¥create "," ¥delete "," ¥where "," ¥update "," ¥order "," ¥by ",
		" ¥and "," ¥or "," ¥in "," ¥between "," ¥aliases "," ¥join "," ¥union "," ¥alter ",
		" ¥functions "," ¥group "," ¥into ", " ¥view ", NULL };

BString EscapeIllegalCharacters(const char *instr)
{
	// Because the £ symbol isn't allowed in a category but is a valid database character,
	// we'll use it as the escape character for illegal characters
	
	BString string(instr);
	if(string.CountChars()<1)
		return string;
	
	string.RemoveAll("£");
	string.RemoveAll("¥");
	
	int32 i=0;
	while(sIllegalCharacters[i])
	{
		string.ReplaceAll(sIllegalCharacters[i],sReplacementCharacters[i]);
		i++;
	}
	
	// Just to make sure that reserved words aren't used, we'll prefix them with the ¥ character
	// for the same reasons that we used £ with bad characters
	i=0;
	while(sIllegalWords[i])
	{
		string.ReplaceAll(sIllegalWords[i],sReplacementWords[i]);
		i++;
	}
	return string;
}

BString DeescapeIllegalCharacters(const char *instr)
{
	BString string(instr);
	if(string.CountChars()<1)
		return string;
	
	int32 i=0;
	while(sIllegalCharacters[i])
	{
		string.ReplaceAll(sReplacementCharacters[i],sIllegalCharacters[i]);
		i++;
	}
	
	// Just to make sure that reserved words aren't used, we'll prefix them with the ¥ character
	// for the same reasons that we used £ with bad characters
	i=0;
	while(sIllegalWords[i])
	{
		string.ReplaceAll(sReplacementWords[i],sIllegalWords[i]);
		i++;
	}
	return string;
}


void DBCommand(CppSQLite3DB &db, const char *command, const char *functionname)
{
	if(!command)
		printf("NULL database command in Database::DBCommand");
	if(!functionname)
		printf("NULL function name in Database::DBCommand");
	
	try
	{
		db.execDML(command);
	}
	catch(CppSQLite3Exception &e)
	{
		BString msg("Database Exception in ");
		msg << functionname << ".\n\n" << e.errorMessage()
			<< "\n\nDatabase Exception Command: " << command << "\n";
		printf(msg.String());
	}
}

CppSQLite3Query DBQuery(CppSQLite3DB &db, const char *query, const char *functionname)
{
	if(!query)
		printf("NULL database command in Database::DBQuery");
	if(!functionname)
		printf("NULL function name in Database::DBQuery");
	
	try
	{
		return db.execQuery(query);
	}
	catch(CppSQLite3Exception &e)
	{
		BString msg("Database Exception in ");
		msg << functionname << ".\n\n" << e.errorMessage()
			<< "\n\nDatabase Exception Query: " << query << "\n";
		printf(msg.String());
	}
	// this will never be reached - just to shut up the compiler
	return CppSQLite3Query();
}
