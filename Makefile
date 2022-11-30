# Project Name
PROJECT = ch32v307-lwip

# Path you your toolchain and openocd installation, leave empty if already in system PATH
TOOLCHAIN_ROOT = "$(HOME)/Downloads/MRS_Toolchain_Linux_x64_V1.60/RISC-V Embedded GCC/bin/"
OPENOCD_ROOT   = $(HOME)/Downloads/MRS_Toolchain_Linux_x64_V1.60/OpenOCD/bin/

# Path to the WCH vendor codebase, make sure to update the submodule to get the code
VENDOR_ROOT = ./vendor/

FREERTOS_ROOT = ./src/lib/FreeRTOS/
LWIP_ROOT = ./src/lib/lwip/src/

###############################################################################

# Project specific
TARGET = $(PROJECT).elf
SRC_DIR = src/

CORE_DIR = $(VENDOR_ROOT)Core/
DEBUG_DIR = $(VENDOR_ROOT)Debug/
PERIPHERAL_DIR = $(VENDOR_ROOT)Peripheral/
STARTUP_DIR = $(VENDOR_ROOT)Startup/

# Toolchain
CC = $(TOOLCHAIN_ROOT)riscv-none-embed-gcc
DB = $(TOOLCHAIN_ROOT)riscv-none-embed-gdb
SIZE = $(TOOLCHAIN_ROOT)riscv-none-embed-size

# Project sources
SRC_FILES  = $(wildcard $(SRC_DIR)*.c) 

# FreeRTOS
SRC_FILES += $(FREERTOS_ROOT)croutine.c
SRC_FILES += $(FREERTOS_ROOT)event_groups.c
SRC_FILES += $(FREERTOS_ROOT)list.c
SRC_FILES += $(FREERTOS_ROOT)queue.c
SRC_FILES += $(FREERTOS_ROOT)stream_buffer.c
SRC_FILES += $(FREERTOS_ROOT)tasks.c
SRC_FILES += $(FREERTOS_ROOT)timers.c
SRC_FILES += $(FREERTOS_ROOT)portable/GCC/RISC-V/port.c
ASM_FILES += $(FREERTOS_ROOT)portable/GCC/RISC-V/portASM.S
SRC_FILES += $(FREERTOS_ROOT)portable/MemMang/heap_4.c

# LwIP
SRC_FILES += $(LWIP_ROOT)core/def.c
SRC_FILES += $(LWIP_ROOT)core/inet_chksum.c
SRC_FILES += $(LWIP_ROOT)core/init.c
SRC_FILES += $(LWIP_ROOT)core/ip.c
SRC_FILES += $(LWIP_ROOT)core/mem.c
SRC_FILES += $(LWIP_ROOT)core/memp.c
SRC_FILES += $(LWIP_ROOT)core/netif.c
SRC_FILES += $(LWIP_ROOT)core/pbuf.c
SRC_FILES += $(LWIP_ROOT)core/raw.c
SRC_FILES += $(LWIP_ROOT)core/stats.c
SRC_FILES += $(LWIP_ROOT)core/sys.c
SRC_FILES += $(LWIP_ROOT)core/tcp_in.c
SRC_FILES += $(LWIP_ROOT)core/tcp_out.c
SRC_FILES += $(LWIP_ROOT)core/tcp.c
SRC_FILES += $(LWIP_ROOT)core/timeouts.c
SRC_FILES += $(LWIP_ROOT)core/udp.c
SRC_FILES += $(LWIP_ROOT)core/ipv4/acd.c
SRC_FILES += $(LWIP_ROOT)core/ipv4/dhcp.c
SRC_FILES += $(LWIP_ROOT)core/ipv4/etharp.c
SRC_FILES += $(LWIP_ROOT)core/ipv4/icmp.c
SRC_FILES += $(LWIP_ROOT)core/ipv4/ip4_addr.c
SRC_FILES += $(LWIP_ROOT)core/ipv4/ip4_frag.c
SRC_FILES += $(LWIP_ROOT)core/ipv4/ip4.c
SRC_FILES += $(LWIP_ROOT)netif/ethernet.c
SRC_FILES += $(LWIP_ROOT)api/api_lib.c
SRC_FILES += $(LWIP_ROOT)api/api_msg.c
SRC_FILES += $(LWIP_ROOT)api/err.c
SRC_FILES += $(LWIP_ROOT)api/if_api.c
SRC_FILES += $(LWIP_ROOT)api/netbuf.c
SRC_FILES += $(LWIP_ROOT)api/netdb.c
SRC_FILES += $(LWIP_ROOT)api/netifapi.c
SRC_FILES += $(LWIP_ROOT)api/sockets.c
SRC_FILES += $(LWIP_ROOT)api/tcpip.c
SRC_FILES += $(LWIP_ROOT)core/altcp.c
SRC_FILES += $(LWIP_ROOT)core/altcp_tcp.c
SRC_FILES += $(LWIP_ROOT)apps/http/httpd.c
SRC_FILES += $(LWIP_ROOT)apps/http/fs.c
SRC_FILES += $(LWIP_ROOT)../contrib/ports/freertos/sys_arch.c

# Project includes
INCLUDES  = -I$(SRC_DIR)

INCLUDES += -I$(FREERTOS_ROOT)/include/
INCLUDES += -I$(FREERTOS_ROOT)/portable/GCC/RISC-V/
INCLUDES += -I$(FREERTOS_ROOT)/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions/

INCLUDES += -I$(LWIP_ROOT)
INCLUDES += -I$(LWIP_ROOT)../contrib/ports/freertos/include
INCLUDES += -I$(LWIP_ROOT)/include

# Vendor sources:
ASM_FILES += $(STARTUP_DIR)startup_ch32v30x_D8C.S
SRC_FILES += $(CORE_DIR)core_riscv.c
SRC_FILES += $(DEBUG_DIR)debug.c
SRC_FILES += $(PERIPHERAL_DIR)src/ch32v30x_adc.c
SRC_FILES += $(PERIPHERAL_DIR)src/ch32v30x_bkp.c
SRC_FILES += $(PERIPHERAL_DIR)src/ch32v30x_can.c
SRC_FILES += $(PERIPHERAL_DIR)src/ch32v30x_crc.c
SRC_FILES += $(PERIPHERAL_DIR)src/ch32v30x_dac.c
SRC_FILES += $(PERIPHERAL_DIR)src/ch32v30x_dbgmcu.c
SRC_FILES += $(PERIPHERAL_DIR)src/ch32v30x_dma.c
SRC_FILES += $(PERIPHERAL_DIR)src/ch32v30x_dvp.c
SRC_FILES += $(PERIPHERAL_DIR)src/ch32v30x_eth.c
SRC_FILES += $(PERIPHERAL_DIR)src/ch32v30x_exti.c
SRC_FILES += $(PERIPHERAL_DIR)src/ch32v30x_flash.c
SRC_FILES += $(PERIPHERAL_DIR)src/ch32v30x_fsmc.c
SRC_FILES += $(PERIPHERAL_DIR)src/ch32v30x_gpio.c
SRC_FILES += $(PERIPHERAL_DIR)src/ch32v30x_i2c.c
SRC_FILES += $(PERIPHERAL_DIR)src/ch32v30x_iwdg.c
SRC_FILES += $(PERIPHERAL_DIR)src/ch32v30x_misc.c
SRC_FILES += $(PERIPHERAL_DIR)src/ch32v30x_opa.c
SRC_FILES += $(PERIPHERAL_DIR)src/ch32v30x_pwr.c
SRC_FILES += $(PERIPHERAL_DIR)src/ch32v30x_rcc.c 
SRC_FILES += $(PERIPHERAL_DIR)src/ch32v30x_rng.c
SRC_FILES += $(PERIPHERAL_DIR)src/ch32v30x_rtc.c
SRC_FILES += $(PERIPHERAL_DIR)src/ch32v30x_sdio.c
SRC_FILES += $(PERIPHERAL_DIR)src/ch32v30x_spi.c
SRC_FILES += $(PERIPHERAL_DIR)src/ch32v30x_tim.c
SRC_FILES += $(PERIPHERAL_DIR)src/ch32v30x_usart.c
SRC_FILES += $(PERIPHERAL_DIR)src/ch32v30x_wwdg.c

# Vendor includes
INCLUDES  += -I$(CORE_DIR)
INCLUDES  += -I$(DEBUG_DIR)
INCLUDES  += -I$(PERIPHERAL_DIR)inc/

# Vendor Link Script
LD_SCRIPT = $(VENDOR_ROOT)Ld/Link.ld

# Compiler Flags
CFLAGS  = -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -mno-save-restore -O0
CFLAGS += -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized -g
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)"
CFLAGS += $(INCLUDES)

# Assembler Flags
ASMFLAGS = -x assembler-with-cpp

# Linker Flags
LFLAGS = -T $(LD_SCRIPT) -u _printf_float -nostartfiles -Xlinker --gc-sections -Wl,-Map,$(PROJECT).map --specs=nano.specs --specs=nosys.specs -static

###############################################################################

# This does an in-source build. An out-of-source build that places all object
# files into a build directory would be a better solution, but the goal was to
# keep this file very simple.

CXX_OBJS = $(SRC_FILES:.c=.o)
ASM_OBJS = $(ASM_FILES:.S=.o)
ALL_OBJS = $(ASM_OBJS) $(CXX_OBJS)

.PHONY: clean prog gdb-server_openocd gdb-client

all: $(TARGET) $(PROJECT).hex

# Compile
$(ASM_OBJS): %.o: %.S
	@echo "[ASM CC] $@"
	@$(CC) $(CFLAGS) $(ASMFLAGS) -c $< -o $@

$(CXX_OBJS): %.o: %.c
	@echo "[CC] $@"
	@$(CC) $(CFLAGS) -c $< -o $@

# Link
%.elf: $(ALL_OBJS)
	@echo "[LD] $@"
	@$(CC) $(CFLAGS) $(LFLAGS) $(ALL_OBJS) -o $@
	@$(SIZE) $@
	@@

%.hex: %.elf
	@echo "[OBJCOPY] $@"
	@$(TOOLCHAIN_ROOT)riscv-none-embed-objcopy -O ihex "$(PROJECT).elf" "$(PROJECT).hex"

# Clean
clean:
	@rm -f $(ALL_OBJS) $(ALL_OBJS:o=d) $(TARGET) $(PROJECT).hex $(PROJECT).map

# Program
flash: $(TARGET)
	sudo $(OPENOCD_ROOT)openocd -f $(OPENOCD_ROOT)wch-riscv.cfg -c init -c halt -c "program $(PROJECT).hex" -c "resume 0" -c exit

gdb:
	sudo $(OPENOCD_ROOT)openocd -f $(OPENOCD_ROOT)wch-riscv.cfg