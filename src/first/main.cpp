#include <iostream>
#include <string.h>
#include <stdlib.h>
#include "event2/listener.h"
#include "event2/event.h"


#ifndef _WIN32
#include <signal.h>
#endif // _WIN32

int main()
{
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#else
    if (signal(SIGPIPE, SIG_IGN))
    {
        return 1;
    }
#endif // _WIN32

    event_base* base = event_base_new();
    if (base)
    {
        printf("sucess\r\n");
    }

    system("pause");
    return 0;
}