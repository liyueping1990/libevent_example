#include <iostream>
#include <event2/event.h>
#include <thread>
#include <chrono>
#include <string.h>

#ifndef _WIN32
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#define PORT 5001

// 正常断开连接、超时都会进入
static void cllient_cb(evutil_socket_t s, short w, void* arg)
{
    std::cout << __func__ << std::endl;

    event* ev = (event*)arg;
    if (w&EV_TIMEOUT)
    {
        // 需要清理event            
        std::cout << "timeout" << std::flush;
        evutil_closesocket(s);
        event_free(ev);
        return;
    }
    
    
    char buf[4096] = {0};
    int len = recv(s, buf, sizeof(buf) - 1, 0);
    if (len > 0)
    {
         std::cout << buf << std::endl;
         send(s, "ok", 2, 0);
    }
    else
    {
        // 需要清理event
        std::cout << "." << std::flush;
        event_free(ev);
        evutil_closesocket(s);
    }   
}



static void cb(evutil_socket_t s, short w, void* arg)
{
    std::cout << __func__ << std::endl;
    // 读取连接信息
    sockaddr_in sin;
    socklen_t size = sizeof(sin);
    evutil_socket_t client = accept(s, (sockaddr*)&sin, &size);
    
    char ip[16] = {0};
    evutil_inet_ntop(AF_INET, &sin.sin_addr, ip, sizeof(ip) -1);
    std::cout << "ip: " << ip << std::endl;

    // 带超时的读取数据事件
    event_base* base = (event_base*)arg; 
    event* ev = event_new(base, client, EV_READ | EV_PERSIST, cllient_cb, event_self_cbarg());
    timeval t = {10, 0};
    event_add(ev, &t);
}


int main(int argc, char* argv[])
{
    #ifdef _WIN32
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
    #else
        if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        {
            return 1;
        }
    #endif
    
    event_base* base = event_base_new();

    // event 服务器
    evutil_socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock <= 0)
    {
        std::cout << "socket error " << strerror(sock) << std::endl;
        return -1;
    }

    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);
    int re = bind(sock, (sockaddr*)&sin, sizeof(sin));
    if (re != 0)
    {
        std::cerr << "bind error:" << strerror(errno) << std::endl;
        return -1;
    }

    // 设置地址复用和非阻塞
    evutil_make_socket_nonblocking(sock);
    evutil_make_listen_socket_reuseable(sock);
    
    // 开始监听并接受连接事件, 默认水平触发，不处理会一直进回调，边缘触发只进入一次，回调里需要直到收不到数据再退出
    listen(sock, 10);
    event* ev = event_new(base, sock, EV_READ | EV_PERSIST, cb, base);       // 水平触发
    //event* ev = event_new(base, sock, EV_READ | EV_PERSIST | EV_ET, cb, base); // 边缘触发
    event_add(ev, 0);    

    event_base_dispatch(base);
    event_base_free(base);
    evutil_closesocket(sock);
}