#include <iostream>
#include <string.h>
#include <stdlib.h>
#include "event2/event.h"
#include "event2/thread.h"
#include "event2/listener.h"

#ifdef _WIN32
#else
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

    // 配置上下文
    event_config* conf = event_config_new();

    // 显示支持的网络模式
    const char **method = event_get_supported_methods();
    std::cout << "支持的网络模式：" << std::endl;
    for (size_t i = 0; method[i] != nullptr; i++)
    {
        std::cout << method[i] << std::endl;
    }



#ifdef _WIN32
    // windows支持iocp
    event_config_set_flag(conf, EVENT_BASE_FLAG_STARTUP_IOCP);

    // 初始化线程池
    evthread_use_windows_threads();

    // 设置cpu数量
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    event_config_set_num_cpus_hint(conf, si.dwNumberOfProcessors);
#else
    // 设置特征
    event_config_require_features(conf, EV_FEATURE_FDS); // 配置之后不支持epoll

    //设置网络模型，使用select
    event_config_avoid_method(conf, "epoll");
    event_config_avoid_method(conf, "poll");

#endif // _WIN32

    // 申请资源
    event_base* base = event_base_new_with_config(conf);
    if (!base)
    {
        std::cout << "config failed" << std::endl;
        base = event_base_new();
        if (!base)
        {
            std::cout << "base failed" << std::endl;
            return 0;
        }
    }

    std::cout << "success" << std::endl;
    std::cout << "current method:" << event_base_get_method(base) << std::endl;
    std::cout << "current features:" << event_base_get_features(base) << std::endl;

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
    event_base_dispatch(base);

    if (base)
    {
        event_base_free(base);// 释放资源
    }

    if (conf)
    {
        event_config_free(conf);
    }

    if (ev)
    {
        evconnlistener_free(ev);
    }

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}