//
// Created by 31613 on 2025/10/26.
//

#include <math.h>

#include "network.h"
#include "lwip/sockets.h"
#include "fatfs.h"
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
uint32_t nowFileSize = 0;
static const char b64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

extern uint16_t SavedFileNum;
extern osSemaphoreId networkFiniSemHandle;

static int base64_encode(const uint8_t *in, size_t in_len, char *out, size_t out_size)
{
    size_t needed = 4 * ((in_len + 2) / 3);
    if (out_size < needed) return -1;

    size_t i = 0, o = 0;
    while (i + 2 < in_len) {
        uint32_t triple = (in[i] << 16) | (in[i+1] << 8) | in[i+2];
        out[o++] = b64_table[(triple >> 18) & 0x3F];
        out[o++] = b64_table[(triple >> 12) & 0x3F];
        out[o++] = b64_table[(triple >> 6)  & 0x3F];
        out[o++] = b64_table[triple & 0x3F];
        i += 3;
    }
    // process 1 or 2 remaining bytes
    if (i < in_len) {
        uint8_t a = in[i];
        uint8_t b = (i + 1 < in_len) ? in[i+1] : 0;
        uint32_t triple = (a << 16) | (b << 8);

        out[o++] = b64_table[(triple >> 18) & 0x3F];
        out[o++] = b64_table[(triple >> 12) & 0x3F];
        if (i + 1 < in_len) {
            out[o++] = b64_table[(triple >> 6) & 0x3F];
            out[o++] = '=';
        } else {
            out[o++] = '=';
            out[o++] = '=';
        }
    }
    return (int)o;
}

static int base64_decode(const char *in, size_t in_len, uint8_t *out, size_t out_size)
{
    static int b64_rev_table[256];
    static int table_init = 0;

    if (!table_init) {
        int i;
        for (i = 0; i < 256; i++) {
            b64_rev_table[i] = -1;
        }
        for (i = 0; i < 64; i++) {
            b64_rev_table[(unsigned char)b64_table[i]] = i;
        }
        table_init = 1;
    }

    if (!in || !out) {
        return -1;
    }
    if (in_len % 4 != 0) {
        return -1;
    }

    size_t out_len = (in_len / 4) * 3;
    if (in_len > 0 && in[in_len - 1] == '=') {
        out_len--;
    }
    if (in_len > 1 && in[in_len - 2] == '=') {
        out_len--;
    }
    if (out_size < out_len) {
        return -1;
    }

    size_t i = 0, o = 0;
    while (i < in_len) {
        int v1 = b64_rev_table[(unsigned char)in[i++]];
        int v2 = b64_rev_table[(unsigned char)in[i++]];
        int v3 = b64_rev_table[(unsigned char)in[i++]];
        int v4 = b64_rev_table[(unsigned char)in[i++]];

        if (v1 < 0 || v2 < 0) {
            return -1;
        }

        out[o++] = (uint8_t)((v1 << 2) | (v2 >> 4));

        if (in[i - 2] != '=') {
            if (v3 < 0) return -1;
            out[o++] = (uint8_t)(((v2 & 0x0F) << 4) | (v3 >> 2));
        } else {
            if (in[i - 1] != '=') return -1;
            break; // decode over
        }

        if (in[i - 1] != '=') {
            if (v4 < 0) return -1;
            out[o++] = (uint8_t)(((v3 & 0x03) << 6) | v4);
        } else {
            break; // decode over
        }
    }

    if (o != out_len) {
        return -1;
    }
    return (int)o;
}

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

static int send_all(int sock, const void *buf, size_t len)
{
    const uint8_t *p = (const uint8_t *)buf;
    size_t sent = 0;
    while (sent < len) {
        int n = send(sock, p + sent, (int)(len - sent), 0);
        if (n <= 0) {
            return -1;
        }
        sent += (size_t)n;
    }
    return 0;
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

int send_file_base64_over_socket(int sock,
                                 const char *filename,
                                 char *request_buffer,
                                 size_t request_buffer_size)
{
    if (request_buffer_size < OUTBUF_SIZE) return -1; // 要求最小 buffer

    UINT br;

    retSD = f_open(&SDFile, filename, FA_READ);
    if (retSD != FR_OK) {
        printf("f_open failed: %d\r\n", retSD);
        return -2;
    }

    uint8_t data_in[READ_CHUNK_SIZE];
    char *encoded_out = request_buffer;
    size_t encoded_out_max = request_buffer_size;

    for (;;) {
        retSD = f_read(&SDFile, data_in, READ_CHUNK_SIZE, &br); // br = 实际读到的 bytes
        if (retSD != FR_OK) {
            printf("f_read failed: %d\r\n", retSD);
            f_close(&SDFile);
            return -4;
        }
        if (br == 0) {
            // EOF
            break;
        }

        int encoded_len = base64_encode(data_in, (size_t)br, encoded_out, encoded_out_max);
        if (encoded_len < 0) {
            printf("base64_encode: output buffer too small\r\n");
            f_close(&SDFile);
            return -5;
        }

        if (send_all(sock, encoded_out, (size_t)encoded_len) != 0) {
            printf("send_all failed\r\n");
            f_close(&SDFile);
            return -6;
        }

        // if want to send newline after each chunk
        // const char nl = '\n';
        // send_all(sock, &nl, 1);

        // this means we have reached EOF
        if (br < READ_CHUNK_SIZE) {
            // already reached EOF
            // let next read fount that and exit
            continue;
        }
    }
    printf("File sent successfully\r\n");
    f_close(&SDFile);
    return 0;
}

void doSendAudioToZhiPuServer()
{
    ip_addr_t ipAddr;
    ip_addr_t *dnsServerAddr;
    err_t err;
    char fileName[40];

    printf("post Request\r\n");

    dns_init();

    dnsServerAddr = (ip_addr_t *) dns_getserver(0);

    printf("dns Server ip: %s\r\n", dnsServerAddr ? ipaddr_ntoa(dnsServerAddr) : "NULL");

    if (dnsServerAddr == NULL || dnsServerAddr->addr == 0) {
        dnsServerAddr = (ip_addr_t *) mem_malloc(sizeof(ip_addr_t));
        dnsServerAddr->addr = ipaddr_addr("192.168.1.1");
        dns_setserver(0, dnsServerAddr);
        mem_free(dnsServerAddr);
        dnsServerAddr = (ip_addr_t *) dns_getserver(0);
        printf("new dns Server ip: %s\r\n", dnsServerAddr ? ipaddr_ntoa(dnsServerAddr) : "NULL");
    }

    dnsFinished = 0;
    err = dns_gethostbyname("open.bigmodel.cn", &ipAddr, myDNSFountCallback, NULL);

    if (err == ERR_INPROGRESS) {
        while (!dnsFinished) {
            osDelay(10);
        }
        dns_gethostbyname("open.bigmodel.cn", &ipAddr, myDNSFountCallback, NULL);
    } else if (err == ERR_OK) {
        printf("DNS cached!!!!\r\n");
    } else {
        printf("DNS error: %d\r\n", err);
        return;
    }

    printf("Resolved IP: %s\r\n", ipaddr_ntoa(&ipAddr));

    const char *path = "/api/paas/v4/chat/completions";
    const char *json_body_part1 = "{\"model\":\"glm-4-voice\",\"messages\":[{\"role\":\"user\",\"content\":[{\"type\":\"text\",\"text\":\"你好，这是我的语音输入测试，请慢速复述一遍\"},{\"type\":\"input_audio\",\"input_audio\":{\"data\":\"";
    const char *json_body_part2 = "\",\"format\":\"wav\"}}]}]}";
    size_t body_length = strlen(json_body_part1) + strlen(json_body_part2) + 4 * (int) ceil((double) nowFileSize / 3);

    char request_buffer[512];
    int header_len = snprintf(request_buffer, sizeof(request_buffer),
                              "POST %s HTTP/1.1\r\n"
                              "Host: open.bigmodel.cn\r\n"
                              "User-Agent: STM32F746-AISpeaker(Created by ZSX,LSP)\r\n"
                              "Content-Type: application/json\r\n"
                              "Authorization: Bearer 09ccf85f65514088a62da2af2744b0b2.TBqQ09VrKCxw0efd\r\n"
                              "Content-Length: %u\r\n"
                              "Connection: keep-alive\r\n"
                              "\r\n",
                              path, body_length);

    printf("Request Header:\r\n%s", request_buffer);
    if (header_len < 0 || header_len >= (int) sizeof(request_buffer)) {
        printf("wrong header length\r\n");
        return;
    }

    int sock = connect_with_timeout(ipaddr_ntoa(&ipAddr), 80, 5000);

    send_all(sock, request_buffer, header_len);


    // send body part1
    send_all(sock, json_body_part1, strlen(json_body_part1));

    // send audio data in base64
    snprintf(fileName, 40, "/Audio/RECORDED%d.WAV", SavedFileNum);
    printf("now file name: %s", fileName);

    send_file_base64_over_socket(sock, fileName, request_buffer, sizeof(request_buffer));

    // send body part2
    send_all(sock, json_body_part2, strlen(json_body_part2));

    char buf[1024];
    ssize_t r;
    while ((r = recv(sock, buf, sizeof(buf)-1, 0)) > 0) {
        buf[r] = '\0';
        printf("%s", buf);
    }
    closesocket(sock);

    osSemaphoreRelease(networkFiniSemHandle);
}

void StartMyNetworkTask(void const * argument)
{
    taskParameters_t* tempTaskParameters = (taskParameters_t*)argument;
    switch (tempTaskParameters->taskType) {
        case 0:
            doPostRequest();
            break;
        case 1:
            doSendAudioToZhiPuServer();
            break;
        default:
            break;
    }
    osThreadTerminate(myNetworkTaskHandle);
}

void startFileSending(void)
{
    taskParameters.taskType = 1;

    osThreadDef(myNetworkTask, StartMyNetworkTask, osPriorityNormal, 0, 1024);
    myNetworkTaskHandle = osThreadCreate(osThread(myNetworkTask), &taskParameters);
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
