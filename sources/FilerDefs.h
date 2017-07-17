/*
 * Copyright 2016. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#ifndef FILERDEFS_H
#define FILERDEFS_H

static const char*	kDoAll = "all";
static const char*	kReplace = "rpl";
static const char*	kPointer = "ptr";
static const char*	kFilerSignature = "application/x-vnd.dw-Filer";
static const char*	kAutoFilerSignature = "application/x-vnd.dw-AutoFiler";
static const char	kSettingsFolder[] = "Filer";
static const char	kSettingsFile[] = "Filer_settings";

#define MSG_AUTO_FILER			'auto'

#define MSG_DOCS				'docs'
#define MSG_HELP				'help'
#define MSG_TYPE_CHOSEN			'tych'
#define MSG_NEW_DESCRIPTION		'dsch'
#define MSG_OK					'okay'
#define MSG_CANCEL				'cncl'

#define MSG_REPLICATE			'repl'
#define MSG_OPEN_FILER			'opfi'

#define MSG_ADD_TEST			'adts'
#define MSG_REMOVE_TEST			'rmts'
#define MSG_ADD_ACTION			'adac'
#define MSG_REMOVE_ACTION		'rmac'
#define MSG_ADD_RULE			'adrl'
#define MSG_UPDATE_RULE			'uprl'
#define MSG_FORCE_QUIT			'frcq'

#define MSG_SHOW_ADD_WINDOW		'shaw'
#define MSG_SHOW_EDIT_WINDOW	'shew'
#define MSG_DISABLE_RULE		'shdr'
#define MSG_REMOVE_RULE			'shrr'
#define MSG_RULE_SELECTED		'rlsl'
#define MSG_MOVE_RULE_UP		'mvup'
#define MSG_MOVE_RULE_DOWN		'mvdn'
#define MSG_MATCH_ONCE			'chon'

#define MSG_STARTSTOP_AUTOFILER	'ssaf'
#define MSG_AUTOFILER_AUTORUN	'afas'
#define MSG_UPDATE_LABEL		'upla'
#define MSG_SHOW_ADD_PANEL		'shap'
#define MSG_SHOW_EDIT_PANEL		'shep'
#define MSG_REMOVE_FOLDER		'rmfl'
#define MSG_FOLDER_SELECTED		'flsl'
#define MSG_FOLDER_CHOSEN		'flch'
#define MSG_REFRESH_FOLDERS		'flrf'

#define MSG_ACTION_CHOSEN		'tsch'
#define MSG_SHOW_ACTION_MENU	'sham'

#define MSG_TEST_CHOSEN			'tsch'
#define MSG_MODE_CHOSEN			'mdch'
#define MSG_UNIT_CHOSEN			'utch'
#define MSG_SHOW_TEST_MENU		'shtm'
#define MSG_SHOW_MODE_MENU		'shmm'

#endif // FILERDEFS_H
