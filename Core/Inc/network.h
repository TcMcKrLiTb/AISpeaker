#ifndef __NETWORK_H__
#define __NETWORK_H__

#define READ_CHUNK_SIZE 300   //  bytes read from file each time
#define OUTBUF_SIZE     512   //  buffer size for base64 encoding

#ifdef __cplusplus
extern "C" {
#endif

void startFileSending(void);
void startPostRequest(void);

#ifdef __cplusplus
};
#endif

#endif //__NETWORK_H__
