#include "eth_driver.h"
#include "FreeRTOS.h"
#include "debug.h"
#include "ethernetif.h"
#include "lwipopts.h"
#include "string.h"
#include "task.h"

/* MII/MDI interface select */
#define PHY_ADDRESS 0x01

#define USE10BASE_T
#ifndef USE10BASE_T
//    #define USE_GIGA_MAC
#ifndef USE_GIGA_MAC
#define USE_FAST_MAC
//#define USE_RMII
#endif
#endif

__attribute__((aligned(4))) ETH_DMADESCTypeDef DMARxDscrTab[ETH_RXBUFNB]; /* 接收描述符表 */
__attribute__((aligned(4))) ETH_DMADESCTypeDef DMATxDscrTab[ETH_TXBUFNB]; /* 发送描述符表 */
__attribute__((aligned(4))) uint8_t Rx_Buff[ETH_RXBUFNB][ETH_MAX_PACKET_SIZE]; /* 接收队列 */
__attribute__((aligned(4))) uint8_t Tx_Buff[ETH_TXBUFNB][ETH_MAX_PACKET_SIZE]; /* 发送队列 */

/* extern variable */

/* Macro */
#ifndef ETH_ERROR
#define ETH_ERROR ((uint32_t)0)
#endif

#ifndef ETH_SUCCESS
#define ETH_SUCCESS ((uint32_t)1)
#endif

#define ETH_DMARxDesc_FrameLengthShift 16

#define define_O(a, b)                                \
    GPIO_InitStructure.GPIO_Pin = b;                  \
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; \
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   \
    GPIO_Init(a, &GPIO_InitStructure)

#define define_I(a, b)                                    \
    GPIO_InitStructure.GPIO_Pin = b;                      \
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     \
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; \
    GPIO_Init(a, &GPIO_InitStructure)

void Ethernet_LED_LINKSET(uint8_t setbit) {
    if (setbit) {
        GPIO_SetBits(GPIOB, GPIO_Pin_8);
    } else {
        GPIO_ResetBits(GPIOB, GPIO_Pin_8);
    }
}

void Ethernet_LED_DATASET(uint8_t setbit) {
    if (setbit) {
        GPIO_SetBits(GPIOB, GPIO_Pin_9);
    } else {
        GPIO_ResetBits(GPIOB, GPIO_Pin_9);
    }
}

void init_phy(void) {
    GPIO_InitTypeDef GPIO = {0};
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO.GPIO_Pin = GPIO_Pin_8;
    GPIO.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOA, &GPIO);

    RCC_PLL3Cmd(DISABLE);
    RCC_PREDIV2Config(RCC_PREDIV2_Div2);
    RCC_PLL3Config(RCC_PLL3Mul_15);
    RCC_MCOConfig(RCC_MCO_PLL3CLK);
    RCC_PLL3Cmd(ENABLE);

    vTaskDelay(100 / portTICK_PERIOD_MS);

    while (RESET == RCC_GetFlagStatus(RCC_FLAG_PLL3RDY)) {
        printf("Wait for PLL3 ready. \r\n");
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    printf("PLL3 is ready. \r\n");

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    /* Ethernet LED Configuration */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO);

    Ethernet_LED_LINKSET(1); /* turn off link led. */
    Ethernet_LED_DATASET(1); /* turn off data led. */

    /*
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_RNG, ENABLE);
        RNG_Cmd(ENABLE);
        printf("enable rng ok \r\n");
    */

    /* Enable Ethernet MAC clock */
    RCC_AHBPeriphClockCmd(
        RCC_AHBPeriph_ETH_MAC | RCC_AHBPeriph_ETH_MAC_Tx | RCC_AHBPeriph_ETH_MAC_Rx, ENABLE);

#ifdef USE10BASE_T
    /* Enable internal 10BASE-T PHY*/
    EXTEN->EXTEN_CTR |= EXTEN_ETH_10M_EN; /* 使能10M以太网物理层   */
#endif

#ifdef USE_GIGA_MAC
    /* Enable 1G MAC*/
    EXTEN->EXTEN_CTR |= EXTEN_ETH_RGMII_SEL;       /* 使能1G以太网MAC */
    RCC_ETH1GCLKConfig(RCC_ETH1GCLKSource_PB1_IN); /* 选择外部125MHz输入 */
    RCC_ETH1G_125Mcmd(ENABLE);                     /* 使能125MHz时钟 */

    /*  Enable RGMII GPIO */
    GETH_pin_init();
#endif

#ifdef USE_FAST_MAC
    /*  Enable MII or RMII GPIO */
    FETH_pin_init();
#endif

    /* Reset ETHERNET on AHB Bus */
    ETH_DeInit();

    /* Software reset */
    ETH_SoftwareReset();

    /* Ethernet_Configuration */
    static ETH_InitTypeDef ETH_InitStructure = {0};
    static uint32_t timeout;

    /* Wait for software reset */
    timeout = 10;
    // OS_SUBNT_SET_STATE();
    if (ETH->DMABMR & ETH_DMABMR_SR) {
        timeout--;
        if (timeout == 0) {
            printf("Error:Eth soft-reset timeout!\nPlease check RGMII TX & RX clock line. \r\n");
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    /* ETHERNET Configuration ------------------------------------------------------*/
    /* Call ETH_StructInit if you don't like to configure all ETH_InitStructure parameter */
    ETH_StructInit(&ETH_InitStructure);
    /* Fill ETH_InitStructure parametrs */
    /*------------------------   MAC   -----------------------------------*/
    ETH_InitStructure.ETH_Mode = ETH_Mode_FullDuplex;
    ETH_InitStructure.ETH_Speed = ETH_Speed_1000M;
    ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;
    ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;
    ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
    ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
    ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Enable;
    ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;
    ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Enable;
    ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
    ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
#ifdef CHECKSUM_BY_HARDWARE
    ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable;
#endif
    /*------------------------   DMA   -----------------------------------*/
    /* When we use the Checksum offload feature, we need to enable the Store and Forward mode:
    the store and forward guarantee that a whole frame is stored in the FIFO, so the MAC can
    insert/verify the checksum, if the checksum is OK the DMA can handle the frame otherwise the
    frame is dropped */
    ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable;
    ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;
    ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;
    ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Enable;
    ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Enable;
    ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Disable;
    ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;
    ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;
    ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;
    ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;
    ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;

    /* Configure Ethernet */
    uint32_t tmpreg = 0;
    static uint16_t RegValue = 0;

    /*---------------------- 物理层配置 -------------------*/
    /* 置SMI接口时钟 ，置为主频的42分频  */
    tmpreg = ETH->MACMIIAR;
    tmpreg &= MACMIIAR_CR_MASK;
    tmpreg |= (uint32_t)ETH_MACMIIAR_CR_Div42;
    ETH->MACMIIAR = (uint32_t)tmpreg;

    /* 复位物理层 */
    ETH_WritePHYRegister(PHY_ADDRESS, PHY_BCR, PHY_Reset); /* 复位物理层  */

    vTaskDelay(100 / portTICK_PERIOD_MS);

    timeout = 10000; /* 最大超时十秒   */
    RegValue = 0;

    RegValue = ETH_ReadPHYRegister(PHY_ADDRESS, PHY_BCR);
    if ((RegValue & (PHY_Reset))) {
        timeout--;
        if (timeout <= 0) {
            printf(
                "Error:Wait phy software timeout!\nPlease cheak PHY/MID.\nProgram has been "
                "blocked!\n");
            while (1)
                ;
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    /* 等待物理层与对端建立LINK */
    timeout = 10000; /* 最大超时十秒   */
    RegValue = 0;

    RegValue = ETH_ReadPHYRegister(PHY_ADDRESS, PHY_BSR);
    if ((RegValue & (PHY_Linked_Status)) == 0) {
        timeout--;
        if (timeout <= 0) {
            printf(
                "Error:Wait phy linking timeout!\nPlease cheak MID.\nProgram has been blocked!\n");
            while (1)
                ;
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    /* 等待物理层完成自动协商 */
    timeout = 10000; /* 最大超时十秒   */
    RegValue = 0;

    RegValue = ETH_ReadPHYRegister(PHY_ADDRESS, PHY_BSR);
    if ((RegValue & PHY_AutoNego_Complete) == 0) {
        timeout--;
        if (timeout <= 0) {
            printf(
                "Error:Wait phy auto-negotiation complete timeout!\nPlease cheak MID.\nProgram has "
                "been blocked!\n");
            while (1)
                ;
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    RegValue = ETH_ReadPHYRegister(PHY_ADDRESS, 0x10);
    printf("PHY_SR value:%04x. \r\n", RegValue);

    if (RegValue & (1 << 2)) {
        ETH_InitStructure.ETH_Mode = ETH_Mode_FullDuplex;
        printf("Full Duplex. \r\n");
    } else {
        ETH_InitStructure.ETH_Mode = ETH_Mode_HalfDuplex;
        printf("Half Duplex. \r\n");
    }
    ETH_InitStructure.ETH_Speed = ETH_Speed_10M;
    if (RegValue & (1 << 3)) {
        printf("Loopback_10M \r\n");
    } else {
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);

    /* 点亮link状态led灯 */
    Ethernet_LED_LINKSET(0);

    /*------------------------ MAC寄存器配置  ----------------------- --------------------*/
    /* MACCCR在RGMII接口模式下具有调整RGMII接口时序的域，请注意 */
    tmpreg = ETH->MACCR;
    tmpreg &= MACCR_CLEAR_MASK;
    tmpreg |=
        (uint32_t)(ETH_InitStructure.ETH_Watchdog | ETH_InitStructure.ETH_Jabber |
                   ETH_InitStructure.ETH_InterFrameGap | ETH_InitStructure.ETH_CarrierSense |
                   ETH_InitStructure.ETH_Speed | ETH_InitStructure.ETH_ReceiveOwn |
                   ETH_InitStructure.ETH_LoopbackMode | ETH_InitStructure.ETH_Mode |
                   ETH_InitStructure.ETH_ChecksumOffload | ETH_InitStructure.ETH_RetryTransmission |
                   ETH_InitStructure.ETH_AutomaticPadCRCStrip | ETH_InitStructure.ETH_BackOffLimit |
                   ETH_InitStructure.ETH_DeferralCheck);
    /* 写MAC控制寄存器 */
    ETH->MACCR = (uint32_t)tmpreg;
#ifdef USE10BASE_T
    ETH->MACCR |= ETH_Internal_Pull_Up_Res_Enable; /* 启用内部上拉  */
#endif
    ETH->MACFFR =
        (uint32_t)(ETH_InitStructure.ETH_ReceiveAll | ETH_InitStructure.ETH_SourceAddrFilter |
                   ETH_InitStructure.ETH_PassControlFrames |
                   ETH_InitStructure.ETH_BroadcastFramesReception |
                   ETH_InitStructure.ETH_DestinationAddrFilter |
                   ETH_InitStructure.ETH_PromiscuousMode |
                   ETH_InitStructure.ETH_MulticastFramesFilter |
                   ETH_InitStructure.ETH_UnicastFramesFilter);
    /*--------------- ETHERNET MACHTHR and MACHTLR Configuration ---------------*/
    /* Write to ETHERNET MACHTHR */
    ETH->MACHTHR = (uint32_t)ETH_InitStructure.ETH_HashTableHigh;
    /* Write to ETHERNET MACHTLR */
    ETH->MACHTLR = (uint32_t)ETH_InitStructure.ETH_HashTableLow;
    /*----------------------- ETHERNET MACFCR Configuration --------------------*/
    /* Get the ETHERNET MACFCR value */
    tmpreg = ETH->MACFCR;
    /* Clear xx bits */
    tmpreg &= MACFCR_CLEAR_MASK;

    tmpreg |=
        (uint32_t)((ETH_InitStructure.ETH_PauseTime << 16) | ETH_InitStructure.ETH_ZeroQuantaPause |
                   ETH_InitStructure.ETH_PauseLowThreshold |
                   ETH_InitStructure.ETH_UnicastPauseFrameDetect |
                   ETH_InitStructure.ETH_ReceiveFlowControl |
                   ETH_InitStructure.ETH_TransmitFlowControl);
    ETH->MACFCR = (uint32_t)tmpreg;

    ETH->MACVLANTR = (uint32_t)(ETH_InitStructure.ETH_VLANTagComparison |
                                ETH_InitStructure.ETH_VLANTagIdentifier);

    tmpreg = ETH->DMAOMR;
    /* Clear xx bits */
    tmpreg &= DMAOMR_CLEAR_MASK;

    tmpreg |= (uint32_t)(ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame |
                         ETH_InitStructure.ETH_ReceiveStoreForward |
                         ETH_InitStructure.ETH_FlushReceivedFrame |
                         ETH_InitStructure.ETH_TransmitStoreForward |
                         ETH_InitStructure.ETH_TransmitThresholdControl |
                         ETH_InitStructure.ETH_ForwardErrorFrames |
                         ETH_InitStructure.ETH_ForwardUndersizedGoodFrames |
                         ETH_InitStructure.ETH_ReceiveThresholdControl |
                         ETH_InitStructure.ETH_SecondFrameOperate);
    ETH->DMAOMR = (uint32_t)tmpreg;

    ETH->DMABMR =
        (uint32_t)(ETH_InitStructure.ETH_AddressAlignedBeats | ETH_InitStructure.ETH_FixedBurst |
                   ETH_InitStructure.ETH_RxDMABurstLength | /* !! if 4xPBL is selected for Tx or Rx
                                                               it is applied for the other */
                   ETH_InitStructure.ETH_TxDMABurstLength |
                   (ETH_InitStructure.ETH_DescriptorSkipLength << 2) |
                   ETH_InitStructure.ETH_DMAArbitration | ETH_DMABMR_USP);

    /* Enable the Ethernet Rx Interrupt */
    ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R | ETH_DMA_IT_T, ENABLE);

    NVIC_EnableIRQ(ETH_IRQn);
    ETH_DMATxDescChainInit(DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);
    ETH_DMARxDescChainInit(DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);
    ETH_Start();
}

void ETH_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/*********************************************************************
 * @fn      ETH_IRQHandler
 *
 * @brief   This function handles ETH exception.
 *
 * @return  none
 */
void ETH_IRQHandler(void) {
    if (ETH->DMASR & ETH_DMA_IT_R) {
        ETH_DMAClearITPendingBit(ETH_DMA_IT_R);

        ethernetif_input(&lwip_netif);
    }
    if (ETH->DMASR & ETH_DMA_IT_T) {
        ETH_DMAClearITPendingBit(ETH_DMA_IT_T);
    }

    ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);
}

/*********************************************************************
 * @fn      CH30x_RNG_GENERATE
 *
 * @brief   CH30x_RNG_GENERATE api function for lwip.
 *
 * @param   None.
 *
 * @return  None.
 */
uint32_t CH30x_RNG_GENERATE() {
    while (1) {
        if (RNG_GetFlagStatus(RNG_FLAG_DRDY) == SET) {
            break;
        }
        if (RNG_GetFlagStatus(RNG_FLAG_CECS) == SET) {
            /* 时钟错误 */
            RNG_ClearFlag(RNG_FLAG_CECS);
            vTaskDelay(1 / portTICK_PERIOD_MS);
        }
        if (RNG_GetFlagStatus(RNG_FLAG_SECS) == SET) {
            /* 种子错误 */
            RNG_ClearFlag(RNG_FLAG_SECS);
            RNG_Cmd(DISABLE);
            vTaskDelay(1 / portTICK_PERIOD_MS);
            RNG_Cmd(ENABLE);
            vTaskDelay(1 / portTICK_PERIOD_MS);
        }
    }
    return RNG_GetRandomNumber();
}

/* received data */
FrameTypeDef ETH_RxPkt_ChainMode(void) {
    u32 framelength = 0;
    FrameTypeDef frame = {0, 0};

    /* Check if the descriptor is owned by the ETHERNET DMA (when set) or CPU (when reset) */
    if ((DMARxDescToGet->Status & ETH_DMARxDesc_OWN) != (u32)RESET) {
        frame.length = ETH_ERROR;
        if ((ETH->DMASR & ETH_DMASR_RBUS) != (u32)RESET) {
            /* Clear RBUS ETHERNET DMA flag */
            ETH->DMASR = ETH_DMASR_RBUS;
            /* Resume DMA reception */
            ETH->DMARPDR = 0;
        }
        printf("Error:ETH_DMARxDesc_OWN.\r\n");
        /* Return error: OWN bit set */
        return frame;
    }

    if (((DMARxDescToGet->Status & ETH_DMARxDesc_ES) == (u32)RESET) &&
        ((DMARxDescToGet->Status & ETH_DMARxDesc_LS) != (u32)RESET) &&
        ((DMARxDescToGet->Status & ETH_DMARxDesc_FS) != (u32)RESET)) {
        /* Get the Frame Length of the received packet: substruct 4 bytes of the CRC */
        framelength =
            ((DMARxDescToGet->Status & ETH_DMARxDesc_FL) >> ETH_DMARxDesc_FrameLengthShift) - 4;

        /* Get the addrees of the actual buffer */
        frame.buffer = DMARxDescToGet->Buffer1Addr;
    } else {
        /* Return ERROR */
        framelength = ETH_ERROR;
        printf("Error:recv error frame,status:0x%08x.\r\n", DMARxDescToGet->Status);
    }
    DMARxDescToGet->Status |= ETH_DMARxDesc_OWN;
    frame.length = framelength;
    frame.descriptor = DMARxDescToGet;

    /* Update the ETHERNET DMA global Rx descriptor with next Rx decriptor */
    /* Chained Mode */
    /* Selects the next DMA Rx descriptor list for next buffer to read */
    DMARxDescToGet = (ETH_DMADESCTypeDef*)(DMARxDescToGet->Buffer2NextDescAddr);
    /* Return Frame */
    return (frame);
}

/*********************************************************************
 * @fn      ETH_TxPkt_ChainMode
 *
 * @brief   MAC send a ethernet frame in chain mode.
 *
 * @param   Send length
 *
 * @return  Send status.
 */
uint32_t ETH_TxPkt_ChainMode(u16 FrameLength) {
    /* Check if the descriptor is owned by the ETHERNET DMA (when set) or CPU (when reset) */
    if ((DMATxDescToSet->Status & ETH_DMATxDesc_OWN) != (u32)RESET) {
        /* Return ERROR: OWN bit set */
        return ETH_ERROR;
    }

    /* Setting the Frame Length: bits[12:0] */
    DMATxDescToSet->ControlBufferSize = (FrameLength & ETH_DMATxDesc_TBS1);
#ifdef CHECKSUM_BY_HARDWARE
    /* Setting the last segment and first segment bits (in this case a frame is transmitted in one
     * descriptor) */
    DMATxDescToSet->Status |=
        ETH_DMATxDesc_LS | ETH_DMATxDesc_FS | ETH_DMATxDesc_CIC_TCPUDPICMP_Full;
#else
    DMATxDescToSet->Status |= ETH_DMATxDesc_LS | ETH_DMATxDesc_FS;
#endif
    /* Set Own bit of the Tx descriptor Status: gives the buffer back to ETHERNET DMA */
    DMATxDescToSet->Status |= ETH_DMATxDesc_OWN;

    /* When Tx Buffer unavailable flag is set: clear it and resume transmission */
    if ((ETH->DMASR & ETH_DMASR_TBUS) != (u32)RESET) {
        /* Clear TBUS ETHERNET DMA flag */
        ETH->DMASR = ETH_DMASR_TBUS;
        /* Resume DMA transmission*/

        ETH->DMATPDR = 0;
    }

    /* Update the ETHERNET DMA global Tx descriptor with next Tx decriptor */
    /* Chained Mode */
    /* Selects the next DMA Tx descriptor list for next buffer to send */
    DMATxDescToSet = (ETH_DMADESCTypeDef*)(DMATxDescToSet->Buffer2NextDescAddr);

    /* Return SUCCESS */
    return ETH_SUCCESS;
}

/*********************************************************************
 * @fn      PHY_control_pin_init
 *
 * @brief   PHY interrupt GPIO Initialization.
 *
 * @return  none
 */
void PHY_control_pin_init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource15);
    EXTI_InitStructure.EXTI_Line = EXTI_Line15;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/*********************************************************************
 * @fn      GETH_pin_init
 *
 * @brief   PHY RGMII interface GPIO initialization.
 *
 * @return  none
 */
void GETH_pin_init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    /* PB12/13置为推挽复用输出 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC,
                           ENABLE);
    GPIOB->CFGHR &= ~(0xff << 16);
    GPIOB->CFGHR |= (0xbb << 16);
    GPIOB->CFGLR &= ~(0xff << 4);

    define_O(GPIOA, GPIO_Pin_2);
    define_O(GPIOA, GPIO_Pin_3);
    define_O(GPIOA, GPIO_Pin_7);
    define_O(GPIOC, GPIO_Pin_4);
    define_O(GPIOC, GPIO_Pin_5);
    define_O(GPIOB, GPIO_Pin_0);

    define_I(GPIOC, GPIO_Pin_0);
    define_I(GPIOC, GPIO_Pin_1);
    define_I(GPIOC, GPIO_Pin_2);
    define_I(GPIOC, GPIO_Pin_3);
    define_I(GPIOA, GPIO_Pin_0);
    define_I(GPIOA, GPIO_Pin_1);

    define_I(GPIOB, GPIO_Pin_1); /* 125m in */
}

/*********************************************************************
 * @fn      FETH_pin_init
 *
 * @brief   PHY MII/RMII interface GPIO initialization.
 *
 * @return  none
 */
void FETH_pin_init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

#ifdef USE_RMII
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO,
        ENABLE);
    GPIO_ETH_MediaInterfaceConfig(GPIO_ETH_MediaInterface_RMII);
    define_O(GPIOA, GPIO_Pin_2); /* MDC */
    define_O(GPIOC, GPIO_Pin_1); /* MDIO */

    define_O(GPIOB, GPIO_Pin_11);  // txen
    define_O(GPIOB, GPIO_Pin_12);  // txd0
    define_O(GPIOB, GPIO_Pin_13);  // txd1

    define_I(GPIOA, GPIO_Pin_1); /* PA1 REFCLK */
    define_I(GPIOA, GPIO_Pin_7); /* PA7 CRSDV */
    define_I(GPIOC, GPIO_Pin_4); /* RXD0 */
    define_I(GPIOC, GPIO_Pin_5); /* RXD1 */

#else
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC,
                           ENABLE);

    define_O(GPIOA, GPIO_Pin_2); /* MDC */
    define_O(GPIOC, GPIO_Pin_1); /* MDIO */

    define_I(GPIOC, GPIO_Pin_3);   // txclk
    define_O(GPIOB, GPIO_Pin_11);  // txen
    define_O(GPIOB, GPIO_Pin_12);  // txd0
    define_O(GPIOB, GPIO_Pin_13);  // txd1
    define_O(GPIOC, GPIO_Pin_2);   // txd2
    define_O(GPIOB, GPIO_Pin_8);   // txd3
    /* RX组 */
    define_I(GPIOA, GPIO_Pin_1);  /* PA1 RXC */
    define_I(GPIOA, GPIO_Pin_7);  /* PA7 RXDV */
    define_I(GPIOC, GPIO_Pin_4);  /* RXD0 */
    define_I(GPIOC, GPIO_Pin_5);  /* RXD1 */
    define_I(GPIOB, GPIO_Pin_0);  /* RXD2 */
    define_I(GPIOB, GPIO_Pin_1);  /* RXD3 */
    define_I(GPIOB, GPIO_Pin_10); /* RXER */

    define_O(GPIOA, GPIO_Pin_0); /* PA0 */
    define_O(GPIOA, GPIO_Pin_3); /* PA3 */
#endif
}
