#include <iostream>
#include <event2/event.h>
#include <signal.h>

static void ctrl_c(int sock, short which, void* arg)
{
    std::cout << __func__ << std::endl;
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
    
    // 事件主循环
    event_base_dispatch(base);
    event_free(csig);
    event_base_free(base);    

    return 0;
}