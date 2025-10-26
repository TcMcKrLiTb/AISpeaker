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

void myDNSFountCallback(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    printf("DNS finished!!!!\r\n");
    printf("remote host name: %s\r\n", name);
    printf("remote host ip: %s\r\n", ipaddr ? ipaddr_ntoa(ipaddr) : "NULL");
    *((uint8_t *)callback_arg) = 1;
}

void doPostRequest()
{
    ip_addr_t ipAddr;
    ip_addr_t *dnsServerAddr;
    uint8_t finished = 0;

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

    dns_gethostbyname("dxxyzhxg.bjtu.edu.cn", &ipAddr, myDNSFountCallback, &finished);

    while (finished == 0) {
        osDelay(1);
    }

    printf("Resolved IP: %s\r\n", ipaddr_ntoa(&ipAddr));
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
