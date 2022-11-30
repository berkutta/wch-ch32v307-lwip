#include "debug.h"

#include "FreeRTOS.h"
#include "task.h"

#include "lwip/init.h"
#include "lwip/tcpip.h"

#include "lwip/dhcp.h"
#include "lwip/apps/httpd.h"


#include "ethernetif.h"

void GPIO_Toggle_INIT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}


struct netif lwip_netif;

void LwIP_Init(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_RNG, ENABLE);
    RNG_Cmd(ENABLE);
    printf("enable rng ok \r\n");

    tcpip_init(NULL, NULL);
    netif_add(&lwip_netif, NULL, NULL, NULL, NULL, &ethernetif_init, &tcpip_input);
    netif_set_default(&lwip_netif);

    /*When the netif is fully configured this function must be called*/
    netif_set_up(&lwip_netif);

    dhcp_start(&lwip_netif);

    httpd_init();

    printf("Initialized LwIP \r\n");

}

void t1_task(void *pvParameters)
{
    while(1) {
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        printf("Task1 \r\n");
    }
}


/* Global Variable */
TaskHandle_t Task1Task_Handler;

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    Delay_Init();
    USART_Printf_Init(115200);

    GPIO_Toggle_INIT();

    printf("SystemClk:%d\r\n", SystemCoreClock);

    printf("FreeRTOS Kernel Version:%s\r\n", tskKERNEL_VERSION_NUMBER);
    printf("LwIP Version: %s \r\n", LWIP_VERSION_STRING);

    LwIP_Init();

    xTaskCreate((TaskFunction_t)t1_task,
                (const char *)"t1_task",
                (uint16_t)(1024*8)/4,
                (void *)NULL,
                (UBaseType_t)5,
                (TaskHandle_t *)&Task1Task_Handler);


    vTaskStartScheduler();
}
