#ifndef _READCONFIG_
#define _READCONFIG_

#include <stdio.h>
#include "stringlist.h"
#include "stringchunk.h"
#include "array.h"
#include "common.h"

/* A valid line of a configuration file has the following structure:
 *  <Option> <value>
 * Where `<Option>' is the name of a option, here we call it `KEY NAME'.
 * And `<value>' is the option's value, we just call it `value'.
 * A line started with `#' is a comment, which will be ignored when it is read.
 * A valid option can be followed a comment which will be ignored too:
 *  <Option> <value> # I'm a comment.
 *
 */

/* Set the max length of a key name */
#define	KEY_NAME_MAX_SIZE	64

/* A value must have a type. We just need these types for now. */
typedef enum _OptionType{
    TYPE_ALIAS = -1,
	TYPE_UNDEFINED = 0,

	TYPE_INT32,
	TYPE_BOOLEAN,
	TYPE_PATH,
	TYPE_STRING
} OptionType;

typedef enum _MultilineStrategy{
	STRATEGY_DEFAULT = 0,
	STRATEGY_REPLACE,
	STRATEGY_APPEND,
	STRATEGY_APPEND_DISCARD_DEFAULT
} MultilineStrategy;

typedef union _VType{
	const char  *str;
	int32_t     INT32;
	BOOL        boolean;
} VType;

typedef enum _OptionStatus{
	STATUS_DEPRECATED = -2,
	STATUS_UNUSED = 0,
	STATUS_DEFAULT_VALUE,
	STATUS_SPECIAL_VALUE
}OptionStatus;

/* An option */
typedef struct _Option{
	/* Designate if this option is used. */
	OptionStatus	Status;

	MultilineStrategy	Strategy;

	/* Type */
	OptionType	Type;

	/* Value holder */
	union {
		StringList	str;
		int32_t     INT32;
		BOOL		boolean;

		struct {
		    char    *Target;
		    char    *Prepending;
		} Aliasing;
	} Holder;

	char *Delimiters;

} ConfigOption;

/* The exposed type(The infomations about a configuration file) to read options from a configuration file. */
typedef struct _ConfigFileInfo
{
	/* Static, once inited, never changed. */
	FILE	*fp;

    StringList  StrBuffer;

	/* An array of all the options. */
	StringChunk	Options;
} ConfigFileInfo;

int ConfigInitInfo(ConfigFileInfo *Info);

int ConfigOpenFile(ConfigFileInfo *Info, const char *File);

int ConfigCloseFile(ConfigFileInfo *Info);

int ConfigAddOption(ConfigFileInfo *Info,
                    char *KeyName,
                    MultilineStrategy Strategy,
                    OptionType Type,
                    VType Initial
                    );

int ConfigAddAlias(ConfigFileInfo *Info,
                   const char *Target,
                   const char *Alias,
                   const char *Prepending,
                   const char *StringDelimiters
                   );

int ConfigSetStringDelimiters(ConfigFileInfo *Info,
                              char *KeyName,
                              const char *Delimiters
                              );

int ConfigRead(ConfigFileInfo *Info);

const char *ConfigGetRawString(ConfigFileInfo *Info, char *KeyName);

StringList *ConfigGetStringList(ConfigFileInfo *Info, char *KeyName);

int32_t ConfigGetNumberOfStrings(ConfigFileInfo *Info, char *KeyName);

int32_t ConfigGetInt32(ConfigFileInfo *Info, char *KeyName);

BOOL ConfigGetBoolean(ConfigFileInfo *Info, char *KeyName);

/* Won't change the Option's status */
void ConfigSetDefaultValue(ConfigFileInfo *Info, VType Value, char *KeyName);

void ConfigFree(ConfigFileInfo *Info);

#endif // _READCONFIG_
