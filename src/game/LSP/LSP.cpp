#include "LSP.hpp"
#include "../Dvar/Dvar.hpp"
#include "../IWNet/IWNet.hpp"
#include "../Live/Live.hpp"
#include "../Sys/Sys.hpp"
#include "../Com/Com.hpp"
#include "../MSG/MSG.hpp"
#include "../CL/CL.hpp"
#include "../Unsorted/Unsorted.hpp"

#include <utils/memory/memory.hpp>

//DONE : 0x00456330
void LSP_Init()
{
    s_logStrings = 1;
    s_sendStats = 1;
	lsp_debug = Dvar_RegisterBool("lsp_debug", 0, 0, "Whether to print lsp debug info");
    g_iwnetMatchmakingServerAddr.type = NA_IP;
    g_iwnetStorageServerAddr.type = NA_IP; // Idk if this one is correct, the one above is correct tho
}

//DONE : 0x00435D90
bool LSP_Connected()
{
    return lsp_connected;
}

//DONE : 0x00481000
bool LSP_FindTitleServers()
{
	bool result = IWNet_DNSResolved();
	lsp_connected = result;
	return result;
}

//DONE : 0x0057D890
bool LSP_FindTitleServers_f()
{
	return LSP_FindTitleServers();
}

//DONE : 0x0066D500
void InitLog(std::int32_t localControllerIndex)
{
	std::int64_t v2[2];

    MSG_Init(&unk_66C7160, *(char**)0x66C7188, 1200);
    Live_GetLSPXuid(localControllerIndex, v2);
	const char* localClientName = Live_GetLocalClientName();
    LSP_WritePacketHeader(localControllerIndex, &unk_66C7160, v2[0], v2[1], (char*)localClientName);
}

//TODO : 0x682400
void /*__usercall*/ LSP_CheckForLogSend(std::int32_t a1, std::int32_t a2)
{
	SessionData* currentSession = Live_GetCurrentSession();

	if (!logMsgInittialized)
	{
		Sys_EnterCriticalSection(CRITSECT_LIVE);

		if (!logMsgInittialized)
		{
			if (!Live_IsSignedIn())
			{
				Sys_LeaveCriticalSection(CRITSECT_LIVE);
				return;
			}
			InitLog(a1);
            logMsgInittialized = true;
		}
		Sys_LeaveCriticalSection(CRITSECT_LIVE);
	}

	if(a2 + *(std::int32_t*)0x66C7174 >= *(std::int32_t*)0x66C7170
		|| *(std::int32_t*)0x66C6C14 /*s_currentControllerIndex*/ != a1
		|| *((unsigned char**)currentSession + 8) && memcmp(currentSession + 118, *(std::int32_t**)0x66C7118, 8))
	{
		//fucking LSP_SendLogRequest is TODO
	}
}

//DONE : 0x4B62C0
void LSP_LogStringEvenIfControllerIsInactive(const char* string)
{
    char* i;
    std::int64_t v10 = 0;

    if (s_logStrings)
    {
        for (i = (char*)string; *i == 10; ++i)
        {
            if (!i[1])
            {
                break;
            }
        }

        if (strlen(i) + (*(msg_t*)0x1BEBC64).curSize + 6 > (*(msg_t*)0x1BEBC64).maxSize)
        {
            LSP_ForceSendPacket();
        }

        if (!logMsgInittialized)
        {
            Sys_EnterCriticalSection(CRITSECT_LSP);
            if (!logMsgInittialized)
            {
                logMsgInittialized = true;
                MSG_Init((msg_t*)0x1BEBC54, *(char**)0x1BEBC80, 1200);

                if (CL_AllLocalClientsInactive() || (CL_GetFirstActiveControllerIndex(), !Live_IsSignedIn()))
                {
                    LSP_WritePacketHeader(*(std::int32_t*)0x1BEB720,
                        (msg_t*)0x1BEBC54,
                        v10,
                        v10, "Not signed in");
                }
                else
                {
                    const char* v7 = memory::call<const char* ()>(0x441FC0)(); //some steam call
                    Live_GetLSPXuid(CL_GetFirstActiveControllerIndex(), &v10);
                    LSP_WritePacketHeader(
                        CL_GetFirstActiveControllerIndex(),
                        (msg_t*)0x1BEBC54,
                        v10,
                        v10,
                        (char*)v7
                    );
                }
            }
            Sys_LeaveCriticalSection(CRITSECT_LSP);
        }

        if (!*(std::int32_t*)0x1BEBC10/*s_firstLogWriteTime*/)
        {
            *(std::int32_t*)0x1BEBC10 = Sys_Milliseconds();
        }

        Sys_EnterCriticalSection(CRITSECT_LSP);
        MSG_WriteByte((msg_t*)0x1BEBC54, 2);
        MSG_WriteLong((msg_t*)0x1BEBC54, Sys_Milliseconds());
        MSG_WriteString((msg_t*)0x1BEBC54, i);
        Sys_LeaveCriticalSection(CRITSECT_LSP);
    }
}

//DONE : 0x00450960
//this function may need to be moved
std::int32_t Xenon_SendLSPPacket(const char* buf, std::int32_t size, netadr_t* net)
{
    sockaddr to;
    NetadrToSockadr(net, &to);
    std::int32_t v3 = sendto(*(std::uint32_t*)0x1A02EC4, (const char*)buf, size, 0, &to, 16);
    if (*(std::uint8_t*)0x1BEAEA4 + 16) //some sort of struct most likely
    {
        std::uint16_t v4 = ntohs(net->port);
        Com_Printf(14,
            "Sending %i byte LSP packet to %u.%u.%u.%u:%i\n",
            size,
            net->ip[0],
            net->ip[1],
            net->ip[2],
            net->ip[3],
            v4);
    }

    if (v3 != size)
    {
        std::uint16_t v5 = ntohs(net->port);
        Com_Printf(14,
            "Sending %i(actually sent %i) byte LSP packet to %u.%u.%u.%u:%i\n",
            size,
            v3,
            net->ip[0],
            net->ip[1],
            net->ip[2],
            net->ip[3],
            v5);
    }

    return v3;
}


//DONE : 0x00423090
void LSP_ForceSendPacket()
{
    if (lsp_connected)
    {
        Sys_EnterCriticalSection(CRITSECT_LSP);

        if (logMsgInittialized)
        {
            g_iwnetLoggingServerAddr.port = htons(3005);

            if (Xenon_SendLSPPacket((const char*)(*(msg_t*)0x1BEBC5C).data, (*(msg_t*)0x1BEBC68).curSize, &g_iwnetLoggingServerAddr) < 0)
            {
                lsp_connected = false;
            }
        }
        logMsgInittialized = false;
        Sys_LeaveCriticalSection(CRITSECT_LSP);
    }
}

//DONE : 0x00498FE0
void LSP_WritePacketHeader(std::int32_t localControllerIndex, msg_t* msg, std::int32_t xuid, std::int32_t gamertag, char* source)
{
    const char* map;
    MSG_WriteByte(msg, 14);
    MSG_WriteBit1(msg);
    MSG_WriteInt64(msg, xuid, gamertag);
    MSG_WriteString(msg, source);
    MSG_WriteString(msg, va("%s %s build %s %s", "OpenIW4 SP", "inf-dev", getBuildNumber(), "win-x86"));

    if (sv_map)
    {
        map = sv_map->current.string;
    }
    else
    {
        map = "no map yet";
    }
    MSG_WriteString(msg, map);
    MSG_WriteBit1(msg);
    *(unsigned long*)0x1BEB720 = localControllerIndex;
    *(std::int32_t*)0x1BEBC10 = 0; //s_firstLogWriteTime
}
