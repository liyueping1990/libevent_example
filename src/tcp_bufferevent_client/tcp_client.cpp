#include <iostream>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
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

void event_cb(bufferevent* be, short what, void* arg)
{
    std::cout << __func__ << std::endl;

    // 超时事件发生后，数据读取停止 
    if (what & BEV_EVENT_TIMEOUT && what & BEV_EVENT_READING)
    {
        std::cout << "BEV_EVENT_READING" << std::endl;

        //bufferevent_enable(be, EV_READ); // 设置重新可读
        bufferevent_free(be); // 清理
    }
    else if (what & BEV_EVENT_TIMEOUT && what & BEV_EVENT_WRITING)
    {
        std::cout << "BEV_EVENT_WRITING" << std::endl;
    }
    else if (what & BEV_EVENT_ERROR) // 错误
    {
        bufferevent_free(be); // 清理
    }
    else
    {

    }
}

void read_cb(bufferevent* be, void* arg)
{
    std::cout << __func__ << std::endl;

    char data[1024] = {0};
    int len = bufferevent_read(be, data, sizeof(data) - 1);
    std::cout << data << std::endl;

    if (strstr(data, "quit") != NULL)
    {
        std::cout << "quit" << std::endl;

        // 关闭buffer_event
        //bufferevent_free(be);
    }

    bufferevent_write(be, "ok", 2);
}

void write_cb(bufferevent* be, void* arg)
{
    std::cout << __func__ << std::endl;    
}


void linsten_cb(struct evconnlistener *e, evutil_socket_t s, struct sockaddr *addr, int socklen, void *arg)
{
    std::cout << __func__ << std::endl;

    event_base* base = (event_base*)arg;

    // 创建bufferevent上下文  BEV_OPT_CLOSE_ON_FREE清理bufferevent时关闭socket
    bufferevent* bev = bufferevent_socket_new(base, s, BEV_OPT_CLOSE_ON_FREE);

    // 添加监控事件
    bufferevent_enable(bev, EV_READ | EV_WRITE);

    // 设置水位，读取水位
    bufferevent_setwatermark(bev, 
                            EV_READ, 
                            0, // 低水位，0 表示无限制，默认0
                            10   // 高水位，0 表示无限制，默认0
                            );

    bufferevent_setwatermark(bev, 
                            EV_WRITE, 
                            0, // 低水位，0 表示无限制，默认0
                            0   // 高水位，0 表示无限制，默认0
                            );

    // 超时事件
    timeval t1 = {3, 0};
    bufferevent_set_timeouts(bev, &t1, 0);

    // set cb
    bufferevent_setcb(bev, read_cb, write_cb, event_cb, base);
}

///////////////////////////////////////////////////////////////////////

void client_event_cb(bufferevent* be, short what, void* arg)
{
    std::cout << __func__ << std::endl;

    std::cout << what << std::endl;

    if (what & BEV_EVENT_TIMEOUT)
    {
        std::cout << "BEV_EVENT_READING" << std::endl;

        //bufferevent_enable(be, EV_READ); // 设置重新可读
        //bufferevent_free(be); // 清理
    }

    if (what & BEV_EVENT_CONNECTED) // 连接上服务端
    {
        std::cout << "BEV_EVENT_CONNECTED" << std::endl;      

        // 激活write事件
        bufferevent_trigger(be, EV_WRITE, 0);
    }

    if (what & BEV_EVENT_EOF) // 服务端关闭事件
    {
        std::cout << "BEV_EVENT_EOF" << std::endl;
    }
}

void client_read_cb(bufferevent* be, void* arg)
{
    std::cout << __func__ << std::endl;


}

void client_write_cb(bufferevent* be, void* arg)
{
    std::cout << __func__ << std::endl;  

    std::cout << "read data" << std::endl;
    FILE* fp = (FILE*)arg;
    if (fp == nullptr)
    {
        std::cout << "fp == nullptr" << std::endl;
    }

    char data[1024] = {0};
    int len = fread(data, 1, sizeof(data)-1, fp);
    if (len <= 0)
    {
        fclose(fp);
        // bufferevent_free(be);// 直接关闭会导致缓冲数据没有发送结束
        bufferevent_disable(be, EV_WRITE); 

        std::cout << "len <= 0" << std::endl;
        return;
    }

    std::cout << data << std::endl;

    // 发送写入buffer
    bufferevent_write(be, data, len);
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

    bufferevent* bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
    if (bev == nullptr)
    {
        std::cout << "bufferevent_socket_new failed" << std::endl;
    }

    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);
    evutil_inet_pton(AF_INET, "10.156.11.69", &sin.sin_addr.s_addr);

    // 设置回调函数
    FILE* fp = fopen("makefile", "rb");
    bufferevent_setcb(bev, client_read_cb, client_write_cb, client_event_cb,fp);
    bufferevent_enable(bev, EV_READ | EV_WRITE);    

    int ret = bufferevent_socket_connect(bev, (sockaddr*)&sin, sizeof(sin));
    if (ret != 0)
    {
        std::cout << "" << std::endl;
    }
    

    /*evconnlistener* ev = evconnlistener_new_bind(base,
        linsten_cb,
        base,
        LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
        10,
        (sockaddr*)&sin,
        sizeof(sin));*/

    event_base_dispatch(base);
    event_base_free(base);
}


