//
// Created by 31613 on 2025/10/26.
//

#include "network.h"
#include "lwip/sockets.h"
#include "lwip.h"
#include "tcp.h"
#include "dns.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t taskType;
} taskParameters_t;

osThreadId myNetworkTaskHandle;
static taskParameters_t taskParameters;
__IO uint8_t dnsFinished = 0;

void myDNSFountCallback(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    printf("DNS finished!!!!\r\n");
    printf("remote host name: %s\r\n", name);
    printf("remote host ip: %s\r\n", ipaddr ? ipaddr_ntoa(ipaddr) : "NULL");
    dnsFinished = 1;
}

int connect_with_timeout(const char *ip, uint16_t port, int timeout_ms)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("socket err %d\n", errno);
        return -1;
    }

    /* 设置非阻塞 */
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0) flags = 0;
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_aton(ip, &server_addr.sin_addr) == 0) {
        printf("inet_aton failed\n");
        close(sock);
        return -1;
    }

    int ret = connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret == 0) {
        /* 立刻连接成功（极少数情况） */
        /* 恢复阻塞模式（可选）*/
        fcntl(sock, F_SETFL, flags);
        return sock;
    } else if (ret < 0) {
        if (errno != EINPROGRESS) {
            printf("connect immediate error: errno=%d\n", errno);
            close(sock);
            return -1;
        }
        /* EINPROGRESS: 正常，继续用 select 等待完成 */
    }

    /* 使用 select 等待写事件（表示 connect 完成或失败）*/
    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(sock, &wfds);
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    ret = select(sock + 1, NULL, &wfds, NULL, &tv);
    if (ret == 0) {
        /* 超时 */
        printf("connect timeout after %d ms\n", timeout_ms);
        close(sock);
        return -1;
    } else if (ret < 0) {
        printf("select error errno=%d\n", errno);
        close(sock);
        return -1;
    }

    /* 检查 socket 上的错误码 */
    int err = 0;
    socklen_t len = sizeof(err);
    if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, &len) < 0) {
        printf("getsockopt failed errno=%d\n", errno);
        close(sock);
        return -1;
    }
    if (err != 0) {
        printf("connect failed after select, so_error=%d\n", err);
        close(sock);
        return -1;
    }

    /* 连接成功：恢复之前的 blocking 标志（可选）*/
    fcntl(sock, F_SETFL, flags);

    return sock; /* 返回已连接的 socket 描述符 */
}

void doPostRequest()
{
    ip_addr_t ipAddr;
    ip_addr_t *dnsServerAddr;
    err_t err;

    printf("post Request\r\n");

    dns_init();

    dnsServerAddr = (ip_addr_t *)dns_getserver(0);

    printf("dns Server ip: %s\r\n", dnsServerAddr ? ipaddr_ntoa(dnsServerAddr) : "NULL");
    if (dnsServerAddr == NULL) {
        dnsServerAddr = (ip_addr_t *)mem_malloc(sizeof(ip_addr_t));
        dnsServerAddr->addr = ipaddr_addr("192.168.1.1");
        dns_setserver(0, dnsServerAddr);
        mem_free(dnsServerAddr);
    }

    dnsFinished = 0;
    err = dns_gethostbyname("dxxyzhxg.bjtu.edu.cn", &ipAddr, myDNSFountCallback, NULL);

    if (err == ERR_INPROGRESS) {
        while (!dnsFinished) {
            osDelay(10);
        }
        dns_gethostbyname("dxxyzhxg.bjtu.edu.cn", &ipAddr, myDNSFountCallback, NULL);
    } else if (err == ERR_OK) {
        printf("DNS cached!!!!\r\n");
    } else {
        printf("DNS error: %d\r\n", err);
        return ;
    }

    printf("Resolved IP: %s\r\n", ipaddr_ntoa(&ipAddr));

    uint16_t server_port = 8087;
    const char* path = "/user/account/login";
    const char *json_body = "{\"userid\":\"8504\",\"password\":\"zhsz8504\"}";
    size_t body_length = strlen(json_body);
    char request_buffer[512];
    int header_len = snprintf(request_buffer, sizeof(request_buffer),
        "POST %s HTTP/1.1\r\n"
        "Host: dxxyzhxg.bjtu.edu.cn:%d\r\n"
        "User-Agent: STM32F746(Created by ZSX,LSP)\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %u\r\n"
        "Connection: keep-alive\r\n"
        "\r\n",
        path, server_port, body_length);
    printf("Request Header:\r\n%s", request_buffer);
    if (header_len < 0 || header_len >= (int)sizeof(request_buffer)) {
        printf("wrong header length\r\n");
        return;
    }
    int sock = connect_with_timeout("59.64.5.160", server_port, 5000);
    size_t sent = 0;
    while (sent < (size_t)header_len) {
        ssize_t n = send(sock, request_buffer + sent, header_len - sent, 0);
        if (n <= 0) {
            printf("send header failed\r\n");
            closesocket(sock);
            return;
        }
        sent += n;
    }

    sent = 0;
    while (sent < body_length) {
        ssize_t n = send(sock, json_body + sent, body_length - sent, 0);
        if (n <= 0) {
            closesocket(sock);
            return;
        }
        sent += n;
    }

    char buf[1024];
    ssize_t r;
    while ((r = recv(sock, buf, sizeof(buf)-1, 0)) > 0) {
        buf[r] = '\0';
        printf("%s", buf);
    }
    closesocket(sock);
}

void StartMyNetworkTask(void const * argument)
{
    taskParameters_t* tempTaskParameters = (taskParameters_t*)argument;
    switch (tempTaskParameters->taskType) {
        case 0:
            doPostRequest();
            break;
        default:
            break;
    }
    osThreadTerminate(myNetworkTaskHandle);
}

void startPostRequest(void)
{
    taskParameters.taskType = 0;

    osThreadDef(myNetworkTask, StartMyNetworkTask, osPriorityNormal, 0, 1024);
    myNetworkTaskHandle = osThreadCreate(osThread(myNetworkTask), &taskParameters);
}

#ifdef __cplusplus
};
#endif
