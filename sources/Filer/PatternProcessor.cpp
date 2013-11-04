/*
	PatternProcessor.cpp: Code to process substitution patterns
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/
#include "PatternProcessor.h"
#include <time.h>
#include <stdio.h>
#include <Node.h>
#include <Path.h>
#include <TypeConstants.h>
#include <fs_attr.h>

/*
	Patterns in Filer
	
	Regular expressions will probably be supported later, but for now there
	is a way to substitute attributes of the file being processed into the
	value string of a particular action before it is executed by the RuleRunner.
	
	Each attribute is places between two percent signs - %BASENAME%, for
	example.
	
	%FILENAME%	- full name of the file
	%EXTENSION%	- just the extension of the file
	%BASENAME% -  file name without extension
	%FILEPATH% - full name and location of the file
	%FOLDER% - the location of the folder containing the file
	
	%DATE% - The current date in the format MM-DD-YYYY
	%EURODATE% - The current date in the format DD-MM-YYYY
	%REVERSEDATE% - The current date in the format YYYY-MM-DD
	%TIME% - The current time
	%ATTR:xxxx% - An attribute of the file. Note that this needs to be
				the internal name of the attributes, such as META:email
	
	
*/

BString
ProcessPatterns(const char *instr, const entry_ref &ref)
{
	if (!instr)
		return BString();
	
	BString outstr(instr);
	
	
	// Handle filename-based patterns
	
	BString basename(ref.name), extension;
	
	int32 strpos = basename.FindFirst(".");
	if (strpos >= 0)
	{
		extension = basename.String() + strpos;
		basename.Truncate(strpos);
	}
	
	outstr.ReplaceAll("%FILENAME%",ref.name);
	outstr.ReplaceAll("%BASENAME%",basename.String());
	outstr.ReplaceAll("%EXTENSION%",extension.String());
	
	BString pathstr = BPath(&ref).Path();
	outstr.ReplaceAll("%FULLPATH%",pathstr.String());
	
	pathstr.ReplaceLast(BPath(&ref).Leaf(),"");
	outstr.ReplaceAll("%FOLDER%",pathstr.String());
	
	// Date-based patterns
	time_t currenttime = time(NULL);
	struct tm timedata = *localtime(&currenttime);
	
	char timestr[64];
	if (outstr.FindFirst("%DATE%") >= 0)
	{
		sprintf(timestr,"%.2d-%.2d-%d",timedata.tm_mday,timedata.tm_mon + 1,
										timedata.tm_year + 1900);
		outstr.ReplaceAll("%DATE%",timestr);
	}
	
	if (outstr.FindFirst("%EURODATE%") >= 0)
	{
		sprintf(timestr,"%.2d-%.2d-%d",timedata.tm_mon + 1,timedata.tm_mday,
										timedata.tm_year + 1900);
		outstr.ReplaceAll("%EURODATE%",timestr);
	}
	
	if (outstr.FindFirst("%REVERSEDATE%") >= 0)
	{
		sprintf(timestr,"%d-%.2d-%.2d",timedata.tm_year + 1900,timedata.tm_mon + 1,
										timedata.tm_mday);
		outstr.ReplaceAll("%REVERSEDATE%",timestr);
	}
	
	if (outstr.FindFirst("%TIME%") >= 0)
	{
		sprintf(timestr,"%.2d:%.2d:%.2d",timedata.tm_hour,timedata.tm_min,
										timedata.tm_sec);
		outstr.ReplaceAll("%TIME%",timestr);
	}
	
	// Attribute-based patterns
	strpos = outstr.FindFirst("%ATTR:");
	while (strpos >= 0)
	{
		int32 strendpos = outstr.FindFirst("%",strpos + 1);
		if (strendpos >= 0)
		{
			BString string,attrname, attrpattern;
			attrname = outstr.String() + strpos;
			
			attrpattern = attrname;
			
			attrname.RemoveFirst("%ATTR:");
			attrname.RemoveFirst("%");
			
			attr_info info;
			BNode node(&ref);
			if (node.InitCheck() != B_OK)
			{
				outstr.RemoveAll(attrpattern.String());
				strpos = outstr.FindFirst("%ATTR:");
				continue;
			}
			
			if (node.GetAttrInfo(attrname.String(),&info) == B_OK)
			{
				if (info.type == B_STRING_TYPE)
				{
					node.ReadAttrString(attrname.String(),&string);
					outstr.ReplaceAll(attrpattern.String(),string.String());
				}
				else if (info.type == B_INT32_TYPE)
				{
					int32 attrdata;
					if (node.ReadAttr(attrname.String(),B_INT32_TYPE,0,
									(void*)&attrdata,sizeof(int32)) == B_OK)
					{
						string = "";
						string << attrdata;
						outstr.ReplaceAll(attrpattern.String(),string.String());
					}
				}
			}
		}
		strpos = outstr.FindFirst("%ATTR:");
	}
	
	return outstr;
}

