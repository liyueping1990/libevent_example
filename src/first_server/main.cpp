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
    if (signal(SIGPIPE, SIG_IGN) == SIG_IGN) // 向关闭的socket发送数据
    {
        return 1;
    }
#endif // _WIN32

    // 申请资源
    event_base* base = event_base_new();
    if (base)
    {
        printf("event_base_new sucess\r\n");
    }

    // 监听端口
    // socket、bind、listen
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(SPORT);

    evconnlistener* ev = evconnlistener_new_bind(base, // libevent的上下文
                         linsten_cb, // 接收连接的回调函数
                         base,       // 回调函数获取的参数
                         LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, // 地址重用，evconnlistenner关闭时关闭socket
                         10, // 连接队列大小，对应listen函数
                         (sockaddr*)&sin, // 绑定的地址和端口
                         sizeof(sin));

    // 事件分发处理
    if (base)
    {
        event_base_dispatch(base);
    }
    if (ev)
    {
        evconnlistener_free(ev);
    }
    if (base) // 释放资源
    {
        event_base_free(base);
    }
#ifdef _WIN32
    WSACleanup();
#endif

    system("pause");
    return 0;
}