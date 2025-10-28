#ifndef __NETWORK_H__
#define __NETWORK_H__

#define STREAM_BUFFER_SIZE       2048 // process buffer size
#define BUFFER_SIZE 4096       // ring buffer size, need to be double of STREAM_BUFFER_SIZE

#define BASE64_CHUNK_SIZE        400  // Base64 input chunk size
#define READ_CHUNK_SIZE 300   //  bytes read from file each time
#define OUTBUF_SIZE     512   //  buffer size for base64 encoding

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    STATE_HTTP_HEADER,
    STATE_CHUNK_SIZE,
    STATE_CHUNK_DATA,
//    STATE_PARSING_JSON_BODY,
    STATE_TRAILER,
    STATE_DONE,
    STATE_ERROR
} ParserState;

typedef enum {
    JSON_SEEK_FINISH_REASON,
    JSON_SEEK_AUDIO_DATA_KEY,
    JSON_STREAM_AUDIO_DATA,
    JSON_SEEK_CONTENT_KEY,
    JSON_STREAM_CONTENT,
    JSON_COMPLETE
} JsonParseState;


void startFileSending(void);
void startPostRequest(void);

#ifdef __cplusplus
};
#endif

#endif //__NETWORK_H__
