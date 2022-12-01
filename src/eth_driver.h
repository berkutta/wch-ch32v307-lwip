#ifndef NET__H
#define NET__H

#include "ch32v30x_eth.h"
#include "ch32v30x_exti.h"
#include "ch32v30x_rng.h"

#define ETH_RXBUFNB 4
#define ETH_TXBUFNB 4

#if 0
#define USE_LOOP_STRUCT 1
#else
#define USE_CHAIN_STRUCT 1
#endif

typedef struct {
    void *next;
    u32 length;
    u32 buffer;
    ETH_DMADESCTypeDef *descriptor;
} FrameTypeDef;

extern ETH_DMADESCTypeDef *DMATxDescToSet;
extern ETH_DMADESCTypeDef *DMARxDescToGet;

extern ETH_DMADESCTypeDef DMARxDscrTab[ETH_RXBUFNB]; /* 接收描述符表 */
extern ETH_DMADESCTypeDef DMATxDscrTab[ETH_TXBUFNB]; /* 发送描述符表 */

extern uint32_t ETH_TxPkt_ChainMode(u16 FrameLength);
void mac_send(uint8_t *content_ptr, uint16_t content_len);

extern FrameTypeDef ETH_RxPkt_ChainMode(void);

extern void PHY_control_pin_init(void);
extern void GETH_pin_init(void);
extern void FETH_pin_init(void);

#define ROM_CFG_USERADR_ID                   0x1FFFF7E8

extern void WCH_GetMacAddr(uint8_t *p);

void init_phy(void);

#endif