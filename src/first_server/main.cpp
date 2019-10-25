#include <iostream>
#include <string.h>
#include <stdlib.h>
#include "event2/listener.h"
#include "event2/event.h"


#ifndef _WIN32
#include <signal.h>
#endif // _WIN32

#define SPORT 5001

void linsten_cb(struct evconnlistener *e, evutil_socket_t s, struct sockaddr *addr, int socklen, void *arg)
{
    std::cout << "listen_cb" << std::endl;
}

int main()
{
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#else
    if (signal(SIGPIPE, SIG_IGN) == SIG_IGN) // ��رյ�socket��������
    {
        return 1;
    }
#endif // _WIN32

    // ������Դ
    event_base* base = event_base_new();
    if (base)
    {
        printf("event_base_new sucess\r\n");
    }

    // �����˿�
    // socket��bind��listen
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(SPORT);

    evconnlistener* ev = evconnlistener_new_bind(base, // libevent��������
                         linsten_cb, // �������ӵĻص�����
                         base,       // �ص�������ȡ�Ĳ���
                         LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, // ��ַ���ã�evconnlistenner�ر�ʱ�ر�socket
                         10, // ���Ӷ��д�С����Ӧlisten����
                         (sockaddr*)&sin, // �󶨵ĵ�ַ�Ͷ˿�
                         sizeof(sin));

    // �¼��ַ�����
    if (base)
    {
        event_base_dispatch(base);
    }
    if (ev)
    {
        evconnlistener_free(ev);
    }
    if (base) // �ͷ���Դ
    {
        event_base_free(base);
    }
#ifdef _WIN32
    WSACleanup();
#endif

    system("pause");
    return 0;
}