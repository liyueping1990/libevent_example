#include <iostream>
#include <event2/event.h>
#include <thread>
#include <chrono>

#ifndef _WIN32
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

static timeval t1 = {1, 0};
static void read_file(evutil_socket_t fd, short event, void* arg)
{
    std::cout << __func__ << std::endl;  
    char buf[1024] = {0};
    int len = read(fd, buf, sizeof(buf)-1);
    if (len > 0)
    {
        std::cout << buf << std::endl;
    }      
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

    event_config* conf = event_config_new();

    // 设置支持文件描述符
    event_config_require_features(conf, EV_FEATURE_FDS);
    event_base* base = event_base_new_with_config(conf);
    event_config_free(conf);
    if (!base)
    {
        std::cout << "event_base_new_with_config failed" << std::endl;
        return -1;
    }

    // 打开文件只读，非阻塞
    int sock = open("/var/log/auth.log", O_RDONLY | O_NONBLOCK, 0);
    if (sock <= 0)
    {
        std::cout << "open failed" << std::endl;
        return -1;
    }
    // 文件指针移动到结尾处
    lseek(sock, 0, SEEK_END);

    // 监听文件数据
    event* fev = event_new(base, sock, EV_READ | EV_PERSIST, read_file, event_self_cbarg());
    event_add(fev, NULL);
    

    event_base_dispatch(base);
    event_base_free(base);
}