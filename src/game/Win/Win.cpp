#include "Win.hpp"
#include "../Com/Com.hpp"

#include <utils/memory/memory.hpp>

//THUNK : 0x445EF0
bool Win_InitLocalization(language_t* language)
{
	return memory::call<bool(language_t*)>(0x445EF0)(language);
}

//DONE : 0x4FEDF0
void Win_ShutdownLocalization()
{
    localization->language = 0;
    localization->strings = 0;
}

//DONE : 0x498760
const char* Win_GetLocalizationFilename()
{
	return "localization.txt";
}

//DONE : 0x429140
const char* Win_LocalizeRef(const char* ref)
{
	Com_BeginParseSession("localization");

	const char* strings = localization->language;

	for (char* i = Com_Parse(&strings); *i; i = Com_Parse(&strings))
	{
        std::int32_t v2 = strcmp(i, ref);
        char* v3 = Com_Parse(&strings);

        if (!*v3)
        {
            break;
        }

        if (!v2)
        {
            Com_EndParseSession();
            return va("%s", v3);
        }
	}

    Com_EndParseSession();
    return va("%s", ref);
}
