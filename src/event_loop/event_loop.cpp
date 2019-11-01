#include <iostream>
#include <event2/event.h>
#include <signal.h>

bool isexit = false;

static void ctrl_c(int sock, short which, void* arg)
{
    std::cout << __func__ << std::endl;
    // isexit = true;  // main 中while使用

    event_base* base = (event_base*)arg;
    //event_base_loopbreak(base); // 执行完当前处理的事件就退出

    timeval t = {3, 0};
    event_base_loopexit(base, &t); // 运行完所有的活动事件后再退出，事件循环没有运行时，也要等运行一次再退出
} 

static void kill(int sock, short which, void* arg)
{
    std::cout << __func__ << std::endl;

    event *ev = (event*)arg;

    // 如果处于非待决状态, 再次添加
    if (!evsignal_pending(ev, NULL))
    {
        event_del(ev);
        event_add(ev, NULL);
    }


    
}

int main(int argc, char* atgv[])
{
    std::cout << "test_event_signal" << std::endl;
    event_base* base = event_base_new();

    // 添加ctrl+c
    // evsignal_new 隐藏的状态 EV_SIG | EV_PERSIST
    event* csig = evsignal_new(base, SIGINT, ctrl_c, base);
    if (!csig)
    {
        std::cerr << "SIGINT evsignal_new failed! " << std::endl;
        return -1;
    }

    // 添加事件到pending
    if (event_add(csig, 0) != 0)
    {
        std::cerr << "SIGINT event_add failed! " << std::endl;
        return -1;
    }

    // kill 信号
    // 非持久事件，只进入一次，event_self_cbarg()传递当前的event
    event* ksig = event_new(base, SIGTERM, EV_SIGNAL, kill, event_self_cbarg());
        if (!csig)
    {
        std::cerr << "SIGTERM evsignal_new failed! " << std::endl;
        return -1;
    }
    // 添加事件到pending
    if (event_add(ksig, 0) != 0)
    {
        std::cerr << "SIGTERM event_add kill failed! " << std::endl;
        return -1;
    }
    
   
    //event_base_dispatch(base);  // 事件主循环，阻塞
    //event_base_loop(base, EVLOOP_ONCE); // 等待一个事件运行，直到没有事件就退出
    //event_base_loop(base, EVLOOP_NONBLOCK); // 有事件就处理，无事件就退出
    /*while (!isexit)
    {
        event_base_loop(base, EVLOOP_NONBLOCK);
    }*/

    
    event_base_loop(base, EVLOOP_NO_EXIT_ON_EMPTY); // 没有注册事件也不返回，用于事件后期多线程添加
    

    event_free(csig);
    event_base_free(base);    

    return 0;
}