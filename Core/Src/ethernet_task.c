#include "main.h"
#include "lwip.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ethernet_task.h"

void StartEthernetTask(void *argument)
{
    for(;;)
    {
        osDelay(1);
    }
}