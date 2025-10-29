#ifndef __NETWORK_H__
#define __NETWORK_H__

#define STREAM_BUFFER_SIZE        4096 // process buffer size
#define FILE_PROCESS_BUFFER_SIZE  4080 // file read buffer size
#define FILE_PROCESS_HALF_BUFFER_SIZE  2040 // half of file read buffer size
#define BASE64_WORK_BUFFER_SIZE (FILE_PROCESS_HALF_BUFFER_SIZE + 4)

#define READ_CHUNK_SIZE 300   //  bytes read from file each time
#define OUTBUF_SIZE     512   //  buffer size for base64 encoding

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    STATE_PROCESS_FIRST_CHUNK,
    STATE_PARSE_CHUNK_HEADER,
    STATE_STREAM_AUDIO_DATA,
    STATE_FINISH,
    STATE_ERROR
} ProcesserState;


void startFileSending(void);
void startPostRequest(void);

#ifdef __cplusplus
};
#endif

#endif //__NETWORK_H__
