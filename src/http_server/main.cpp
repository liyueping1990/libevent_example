#include <event2/event.h>
#include <event2/listener.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include <event2/buffer.h>
#include <event2/http_struct.h>

#include <string.h>
#ifndef _WIN32
#include <signal.h>
#endif
#include <iostream>
#include <string>
using namespace std;
#define WEBROOT "."
#define DEFAULTINDEX "index.html"

void http_cb(struct evhttp_request *request, void *arg)
{
    cout << "http_cb" << endl;
    //1 ��ȡ�������������Ϣ
    //uri
    const char *uri = evhttp_request_get_uri(request);
    cout << "uri:" << uri << endl;

    //�������� GET POST
    string cmdtype;
    switch (evhttp_request_get_command(request))
    {
    case EVHTTP_REQ_GET:
        cmdtype = "GET";
        break;
    case EVHTTP_REQ_POST:
        cmdtype = "POST";
        break;
    }
    cout << "cmdtype:" << cmdtype << endl;

    // ��Ϣ��ͷ
    evkeyvalq *headers = evhttp_request_get_input_headers(request);
    cout << "====== headers ======" << endl;
    for (evkeyval *p = headers->tqh_first; p != NULL; p = p->next.tqe_next)
    {
        cout << p->key << ":" << p->value << endl;
    }

    // �������� (GETΪ�գ�POST�б���Ϣ  )
    evbuffer *inbuf = evhttp_request_get_input_buffer(request);
    char buf[1024] = { 0 };
    cout << "======= Input data ======" << endl;
    while (evbuffer_get_length(inbuf))
    {
        int n = evbuffer_remove(inbuf, buf, sizeof(buf) - 1);
        if (n > 0)
        {
            buf[n] = '\0';
            cout << buf << endl;
        }
    }

    //2 �ظ������
    //״̬�� ��Ϣ��ͷ ��Ӧ���� HTTP_NOTFOUND HTTP_INTERNAL

    //  ������������ļ� uri
    //  ���ø�Ŀ¼ WEBROOT
    // windows ��Ҫ�ӣ���Ŀ����=> C/C++=>Ԥ��������
    // _CRT_SECURE_NO_WARNINGS
    string filepath = WEBROOT;
    filepath += uri;
    if (strcmp(uri, "/") == 0)
    {
        //Ĭ�ϼ�����ҳ�ļ�
        filepath += DEFAULTINDEX;
    }
    //��Ϣ��ͷ

    //��ȡhtml�ļ���������
    filepath = "E:\\Code\\test\\libevent_example\\src\\http_server\\index.html";
    //filepath = "E:\\Code\\test\\libevent_example\\src\\http_server\\test.jpg";
    FILE *fp = fopen(filepath.c_str(), "rb");
    if (!fp)
    {
        evbuffer *outbuf = evhttp_request_get_output_buffer(request);

        const char* reason = "open file failed";
        evbuffer_add(outbuf, reason, strlen(reason));
        evhttp_send_reply(request, HTTP_NOTFOUND, reason, outbuf);
        return;
    }
    evbuffer *outbuf = evhttp_request_get_output_buffer(request);
    for (;;)
    {
        int len = fread(buf, 1, sizeof(buf), fp);
        if (len <= 0)break;
        evbuffer_add(outbuf, buf, len);
    }
    fclose(fp);

    evkeyvalq* outhead = evhttp_request_get_output_headers(request);
    //request->output_headers;
    evhttp_add_header(outhead, "Content-Type", "image/jpg");
    evhttp_send_reply(request, HTTP_OK, "", outbuf);
}


int main()
{
#ifdef _WIN32
    //��ʼ��socket��
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#else
    //���Թܵ��źţ��������ݸ��ѹرյ�socket
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        return 1;
#endif

    std::cout << "test server!\n";
    //����libevent��������
    event_base * base = event_base_new();
    if (base)
    {
        cout << "event_base_new success!" << endl;
    }

    // http  ������
    //1	����evhttp������
    evhttp *evh = evhttp_new(base);

    //2  �󶨶˿ں�IP
    if (evhttp_bind_socket(evh, "0.0.0.0", 8080) != 0)
    {
        cout << "evhttp_bind_socket failed!" << endl;
    }

    //3   �趨�ص�����
    evhttp_set_gencb(evh, http_cb, 0);

    //�¼��ַ�����
    if (base)
        event_base_dispatch(base);
    if (base)
        event_base_free(base);
    if (evh)
        evhttp_free(evh);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
