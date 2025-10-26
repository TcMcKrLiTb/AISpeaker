/* ethernet_task.h */
#ifndef ETHERNET_TASK_H
#define ETHERNET_TASK_H

#include "FreeRTOS.h"
#include "task.h"

void StartEthernetTask(void *argument);

// 如果有其他需要暴露的函数/变量，也在此声明
// 例如：extern void send_data_to_zhipei(void);
// （根据实际是否需要被外部调用决定）

#endif /* ETHERNET_TASK_H */