/*
	PatternProcessor.h: Code to process substitution patterns
	Written by DarkWyrm <darkwyrm@gmail.com>, Copyright 2008
	Released under the MIT license.
*/

#ifndef PATTERN_PROCESSOR_H
#define PATTERN_PROCESSOR_H

#include <Entry.h>
#include <String.h>

BString ProcessPatterns(const char* instr, const entry_ref& ref);

#endif	// PATTERN_PROCESSOR_H
