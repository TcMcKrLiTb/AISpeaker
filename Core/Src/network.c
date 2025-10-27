//
// Created by 31613 on 2025/10/26.
//

#include <stdbool.h>
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
        for (i = 0; i < 256; i++) b64_rev_table[i] = -1;
        for (i = 0; i < 64; i++) b64_rev_table[(unsigned char)b64_table[i]] = i;
        table_init = 1;
    }

    if (!in || !out || in_len % 4 != 0) return -1;

    size_t out_len = (in_len / 4) * 3;
    if (in_len > 0 && in[in_len - 1] == '=') out_len--;
    if (in_len > 1 && in[in_len - 2] == '=') out_len--;

    if (out_size < out_len) return -1;

    size_t i = 0, o = 0;
    while (i < in_len) {
        int v[4];
        v[0] = b64_rev_table[(unsigned char)in[i++]];
        v[1] = b64_rev_table[(unsigned char)in[i++]];
        v[2] = b64_rev_table[(unsigned char)in[i++]];
        v[3] = b64_rev_table[(unsigned char)in[i++]];

        if (v[0] < 0 || v[1] < 0) return -1;

        out[o++] = (uint8_t)((v[0] << 2) | (v[1] >> 4));
        if (v[2] >= 0) {
            out[o++] = (uint8_t)(((v[1] & 0x0F) << 4) | (v[2] >> 2));
        } else {
            break;
        }
        if (v[3] >= 0) {
            out[o++] = (uint8_t)(((v[2] & 0x03) << 6) | v[3]);
        } else {
            break;
        }
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

static char* buffer_find(char* buffer, int buf_len, const char* substr) {
    if (!substr || !buffer || buf_len <= 0) return NULL;
    int sub_len = strlen(substr);
    if (sub_len == 0 || sub_len > buf_len) return NULL;
    for (int i = 0; i <= buf_len - sub_len; ++i) {
        if (memcmp(buffer + i, substr, sub_len) == 0) return buffer + i;
    }
    return NULL;
}

int process_server_response(int sock, const char* audio_filename, char* content_buffer, size_t content_buf_size) {
    FIL audio_file; FRESULT fr; UINT bytes_written;
    char stream_buf[STREAM_BUFFER_SIZE];
    char b64_in_buf[BASE64_CHUNK_SIZE];
    uint8_t decoded_audio_buf[READ_CHUNK_SIZE];

    int data_in_buf = 0; long current_chunk_size = 0;
    int b64_idx = 0; int content_idx = 0;

    ParserState state = STATE_HTTP_HEADER;
    JsonParseState json_state = JSON_SEEK_FINISH_REASON;
    bool connection_alive = true;

    if (content_buffer && content_buf_size > 0) content_buffer[0] = '\0';

    fr = f_open(&audio_file, audio_filename, FA_CREATE_ALWAYS | FA_WRITE);
    if (fr != FR_OK) { return -1; }

    while (state != STATE_DONE && state != STATE_ERROR) {
        int bytes_consumed = 0;
        char* p_buf = stream_buf;

        // 步骤 1: 尝试用当前缓冲区的数据进行解析
        switch (state) {
            case STATE_HTTP_HEADER: {
                char* body_start = buffer_find(p_buf, data_in_buf, "\r\n\r\n");
                if (body_start) {
                    if (!buffer_find(p_buf, (body_start-p_buf), "HTTP/1.1 200 OK")) { state = STATE_ERROR; break; }
                    bytes_consumed = (body_start - p_buf) + 4;
                    state = STATE_CHUNK_SIZE;
                    printf("Debug: HTTP Header found, consumed %d bytes.\r\n", bytes_consumed);
                }
                break;
            }
            case STATE_CHUNK_SIZE: {
                char* eol = buffer_find(p_buf, data_in_buf, "\r\n");
                if (eol) {
                    *eol = '\0'; current_chunk_size = strtol(p_buf, NULL, 16); *eol = '\r';
                    bytes_consumed = (eol - p_buf) + 2;
                    state = (current_chunk_size > 0) ? STATE_CHUNK_DATA : STATE_DONE;
                    printf("Debug: Found chunk size: %ld, consumed %d bytes.\r\n", current_chunk_size, bytes_consumed);
                }
                break;
            }
            case STATE_CHUNK_DATA: {
                int data_to_process = (data_in_buf < current_chunk_size) ? data_in_buf : (int)current_chunk_size;
                char* data_end = p_buf + data_to_process;
                char* current_ptr = p_buf;

                // ... (JSON解析逻辑与之前类似，但现在在一个指针上操作) ...
                if (json_state == JSON_SEEK_FINISH_REASON) { if (buffer_find(current_ptr, data_end - current_ptr, "\"finish_reason\":\"stop\"")) { json_state = JSON_SEEK_AUDIO_DATA_KEY; } else if (buffer_find(current_ptr, data_end - current_ptr, "\"finish_reason\"")) { state = STATE_ERROR; break; }}
                if (json_state == JSON_SEEK_AUDIO_DATA_KEY) { char* key_pos = buffer_find(current_ptr, data_end - current_ptr, "\"data\":\""); if (key_pos) { current_ptr = key_pos + strlen("\"data\":\""); json_state = JSON_STREAM_AUDIO_DATA; }}
                if (json_state == JSON_STREAM_AUDIO_DATA) { while(current_ptr < data_end) { if(*current_ptr == '"') {json_state = JSON_SEEK_CONTENT_KEY; break;} b64_in_buf[b64_idx++]=*current_ptr++; if(b64_idx == BASE64_CHUNK_SIZE){int d=base64_decode(b64_in_buf,BASE64_CHUNK_SIZE,decoded_audio_buf,READ_CHUNK_SIZE);if(d>0)f_write(&audio_file,decoded_audio_buf,d,&bytes_written);b64_idx=0;} }}
                if (json_state == JSON_SEEK_CONTENT_KEY) { char* key_pos = buffer_find(current_ptr, data_end - current_ptr, "\"content\":\""); if (key_pos) { current_ptr = key_pos + strlen("\"content\":\""); json_state = JSON_STREAM_CONTENT; }}
                if (json_state == JSON_STREAM_CONTENT) { while(current_ptr < data_end) { if(*current_ptr == '"') {json_state = JSON_COMPLETE; break;} if(content_idx < content_buf_size - 1) content_buffer[content_idx++] = *current_ptr; current_ptr++; } content_buffer[content_idx] = '\0';}

                bytes_consumed = current_ptr - p_buf;
                current_chunk_size -= bytes_consumed;
                if (current_chunk_size <= 0) {
                    state = STATE_TRAILER;
                    printf("Debug: Chunk data finished.\r\n");
                }
                break;
            }
            case STATE_TRAILER: {
                if (data_in_buf >= 2 && p_buf[0] == '\r' && p_buf[1] == '\n') {
                    bytes_consumed = 2;
                    state = STATE_CHUNK_SIZE;
                    printf("Debug: Found chunk trailer, consumed 2 bytes.\r\n");
                }
                break;
            }
            default: break;
        }

        // 步骤 2: 如果成功消耗了字节，更新缓冲区并立即再次尝试解析
        if (bytes_consumed > 0) {
            if (data_in_buf > bytes_consumed) {
                memmove(stream_buf, stream_buf + bytes_consumed, data_in_buf - bytes_consumed);
            }
            data_in_buf -= bytes_consumed;
            continue; // 立即进行下一次循环，处理缓冲区中的剩余数据
        }

        // 步骤 3: 如果没有消耗字节（数据不足），则尝试从网络读取更多数据
        if (connection_alive && data_in_buf < STREAM_BUFFER_SIZE) {
            printf("Debug: Need more data. data_in_buf: %d, state: %d\r\n", data_in_buf, state);
            int bytes_read = recv(sock, stream_buf + data_in_buf, STREAM_BUFFER_SIZE - data_in_buf, 0);
            if (bytes_read < 0) {
                printf("Error: recv failed with error code.\r\n");
                state = STATE_ERROR;
            } else if (bytes_read == 0) {
                printf("Debug: Connection closed by peer.\r\n");
                connection_alive = false;
            } else {
                data_in_buf += bytes_read;
            }
        } else {
            // 步骤 4: 如果无法读取更多数据（连接已关或缓冲区已满），并且解析仍未完成，则判定为错误
            if (state != STATE_DONE) {
                printf("Error: Parser stalled. connection_alive: %d, data_in_buf: %d, state: %d\r\n", connection_alive, data_in_buf, state);
                state = STATE_ERROR;
            }
        }
    }

    // ... (清理和返回逻辑与之前相同) ...
    if (b64_idx > 0) {
        if (b64_idx % 4 == 0) {
            int decoded = base64_decode(b64_in_buf, b64_idx, decoded_audio_buf, READ_CHUNK_SIZE);
            if (decoded > 0) f_write(&audio_file, decoded_audio_buf, decoded, &bytes_written);
        } else { printf("Warning: Trailing Base64 data is not a multiple of 4.\r\n"); }
    }
    f_close(&audio_file);

    if (state == STATE_DONE) {
        printf("Successfully processed the complete stream.\r\n");
        return 0;
    } else {
        printf("Finished with error state.\r\n");
        return -1;
    }
}

int send_file_base64_over_socket(int sock,
                                 const char *filename,
                                 char *request_buffer,
                                 size_t request_buffer_size)
{
    if (request_buffer_size < OUTBUF_SIZE) return -1;

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
        retSD = f_read(&SDFile, data_in, READ_CHUNK_SIZE, &br);
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
    printf("now file name: %s\r\n", fileName);

    send_file_base64_over_socket(sock, fileName, request_buffer, sizeof(request_buffer));

    // send body part2
    send_all(sock, json_body_part2, strlen(json_body_part2));


    char received_content[OUTBUF_SIZE];
    const char* target_filename = "received_audio.wav";

    int result = process_server_response(sock, target_filename,
                                         received_content, OUTBUF_SIZE);

    if (result == 0) {
        printf("All successful!\r\n");
    } else {
        printf("Error occurred during processing: %d\r\n", result);
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
