//
// Created by 31613 on 2025/10/26.
//

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
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
osThreadId networkReceiveTaskHandle;
static taskParameters_t taskParameters;
__IO uint8_t dnsFinished = 0;
__IO bool gNetworkFinished = false;
uint8_t next[50]; // to hold the next info of pattern string
uint32_t nowFileSize = 0;
uint16_t AIRepliedFileNum = 0;
ALIGN_32BYTES(uint8_t streamDecodeBuffer[BASE_STREAM_BUFFER_SIZE]);
ALIGN_32BYTES(uint8_t streamBuffer[STREAM_BUFFER_SIZE]);

static const char b64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char b64_inverse_table[] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0x3e, 0xff, 0xff, 0xff, 0x3f, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0xff, 0xff,
    0xff, 0xfe, 0xff, 0xff, 0xff, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
    0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1a, 0x1b, 0x1c,
    0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
    0x31, 0x32, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

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

/**
 * decode a base64 string in place
 * @param buffer
 * @param encoded_len
 * @return
 */
static size_t base64_decode(unsigned char *buffer, const size_t encoded_len)
{
    size_t j = 0;
    unsigned char val[4];
    for (size_t i = 0; i < encoded_len; i += 4) {
        int is_invalid_block = 0;
        int pad_count = 0;
        for (int k = 0; k < 4; k++) {
            if (i + k >= encoded_len) {
                is_invalid_block = 1;
                break;
            }
            const unsigned char current_char = buffer[i + k];
            val[k] = b64_inverse_table[current_char];
            if (val[k] == BASE64_INVALID) {
                printf("meet a invalid character, current char is: %c", current_char);
                is_invalid_block = 1;
            }
            if (val[k] == BASE64_PAD_VAL) {
                pad_count++;
            }
        }
        if (is_invalid_block) {
            buffer[j++] = 0;
            buffer[j++] = 0;
            buffer[j++] = 0;
            continue;
        }
        buffer[j++] = (val[0] << 2) | (val[1] >> 4);
        if (pad_count < 2) {
            buffer[j++] = (val[1] << 4) | (val[2] >> 2);
        }
        if (pad_count < 1) {
            buffer[j++] = (val[2] << 6) | val[3];
        }
    }
    return j;
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
        fcntl(sock, F_SETFL, flags);
        return sock;
    } else if (ret < 0) {
        if (errno != EINPROGRESS) {
            printf("connect immediate error: errno=%d\n", errno);
            close(sock);
            return -1;
        }
    }
    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(sock, &wfds);
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    ret = select(sock + 1, NULL, &wfds, NULL, &tv);
    if (ret == 0) {
        printf("connect timeout after %d ms\n", timeout_ms);
        close(sock);
        return -1;
    } else if (ret < 0) {
        printf("select error errno=%d\n", errno);
        close(sock);
        return -1;
    }

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

    fcntl(sock, F_SETFL, flags);

    return sock;
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
    const char* path = "/user/info-maintenance/list-student-xinxi-v2";
    const char *json_body = "{\"size\":2000,\"page\":1,\"classNotEmpty\":2,\"is_school\":1,\"columnStringList\":[\"xm\",\"xh\",\"xb\",\"zzmm\",\"xjzt\",\"xjlx\",\"xslx\",\"xy\",\"grade_now\",\"zy\",\"zrb\",\"xllx\",\"column995\",\"column978\"],\"condition_new\":[],\"orderby\":{}}";
    size_t body_length = strlen(json_body);
    char request_buffer[1200];
    int header_len = snprintf(request_buffer, sizeof(request_buffer),
        "POST %s HTTP/1.1\r\n"
        "Host: 59.64.5.160:%d\r\n"
        "User-Agent: STM32F746(Created by ZSX,LSP)\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %u\r\n"
        "Connection: close\r\n"
        "Accept-Encoding: br\r\n"
        "token: eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiIsImp0aSI6ImxhYi1qd3QifQ.eyJpc3MiOiJodHRwOlwvXC9keHh5Y3kuYmp0dS5lZHUuY24iLCJhdWQiOiJodHRwOlwvXC9keHh5Y3kuYmp0dS5lZHUuY25cL2FkbWluIiwianRpIjoibGFiLWp3dCIsImlhdCI6MTc2MTgwNDk0OSwiZXhwIjoxNzY0Mzk2OTQ5LCJwYXlsb2FkIjp7ImlkIjoiODUwNCIsImlzX3N0dWRlbnQiOjAsImlzX3NjaG9vbCI6MSwiaWRlbnRpdHkiOlsyXSwiY29sbGVnZV9pZCI6MSwicm9sZV9pZCI6OTAxLCJyb2xlX2lkX2xpc3QiOls5MDEsNCwxMSwxMDQzLDEsOTExLDJdLCJncm91cF9pZCI6MzkwNiwiZ3JvdXBfaWRfbGlzdCI6WzQ5NjcxLDQ5NjY4LDQ5NjY1LDQ5NjU3LDQ5NjU2LDQ5NjU1LDQ5NjU0LDQ5NjUzLDQ5NjUyLDQ5NjA1LDQ5NTUwLDQ5NTQzLDQ5NDE4LDE1MzMwLDE1MjIzLDE1MjEyLDE1MjA4LDE1MTg0LDQ1OTEsNDU0MiwzOTA2LDI0MzldLCJmZWF0dXJlX2lkX2xpc3QiOls2LDIzLDcsOCwxMiwyMiwxMCwyMCwyMSwzLDE5LDE4LDE2LDE1LDE0LDEzLDExLDUsMiwxXSwicGFzc3dvcmQiOiIxMjZhNDhkOTMzMzA2NTcyNWFhZGQ2ZDdlMzNlM2U2NSJ9fQ.xwf6bIVbvKRcDU0wFqCgadRvGdtHx5wwJFXoem01Mbo\r\n"
        "\r\n",
        path, server_port, body_length);
    printf("Request Header:\r\n%s", request_buffer);
    if (header_len < 0 || header_len >= (int)sizeof(request_buffer)) {
        printf("wrong header length\r\n");
        return;
    }
    int sock = connect_with_timeout("59.64.5.160", server_port, 5000);
    if (sock < 0) {
        printf("socket create failed....\r\n");
        return ;
    }
    u_long mode = 0; // 0 -> blocking, 1 -> non-blocking
    ioctlsocket(sock, FIONBIO, &mode);
    struct timeval tv;
    tv.tv_sec = 10; tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
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

    FIL testFile;

    char buf[1024];
    ssize_t r;
    ssize_t receivedTimes = 0;
    uint32_t totalBytes = 0;
    retSD = f_open(&testFile, "postResponse.wav", FA_CREATE_ALWAYS | FA_WRITE);
    if (retSD != FR_OK) {
        printf("open Failed, ret is : %d\r\n", retSD);
        closesocket(sock);
        return ;
    }
    while ((r = recv(sock, buf, sizeof(buf)-1, 0)) > 0) {
        receivedTimes++;
        totalBytes += r;
        printf("received %d bytes, time is: %d\r\n", r, receivedTimes);
        printf("total received bytes: %lu\r\n", totalBytes);
        buf[r] = '\0';
        f_printf(&testFile, "%s", buf);
        // osDelay(500);
        // if (totalBytes > 40000)
        //     printf("%s", buf);
    }
    f_close(&testFile);

    osDelay(1000);

    closesocket(sock);
}

/**
 * get next function for KMP algorithm
 * @param s
 * @param len
 */
void getNext(const char* s, const size_t len){
    next[0] = 0;
    int k = 0; //k = next[0]
    for(int i = 1; i < len; i++){
        while (k > 0 && s[i] != s[k])
            k = next[k - 1]; //k = next[k-1]
        if(s[i] == s[k])
            k++;
        next[i] = k; //next[j+1] = k+1 | next[j+1] = 0
    }
}

/**
 * KMP algorithm to find substring S in T
 * @param T
 * @param lenT
 * @param S
 * @param lenS
 * @return pos of S first appear in T, -1 if not found
 */
int kmp(const char *T, const size_t lenT, const char* S, const size_t lenS){
    for (int i = 0, j = 0; i < lenT; i++) {
        while (j > 0 && T[i] != S[j])
            j = next[j - 1];
        if (T[i] == S[j])
            j++;
        if(j == lenS)
            return i - lenS + 1;
    }
    return -1;
}

int send_file_base64_over_socket(const int sock,
                                 const char *filename,
                                 char *request_buffer,
                                 const size_t request_buffer_size)
{
    if (request_buffer_size < OUTBUF_SIZE) return -1;

    FIL sendFile;
    UINT br;

    retSD = f_open(&sendFile, filename, FA_READ);
    if (retSD != FR_OK) {
        f_close(&sendFile);
        printf("f_open failed: %d\r\n", retSD);
        return -2;
    }

    uint8_t data_in[READ_CHUNK_SIZE];
    char *encoded_out = request_buffer;
    size_t encoded_out_max = request_buffer_size;

    for (;;) {
        retSD = f_read(&sendFile, data_in, READ_CHUNK_SIZE, &br);
        if (retSD != FR_OK) {
            printf("f_read failed: %d\r\n", retSD);
            f_close(&sendFile);
            return -4;
        }
        if (br == 0) {
            // EOF
            break;
        }

        int encoded_len = base64_encode(data_in, (size_t)br, encoded_out, encoded_out_max);
        if (encoded_len < 0) {
            printf("base64_encode: output buffer too small\r\n");
            f_close(&sendFile);
            return -5;
        }

        if (send_all(sock, encoded_out, (size_t)encoded_len) != 0) {
            printf("send_all failed\r\n");
            f_close(&sendFile);
            return -6;
        }

        // want to send newline after each chunk
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
    retSD = f_close(&sendFile);
    if (retSD != FR_OK) {
        printf("close send file failed: ret : %d\r\n", retSD);
        return 1;
    }
    return 0;
}

int8_t processResponse(const int sock)
{
    FIL testFile;
    UINT bW;
    char fileName[40];
    AIRepliedFileNum++;
    snprintf(fileName, 40, "/AIReplied/Audio%d.wav", AIRepliedFileNum);
    ssize_t receivedFromSocket = recv(sock, streamBuffer, STREAM_BUFFER_SIZE - STREAM_BUFFER_MARGIN, 0);
    if (receivedFromSocket < 0) {
        printf("received from socket failed!\r\n");
        return 3;
    }
    while (receivedFromSocket < 1024) {
        const ssize_t r = recv(sock, streamBuffer + receivedFromSocket, STREAM_BUFFER_SIZE -
            STREAM_BUFFER_MARGIN - receivedFromSocket, 0);
        if (r < 0) {
            printf("received from socket failed!\r\n");
            return 3;
        }
        receivedFromSocket += r;
    }
    SCB_CleanDCache_by_Addr((uint32_t *)&streamBuffer[0], STREAM_BUFFER_SIZE);
    getNext("HTTP/1.1 ", 9);
    int pos = kmp((const char*)streamBuffer, STREAM_BUFFER_SIZE - STREAM_BUFFER_MARGIN, "HTTP/1.1 ", 9);
    if (pos == -1) {
        printf("can not find HTTP response\r\n");
        return 4;
    }
    if (streamBuffer[pos + 9] != '2' && streamBuffer[pos + 10] != '0' && streamBuffer[pos + 11] != '0') {
        printf("http response is not 200 OK, instead, it is: ");
        for (uint8_t i = 0u; streamBuffer[pos + 9 + i] != '\r'; i++) {
            printf("%c", streamBuffer[pos + 9 + i]);
        }
        printf("\r\n");
        return 5;
    }
    getNext("\r\n\r\n", 4);
    pos = kmp((const char*)streamBuffer, STREAM_BUFFER_SIZE - STREAM_BUFFER_MARGIN, "\r\n\r\n", 4);
    if (pos == -1) {
        printf("can not find ending of HTTP header!\r\n");
        return 6;
    }
    pos += 4;
    uint32_t nowChunkSize = 0;
    for (uint8_t i = 0u; streamBuffer[pos + i] != '\r'; i++) {
        nowChunkSize *= 16;
        if (isdigit(streamBuffer[pos + i])) {
            nowChunkSize += streamBuffer[pos + i] - '0';
        } else if (isupper(streamBuffer[pos + i])) {
            nowChunkSize += streamBuffer[pos + i] - 'A' + 10;
        } else if (islower(streamBuffer[pos + i])) {
            nowChunkSize += streamBuffer[pos + i] - 'a' + 10;
        } else {
            printf("unknown chunksize letter!\r\n");
            return 7;
        }
    }
    printf("new chunk found!, size is: %lu\r\n", nowChunkSize);
    // todo: add a judgement logic to judge if the size of chunk is too small to contain all of the json
    getNext("\"finish_reason\":", 16);
    pos = kmp((const char*)streamBuffer, STREAM_BUFFER_SIZE - STREAM_BUFFER_MARGIN,
        "\"finish_reason\":", 16);
    if (pos < 0) {
        printf("can not find reason of finish\r\n");
        return 8;
    }
    if (streamBuffer[pos + 17] != 's' || streamBuffer[pos + 18] != 't' || streamBuffer[pos + 19] != 'o' ||
        streamBuffer[pos + 20] != 'p') {
        printf("reason of finish is not stop, instead, it is: ");
        for (uint8_t i = 17u; streamBuffer[pos + i] != '\"'; i++) {
            printf("%c", streamBuffer[pos + i]);
        }
        printf("\r\n");
        return 9;
    }
    getNext("\"audio\":{\"data\":\"", 17);
    pos = kmp((const char*)streamBuffer, STREAM_BUFFER_SIZE - STREAM_BUFFER_MARGIN,
        "\"audio\":{\"data\":\"", 17);
    if (pos < 0) {
        printf("can not find audio data\r\n");
        return 10;
    }
    pos += 17;
    retSD = f_open(&testFile, fileName, FA_CREATE_ALWAYS | FA_WRITE);
    if (retSD != FR_OK) {
        printf("open Failed, ret is : %d\r\n", retSD);
        return 1;
    }
    uint16_t decodeBufferPos = 0;
    uint8_t isFinished = 0;
    while (1) {
        for (uint32_t i = pos; i < receivedFromSocket; i++) {
            if (streamBuffer[i] == '\r') {
                if (receivedFromSocket - i < STREAM_BUFFER_MARGIN) {
                    // if we meet end of chunk in the tail of the stream block
                    // to ensure safe, we need to read more 8 byte to get full
                    // description of next chunk  -----------I'm a smart B
                    const ssize_t r = recv(sock, streamBuffer + receivedFromSocket, STREAM_BUFFER_MARGIN, 0);
                    if (r < 0) {
                        printf("received from socket failed!\r\n");
                        return 3;
                    }
                    SCB_CleanDCache_by_Addr((uint32_t *)&streamBuffer[0], STREAM_BUFFER_SIZE);
                    receivedFromSocket += r;
                }
                i += 2; // get through of the \r\n
                nowChunkSize = 0;
                for (; streamBuffer[i] != '\r'; i++) {
                    nowChunkSize *= 16;
                    if (isdigit(streamBuffer[i])) {
                        nowChunkSize += streamBuffer[i] - '0';
                    } else if (isupper(streamBuffer[i])) {
                        nowChunkSize += streamBuffer[i] - 'A' + 10;
                    } else if (islower(streamBuffer[i])) {
                        nowChunkSize += streamBuffer[i] - 'a' + 10;
                    } else {
                        printf("unknown chunk size letter!\r\n");
                        return 7;
                    }
                }
                printf("new chunk found!, size is: %lu\r\n", nowChunkSize);
                if (nowChunkSize == 0) {
                    isFinished = 1;
                    break;
                }
                i += 2; // get through of the \r\n
            }
            if (streamBuffer[i] == '\"') {
                // end of File block
                isFinished = 2;
                // move the rest data to head of buffer
                memcpy(streamBuffer, streamBuffer + i, receivedFromSocket - i);
                receivedFromSocket -= (ssize_t)i;
                while (1) {
                    const ssize_t r = recv(sock, streamBuffer + receivedFromSocket, STREAM_BUFFER_SIZE -
            STREAM_BUFFER_MARGIN - receivedFromSocket, 0);
                    if (r < 0) {
                        printf("received from socket failed!\r\n");
                        return 3;
                    }
                    SCB_CleanDCache_by_Addr((uint32_t *)&streamBuffer[0], STREAM_BUFFER_SIZE);
                    if (r == 0)
                        break;
                    receivedFromSocket += r;
                }
                break;
            }
            streamDecodeBuffer[decodeBufferPos++] = streamBuffer[i];
            if (decodeBufferPos == BASE_STREAM_BUFFER_SIZE) {
                SCB_CleanDCache_by_Addr((uint32_t *)&streamDecodeBuffer[0], BASE_STREAM_BUFFER_SIZE);
                decodeBufferPos = base64_decode(streamDecodeBuffer, decodeBufferPos);
                SCB_CleanDCache_by_Addr((uint32_t *)&streamDecodeBuffer[0], BASE_STREAM_BUFFER_SIZE);
                // printf("decode Finished!, decode result size is: %d\r\n", decodeBufferPos);
                f_write(&testFile, streamDecodeBuffer, 250, &bW);
                UINT allWrote = bW;
                // while (allWrote < decodeBufferPos) {
                //     f_write(&testFile, streamDecodeBuffer + allWrote, 250 - allWrote, &bW);
                //     allWrote += bW;
                // }
                f_write(&testFile, streamDecodeBuffer + 250, 250, &bW);
                allWrote += bW;
                f_write(&testFile, streamDecodeBuffer + 500, 250, &bW);
                allWrote += bW;
                // printf("decoded %d bytes data, wrote %d bytes data!\r\n", decodeBufferPos, allWrote);
                decodeBufferPos = 0;
            }
        }
        if (isFinished == 1 || isFinished == 2) {
            SCB_CleanDCache_by_Addr((uint32_t *)&streamDecodeBuffer[0], BASE_STREAM_BUFFER_SIZE);
            decodeBufferPos = base64_decode(streamDecodeBuffer, decodeBufferPos);
            SCB_CleanDCache_by_Addr((uint32_t *)&streamDecodeBuffer[0], BASE_STREAM_BUFFER_SIZE);
            // printf("decode Finished!, decode result size is: %d\r\n", decodeBufferPos);
            f_write(&testFile, streamDecodeBuffer, decodeBufferPos, &bW);
            UINT allWrote = bW;
            while (allWrote < decodeBufferPos) {
                f_write(&testFile, streamDecodeBuffer + allWrote, decodeBufferPos - allWrote, &bW);
                allWrote += bW;
            }
            // printf("decoded %d bytes data, wrote %d bytes data!\r\n", decodeBufferPos, allWrote);
            break;
        }
        receivedFromSocket = recv(sock, streamBuffer, STREAM_BUFFER_SIZE - STREAM_BUFFER_MARGIN, 0);
        if (receivedFromSocket < 0) {
            printf("received from socket failed!\r\n");
            return 3;
        }
        SCB_CleanDCache_by_Addr((uint32_t *)&streamBuffer[0], STREAM_BUFFER_SIZE);
        pos = 0;
    }
    retSD = f_close(&testFile);
    if (retSD != FR_OK) {
        printf("close file failed!!! ret: %d\r\n", retSD);
        return 12;
    }
    if (isFinished == 0) {
        printf("something wrong happened!\r\n");
        return 11;
    }
    if (isFinished == 1) {
        printf("chunks end without file ends!!!\r\n");
        return 13;
    }
    getNext("\"content\":", 10);
    pos = kmp((const char*)streamBuffer, receivedFromSocket, "\"content\":", 10);
    if (pos < 0) {
        printf("can not find content block");
    }
    for (uint16_t i = 0; i < (uint16_t)(receivedFromSocket - pos - 11); i++) {
        streamBuffer[i] = streamBuffer[i + pos + 11];
        if (streamBuffer[i + pos + 11] == '\"') {
            streamBuffer[i] = '\0';
            break;
        }
    }
    printf("content is : %s\r\n", (char*)streamBuffer);
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
    const char *json_body_part1 = "{\"model\":\"glm-4-voice\",\"messages\":[{\"role\":\"user\",\"content\":[{\"type\":\"input_audio\",\"input_audio\":{\"data\":\"";
    const char *json_body_part2 = "\",\"format\":\"wav\"}}]}]}";
    const size_t body_length = strlen(json_body_part1) + strlen(json_body_part2) + 4 * (int) ceil((double) nowFileSize / 3);

    char request_buffer[512];
    const int header_len = snprintf(request_buffer, sizeof(request_buffer),
                              "POST %s HTTP/1.1\r\n"
                              "Host: open.bigmodel.cn\r\n"
                              "User-Agent: STM32F746-AISpeaker(Created by ZSX,LSP)\r\n"
                              "Content-Type: application/json\r\n"
                              "Content-Length: %u\r\n"
                              "Connection: close\r\n"
                              "Accept-Encoding: br\r\n"
                              "Authorization: Bearer 09ccf85f65514088a62da2af2744b0b2.TBqQ09VrKCxw0efd\r\n"
                              "\r\n",
                              path, body_length);

    if (header_len < 0 || header_len >= (int) sizeof(request_buffer)) {
        printf("wrong header length\r\n");
        return;
    }

    const int sock = connect_with_timeout(ipaddr_ntoa(&ipAddr), 80, 3000);

    send_all(sock, request_buffer, header_len);

    // send body part1
    send_all(sock, json_body_part1, strlen(json_body_part1));

    // send audio data in base64
    snprintf(fileName, 40, "/Audio/RECORDED%d.WAV", SavedFileNum);
    printf("now file name: %s\r\n", fileName);

    send_file_base64_over_socket(sock, fileName, request_buffer, sizeof(request_buffer));

    // send body part2
    send_all(sock, json_body_part2, strlen(json_body_part2));

    err = processResponse(sock);
    if (err != 0) {
        printf("processResponse error: %d\r\n", err);
    }
    closesocket(sock);
    osSemaphoreRelease(networkFiniSemHandle);
}

void StartMyNetworkTask(void const * argument)
{
    const taskParameters_t* tempTaskParameters = (taskParameters_t*)argument;
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

    osThreadDef(myNetworkTask, StartMyNetworkTask, osPriorityNormal, 0, 4096);
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
