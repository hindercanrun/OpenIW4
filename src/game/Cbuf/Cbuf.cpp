#include "Cbuf.hpp"

#include "../Sys/Sys.hpp"

#include <utils/memory/memory.hpp>

//THUNK : 0x00404B20
void Cbuf_AddText(int a1, const char* a2)
{
	memory::call<void(int, const char*)>(0x00404B20)(a1, a2);
}

//DONE : 0x4AFB80
void Con_GetTextCopy(char* text, int maxSize)
{
    
    if (con.consoleWindow.activeLineCount)
    {
        std::int32_t begin = con.consoleWindow.lines[con.consoleWindow.firstLineIndex].textBufPos;
        std::int32_t end = con.consoleWindow.textBufPos;
        std::size_t totalSize = con.consoleWindow.textBufPos - begin;

        if (con.consoleWindow.textBufPos - begin < 0)
        {
            totalSize += con.consoleWindow.textBufSize;
        }

        if (totalSize > maxSize - 1)
        {
            begin += totalSize - maxSize + 1;

            if (begin > con.consoleWindow.textBufSize)
            {
                begin -= con.consoleWindow.textBufSize;
            }
            totalSize = maxSize - 1;
        }

        if (begin >= con.consoleWindow.textBufPos)
        {
            memcpy(text, &con.consoleWindow.circularTextBuffer[begin], con.consoleWindow.textBufSize - begin);
            memcpy(&text[con.consoleWindow.textBufSize - begin], con.consoleWindow.circularTextBuffer, end);
        }
        else
        {
            memcpy(text, &con.consoleWindow.circularTextBuffer[begin], con.consoleWindow.textBufPos - begin);
        }

        text[totalSize] = 0;
    }
    else
    {
        *text = 0;
    }
}

//DONE : 0x64DD30
int Conbuf_CleanText(const char* source, char* target, int sizeofTarget)
{
	char* start;
	const char* last;

	start = target;
	last = target + sizeofTarget - 3;
	while (*source && target <= last)
	{
		if (source[0] == '\n' && source[1] == '\r')
		{
			target[0] = '\r';
			target[1] = '\n';
			target += 2;
			source += 2;
		}
		else
		{
			if (source[0] == '\r' || source[0] == '\n')
			{
				target[0] = '\r';
				target[1] = '\n';
				target += 2;
				++source;
			}
			else if (source && source[0] == '^' && source[1] && source[1] != '^' && source[1] >= 48 && source[1] <= 64)
			{
				source += 2;
			}
			else
			{
				*target++ = *source++;
			}
		}
	}

	*target = 0;
	return target - start;
}

//DONE : 0x004F5770
void Conbuf_AppendText(const char* pMsg)
{
	const char* v1 = pMsg;
	std::int32_t v2;
	char target[32768];

	if (strlen(pMsg) > 16383)
	{
		v1 = &pMsg[strlen(pMsg) - 16383];
	}

	v2 = Conbuf_CleanText(v1, target, sizeof(target));
	*(std::uint32_t*)0x64A38B8 /*s_totalChars*/ += v2;
	
	if (*(std::uint32_t*)0x64A38B8 <= 0x4000)
	{
		SendMessageA(s_wcd->hwndBuffer, 0xB1, 0xFFFF, 0xFFFF);
	}
	else
	{
		SendMessageA(s_wcd->hwndBuffer, 0xB1, 0, -1);
		*(std::uint32_t*)0x64A38B8 = v2;
	}

	SendMessageA(s_wcd->hwndBuffer, 0xB6, 0, 0xFFFF);
	SendMessageA(s_wcd->hwndBuffer, 0xB7, 0, 0);
	SendMessageA(s_wcd->hwndBuffer, 0xC2, 0, (long)target);
}
