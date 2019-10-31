#include <iostream>
#include <event2/event.h>
#include <thread>
#include <chrono>

#ifndef _WIN32
#include <signal.h>
#endif

static timeval t1 = {1, 0};
static void timer1(int sock, short which, void* arg)
{
    std::cout << __func__ << std::endl;
    event* ev = (event*)arg;
    if (!evtimer_pending(ev, &t1))
    {
        // evtimer_add(ev, &t1);
    }
}

static void timer2(int sock, short which, void* arg)
{
    std::cout << __func__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
}

static void timer3(int sock, short which, void* arg)
{
    std::cout << __func__ << std::endl;
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

    // 定时器,非持久事件
    event* evl = evtimer_new(base, timer1, event_self_cbarg());
    if (!evl)
    {
        std::cout << "evtimer_new failed" << std::endl;
    }
    
    evtimer_add(evl, &t1);

    // 持久事件
    static timeval t2;
    t2.tv_sec = 1;
    t2.tv_usec = 200000;
    event* ev2 = event_new(base, -1, EV_PERSIST, timer2, event_self_cbarg());
    evtimer_add(ev2, &t2);

    // 超时优化性能优化，默认event用二叉堆存储O （log(n)）
    // 优化双向队列，插入O（1）
    event* ev3 = event_new(base, -1, EV_PERSIST, timer3, event_self_cbarg());
    static timeval tv_in = {3, 0};
    const timeval *t3;
    t3 = event_base_init_common_timeout(base, &tv_in);
    evtimer_add(ev3, t3);


    event_base_dispatch(base);
    event_base_free(base);
}