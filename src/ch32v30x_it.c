/********************************** (C) COPYRIGHT *******************************
* File Name          : ch32v30x_it.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : Main Interrupt Service Routines.
*******************************************************************************/
#include "ch32v30x_it.h"

#include "FreeRTOS.h"
#include "task.h"

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/*********************************************************************
 * @fn      NMI_Handler
 *
 * @brief   This function handles NMI exception.
 *
 * @return  none
 */
void NMI_Handler(void)
{
}

/*********************************************************************
 * @fn      HardFault_Handler
 *
 * @brief   This function handles Hard Fault exception.
 *
 * @return  none
 */
void HardFault_Handler(void)
{
  printf("HardFault_Handler\r\n");
  printf("mepc  :%08x\r\n", __get_MEPC());
  printf("mcause:%08x\r\n", __get_MCAUSE());
  printf("mtval :%08x\r\n", __get_MTVAL());

  printf("Current Task: %s, dumping register data\r\n", pcTaskGetName(xTaskGetCurrentTaskHandle()));

  printf("~/Downloads/MRS_Toolchain_Linux_x64_V1.60/RISC-V\\ Embedded\\ GCC/bin/riscv-none-embed-addr2line -e cdc-acm.elf -f 0x%08x -a -p \r\n", __get_MEPC());

  while (1);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {   
    printf("%s task stack overflow \r\n", pcTaskName);
    printf("FreeRTOS free heap size: %dB \r\n", xPortGetFreeHeapSize);

    while (1);
}
