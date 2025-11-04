#ifndef __NETWORK_H__
#define __NETWORK_H__

#define STREAM_BUFFER_SIZE        2056 // process buffer size
#define STREAM_BUFFER_MARGIN      (STREAM_BUFFER_SIZE - 2048)
#define BASE_STREAM_BUFFER_SIZE   1000 // base64 decode buffer
#define BASE64_PAD_CHAR   '='
#define BASE64_INVALID    0xFF
#define BASE64_PAD_VAL    0xFE

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

typedef enum {
    SPEED_NORMAL = 0,
    SPEED_FAST,
    SPEED_SLOW,
    SPEED_MAX_NUM
} ReplySpeed;

typedef enum {
    EMOTION_MIDDLE = 0,
    EMOTION_HAPPY,
    EMOTION_SAD,
    EMOTION_SERIOUS,
    EMOTION_MAX_NUM
} ReplyEmotion;

void startFileSending(void);
void startPostRequest(void);

#ifdef __cplusplus
};
#endif

#endif //__NETWORK_H__
