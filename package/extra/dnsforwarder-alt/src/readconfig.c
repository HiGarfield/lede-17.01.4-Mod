#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "readconfig.h"
#include "utils.h"
#include "readline.h"

#define DUP_STRING(i, s)    ((Info)->StrBuffer.Add(&((Info)->StrBuffer), (s), NULL))

int ConfigInitInfo(ConfigFileInfo *Info)
{
	Info->fp = NULL;

	if( StringList_Init(&(Info->StrBuffer), NULL, NULL) != 0 )
    {
        return -14;
    }

    if( StringChunk_Init(&(Info->Options), NULL) != 0 )
    {
        return -19;
    }

	return 0;
}

int ConfigOpenFile(ConfigFileInfo *Info, const char *File)
{
	Info->fp = fopen(File, "r");
	if( Info->fp == NULL )
		return -56;
	else
		return 0;
}

int ConfigCloseFile(ConfigFileInfo *Info)
{
	return fclose(Info->fp);
}

int ConfigAddOption(ConfigFileInfo *Info,
                    char *KeyName,
                    MultilineStrategy Strategy,
                    OptionType Type,
                    VType Initial
                    )
{
	ConfigOption New;

	New.Type = Type;
	New.Status = STATUS_DEFAULT_VALUE;
	New.Strategy = Strategy;

	switch( Type )
	{
		case TYPE_INT32:
			New.Holder.INT32 = Initial.INT32;
			break;

		case TYPE_BOOLEAN:
			New.Holder.boolean = Initial.boolean;
			break;

		case TYPE_PATH:
		case TYPE_STRING:
			if( StringList_Init(&(New.Holder.str), Initial.str, ",") != 0 )
			{
				return 2;
			}

			New.Delimiters = ",";
            if( New.Delimiters == NULL )
            {
                return -68;
            }

			break;

		default:
			break;
	}

	return StringChunk_Add(&(Info->Options), KeyName, (const char *)&New, sizeof(ConfigOption));
}

int ConfigAddAlias(ConfigFileInfo *Info,
                   const char *Target,
                   const char *Alias,
                   const char *Prepending,
                   const char *StringDelimiters
                   )
{
	ConfigOption New;

	New.Type = TYPE_ALIAS;

	New.Holder.Aliasing.Target = DUP_STRING(Info, Target);

	if( Prepending != NULL )
    {
        New.Holder.Aliasing.Prepending = DUP_STRING(Info, Prepending);
    } else {
        New.Holder.Aliasing.Prepending = NULL;
    }

	if( StringDelimiters != NULL )
    {
        New.Delimiters = DUP_STRING(Info, StringDelimiters);
    } else {
        New.Delimiters = NULL;
    }

	return StringChunk_Add(&(Info->Options),
                           Alias,
                           (const char *)&New,
                           sizeof(ConfigOption)
                           );
}

/* **Prepending and **StringDelimiters should be NULL before this is called */
static ConfigOption *GetOptionOfAInfo(ConfigFileInfo *Info,
                                      const char *KeyName,
                                      const char **Prepending,
                                      const char **StringDelimiters
                                      )
{
	ConfigOption *Option;

	if( StringChunk_Match_NoWildCard(&(Info->Options), KeyName, NULL, (void **)&Option) == TRUE )
	{
		if( Option->Type == TYPE_ALIAS )
		{
		    if( Prepending != NULL )
            {
                *Prepending = Option->Holder.Aliasing.Prepending;
            }

            if( StringDelimiters != NULL )
            {
                *StringDelimiters = Option->Delimiters;
            }

			return GetOptionOfAInfo(Info,
                                    Option->Holder.Aliasing.Target,
                                    Prepending,
                                    StringDelimiters
                                    );
		} else {
			return Option;
		}
	} else {
		return NULL;
	}
}

int ConfigSetStringDelimiters(ConfigFileInfo *Info,
                              char *KeyName,
                              const char *Delimiters
                              )
{
    ConfigOption *Option;

    Option = GetOptionOfAInfo(Info, KeyName, NULL, NULL);
    if( Option == NULL )
    {
        return -147;
    }

    if( Option->Type != TYPE_STRING )
    {
        return -148;
    }

    Option->Delimiters = DUP_STRING(Info, Delimiters);
    if( Option->Delimiters == NULL )
    {
        return -130;
    }

    return 0;
}

static BOOL GetBooleanValueFromString(const char *str)
{
	if( isdigit(*str) )
	{
		if( *str == '0' )
			return FALSE;
		else
			return TRUE;
	} else {
	    char Dump[8];

	    strncpy(Dump, str, sizeof(Dump));
	    Dump[sizeof(Dump) - 1] = '\0';

		StrToLower(Dump);

		if( strstr(Dump, "false") != NULL )
			return FALSE;
		else if( strstr(Dump, "true") != NULL )
			return TRUE;

		if( strstr(Dump, "no") != NULL )
			return FALSE;
		else if( strstr(Dump, "yes") != NULL )
			return TRUE;
	}

	return FALSE;
}

static void ParseBoolean(ConfigOption *Option, const char *Value)
{
	switch (Option->Strategy)
	{
		case STRATEGY_APPEND_DISCARD_DEFAULT:
			if( Option->Status == STATUS_DEFAULT_VALUE )
			{
				Option->Strategy = STRATEGY_APPEND;
			}
			/* No break */

		case STRATEGY_DEFAULT:
		case STRATEGY_REPLACE:

			Option->Holder.boolean = GetBooleanValueFromString(Value);

			Option->Status = STATUS_SPECIAL_VALUE;
			break;

		case STRATEGY_APPEND:
			{
				BOOL SpecifiedValue;

				SpecifiedValue = GetBooleanValueFromString(Value);
				Option->Holder.boolean |= SpecifiedValue;

				Option->Status = STATUS_SPECIAL_VALUE;
			}
			break;

		default:
			break;

	}
}

static void ParseInt32(ConfigOption *Option, const char *Value)
{
	switch (Option->Strategy)
	{
		case STRATEGY_APPEND_DISCARD_DEFAULT:
			if( Option->Status == STATUS_DEFAULT_VALUE )
			{
				Option->Strategy = STRATEGY_APPEND;
			}
			/* No break */

		case STRATEGY_DEFAULT:
		case STRATEGY_REPLACE:
			sscanf(Value, "%d", &(Option->Holder.INT32));
			Option->Status = STATUS_SPECIAL_VALUE;
			break;

		case STRATEGY_APPEND:
			{
				int32_t SpecifiedValue;

				sscanf(Value, "%d", &SpecifiedValue);
				Option->Holder.INT32 += SpecifiedValue;

				Option->Status = STATUS_SPECIAL_VALUE;
			}
			break;

		default:
			break;
	}
}

static void ParseString(ConfigOption *Option,
                        const char *Delimiters,
                        const char *Value,
                        ReadLineStatus ReadStatus,
                        BOOL Trim,
                        FILE *fp,
                        char *Buffer,
                        int BufferLength
                        )
{
	switch( Option->Strategy )
	{
		case STRATEGY_APPEND_DISCARD_DEFAULT:
			if( Option->Status == STATUS_DEFAULT_VALUE )
			{
				Option->Strategy = STRATEGY_APPEND;
			}
			/* No break */

		case STRATEGY_DEFAULT:
		case STRATEGY_REPLACE:
			Option->Holder.str.Clear(&(Option->Holder.str));
			/* No break */

		case STRATEGY_APPEND:
			if( Option->Holder.str.Add(&(Option->Holder.str),
                                       Value,
                                       Delimiters
                                       )
                == NULL )
			{
				return;
			}
			Option->Status = STATUS_SPECIAL_VALUE;
			break;

		default:
			return;
			break;
	}

	while( ReadStatus != READ_DONE ){

		ReadStatus = ReadLine(fp, Buffer, BufferLength);
		if( ReadStatus == READ_FAILED_OR_END )
			break;

		Option->Holder.str.AppendLast(&(Option->Holder.str), Buffer, Delimiters);
	}

	if( Trim )
    {
        Option->Holder.str.TrimAll(&(Option->Holder.str), NULL);
    }
}

static char *TrimPath(char *Path)
{
	char *LastCharacter = StrRNpbrk(Path, "\"");
	char *FirstLetter;

	if( LastCharacter != NULL )
	{
		*(LastCharacter + 1) = '\0';

		FirstLetter = StrNpbrk(Path, "\"\t ");
		if( FirstLetter != NULL )
		{
			memmove(Path, FirstLetter, strlen(FirstLetter) + 1);
			return Path;
		} else {
			return NULL;
		}
	} else {
		return NULL;
	}
}

int ConfigRead(ConfigFileInfo *Info)
{
	int				NumOfRead	=	0;

	char			Buffer[2048];
	char			*ValuePos;
	ReadLineStatus	ReadStatus;

	char			*KeyName;
	ConfigOption	*Option;

	const char      *Prepending;
	const char      *StringDelimiters;

	while(TRUE){
		ReadStatus = ReadLine(Info->fp, Buffer, sizeof(Buffer));
		if( ReadStatus == READ_FAILED_OR_END )
			return NumOfRead;

		ValuePos = SplitNameAndValue(Buffer, " \t=");
		if( ValuePos == NULL )
			continue;

		KeyName = Buffer;

		Prepending = NULL;
		StringDelimiters = NULL;
		Option = GetOptionOfAInfo(Info,
                                  KeyName,
                                  &Prepending,
                                  &StringDelimiters
                                  );

		if( Option == NULL )
        {
            continue;
        }

        if( Prepending != NULL )
        {
            switch( Option->Type )
            {
                case TYPE_INT32:
                    ParseInt32(Option, Prepending);
                    break;

                case TYPE_BOOLEAN:
                    ParseBoolean(Option, Prepending);
                    break;

                case TYPE_PATH:
                    StringDelimiters = "";
                    /* No break */

                case TYPE_STRING:
                    ParseString(Option,
                                StringDelimiters == NULL ?
                                    Option->Delimiters : StringDelimiters,
                                Prepending,
                                READ_DONE,
                                FALSE,
                                Info->fp,
                                NULL,
                                0
                                );
                    break;

                default:
                    break;
            }
        }

		switch( Option->Type )
		{
			case TYPE_INT32:
				ParseInt32(Option, ValuePos);
				break;

			case TYPE_BOOLEAN:
				ParseBoolean(Option, ValuePos);
				break;

			case TYPE_PATH:
                if( ReadStatus != READ_DONE )
                {
					break;
                }

                if( TrimPath(ValuePos) == NULL )
                {
					break;
				}

				ExpandPath(ValuePos, sizeof(Buffer) - (ValuePos - Buffer));
				StringDelimiters = "";
				/* No break */

			case TYPE_STRING:
				ParseString(Option,
                            StringDelimiters == NULL ?
                                Option->Delimiters : StringDelimiters,
                            ValuePos,
                            ReadStatus,
                            TRUE,
                            Info->fp,
                            Buffer,
                            sizeof(Buffer)
                            );
				break;

			default:
				break;
		}
		++NumOfRead;
	}

	return NumOfRead;
}

const char *ConfigGetRawString(ConfigFileInfo *Info, char *KeyName)
{
	ConfigOption *Option = GetOptionOfAInfo(Info, KeyName, NULL, NULL);

	if( Option != NULL )
	{
		StringListIterator  sli;

        if( StringListIterator_Init(&sli, &(Option->Holder.str)) != 0 )
        {
            return NULL;
        }

        return sli.Next(&sli);
	} else {
	    return NULL;
	}
}

StringList *ConfigGetStringList(ConfigFileInfo *Info, char *KeyName)
{
	ConfigOption *Option = GetOptionOfAInfo(Info, KeyName, NULL, NULL);

	if( Option != NULL )
	{
		if( Option->Holder.str.Count(&(Option->Holder.str)) == 0 )
		{
			return NULL;
		} else {
			return &(Option->Holder.str);
		}
	} else {
		return NULL;
	}
}

int32_t ConfigGetNumberOfStrings(ConfigFileInfo *Info, char *KeyName)
{
	ConfigOption *Option = GetOptionOfAInfo(Info, KeyName, NULL, NULL);

	if( Option != NULL )
	{
		return Option->Holder.str.Count(&(Option->Holder.str));
	} else {
		return 0;
	}
}

int32_t ConfigGetInt32(ConfigFileInfo *Info, char *KeyName)
{
	ConfigOption *Option = GetOptionOfAInfo(Info, KeyName, NULL, NULL);

	if( Option != NULL )
	{
		return Option->Holder.INT32;
	} else {
		return 0;
	}
}

BOOL ConfigGetBoolean(ConfigFileInfo *Info, char *KeyName)
{
	ConfigOption *Option = GetOptionOfAInfo(Info, KeyName, NULL, NULL);

	if( Option != NULL )
	{
		return Option->Holder.boolean;
	} else {
		return FALSE;
	}
}

/* Won't change the Option's status */
void ConfigSetDefaultValue(ConfigFileInfo *Info, VType Value, char *KeyName)
{
	ConfigOption *Option = GetOptionOfAInfo(Info, KeyName, NULL, NULL);

	if( Option != NULL )
	{
		switch( Option->Type )
		{
			case TYPE_INT32:
				Option->Holder.INT32 = Value.INT32;
				break;

			case TYPE_BOOLEAN:
				Option->Holder.boolean = Value.boolean;
				break;

			case TYPE_STRING:
				Option->Holder.str.Clear(&(Option->Holder.str));
				Option->Holder.str.Add(&(Option->Holder.str),
                                       Value.str,
                                       Option->Delimiters
                                       );
				break;

			default:
				break;
		}
	}
}

void ConfigFree(ConfigFileInfo *Info)
{
    int32_t Start = 0;
    ConfigOption *Option;

    while( StringChunk_Enum_NoWildCard(&(Info->Options),
                                       &Start,
                                       (void **)&Option)
           != NULL )
    {
        if( Option != NULL )
        {
            switch( Option->Type )
            {
                case TYPE_INT32:
                    break;

                case TYPE_BOOLEAN:
                    break;

                case TYPE_STRING:
                    Option->Holder.str.Free(&(Option->Holder.str));
                    break;

                default:
                    break;
            }
        }
    }

    Info->StrBuffer.Free(&(Info->StrBuffer));
    StringChunk_Free(&(Info->Options), TRUE);
}
