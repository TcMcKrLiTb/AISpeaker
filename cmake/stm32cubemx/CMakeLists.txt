cmake_minimum_required(VERSION 3.22)
# Enable CMake support for ASM, C and C++ languages
enable_language(C CXX ASM)
# STM32CubeMX generated symbols (macros)
set(MX_Defines_Syms 
	USE_HAL_DRIVER 
	STM32F746xx
    $<$<CONFIG:Debug>:DEBUG>
)

# STM32CubeMX generated include paths
set(MX_Include_Dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../FATFS/Target
    ${CMAKE_CURRENT_SOURCE_DIR}/../../FATFS/App
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LIBJPEG/App
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LIBJPEG/Target
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LWIP/App
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LWIP/Target
    ${CMAKE_CURRENT_SOURCE_DIR}/../../TouchGFX/App
    ${CMAKE_CURRENT_SOURCE_DIR}/../../TouchGFX/target/generated
    ${CMAKE_CURRENT_SOURCE_DIR}/../../TouchGFX/target
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/system
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FatFs/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/lan8742
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/include/netif/ppp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Device/ST/STM32F7xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/include/lwip
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/include/lwip/apps
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/include/lwip/priv
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/include/lwip/prot
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/include/netif
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/include/compat/posix
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/include/compat/posix/net
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/include/compat/stdc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/system/arch
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Include
)

# STM32CubeMX generated application sources
set(MX_Application_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LWIP/App/lwip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LWIP/Target/ethernetif.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../LIBJPEG/App/libjpeg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../TouchGFX/App/app_touchgfx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../TouchGFX/target/generated/TouchGFXConfiguration.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../TouchGFX/target/generated/TouchGFXGeneratedHAL.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../TouchGFX/target/generated/STM32DMA.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../TouchGFX/target/generated/OSWrappers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../TouchGFX/target/TouchGFXHAL.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../TouchGFX/target/STM32TouchController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../TouchGFX/target/TouchGFXGPIO.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../FATFS/App/fatfs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../FATFS/Target/bsp_driver_sd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../FATFS/Target/sd_diskio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../FATFS/Target/fatfs_platform.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/freertos.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/stm32f7xx_it.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/stm32f7xx_hal_msp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/stm32f7xx_hal_timebase_tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/sysmem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/syscalls.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../startup_stm32f746xx.s
)

# STM32 HAL/LL Drivers
set(STM32_Drivers_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/system_stm32f7xx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_cortex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_i2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_i2c_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_exti.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_eth.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_crc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_crc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma2d.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_ll_fmc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_nor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_sram.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_nand.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_sdram.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_ltdc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_ltdc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dsi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_qspi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_sai.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_sai_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_ll_sdmmc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_sd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_mmc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_uart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_uart_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/BSP/Components/lan8742/lan8742.c
)

# Drivers Midllewares


set(LibJPEG_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jcapimin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jcapistd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jdapimin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jdapistd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jcomapi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jcparam.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jctrans.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jdtrans.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jcinit.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jcmaster.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jcmainct.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jcprepct.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jccoefct.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jccolor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jcsample.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jcdctmgr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jfdctint.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jfdctfst.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jfdctflt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jchuff.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jcarith.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jcmarker.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jdatadst.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jdmaster.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jdinput.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jdmainct.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jdcoefct.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jdpostct.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jdmarker.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jdhuff.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jdarith.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jddctmgr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jidctint.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jidctfst.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jidctflt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jdsample.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jdcolor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jdmerge.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jquant1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jquant2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jdatasrc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jaricom.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jerror.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jmemmgr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jutils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LibJPEG/source/jmemnobs.c
)
set(FreeRTOS_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/croutine.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/event_groups.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/list.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/queue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/tasks.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/timers.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS/cmsis_os.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/port.c
)
set(LwIP_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/auth.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/ccp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/chap_ms.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/chap-md5.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/chap-new.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/demand.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/eap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/eui64.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/fsm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/ipcp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/ipv6cp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/lcp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/magic.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/mppe.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/multilink.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/ppp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/pppapi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/pppcrypt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/pppoe.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/pppol2tp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/pppos.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/upap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/utils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/vj.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/bridgeif.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/bridgeif_fdb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ethernet.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/lowpan6.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/lowpan6_ble.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/lowpan6_common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/slipif.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/zepif.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/netif/ppp/ecp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/api/api_lib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/api/api_msg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/api/err.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/api/if_api.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/api/netbuf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/api/netdb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/api/netifapi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/api/sockets.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/api/tcpip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/altcp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/altcp_alloc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/altcp_tcp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/def.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/dns.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/inet_chksum.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/mem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/memp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/netif.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/pbuf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/raw.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/stats.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/sys.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/tcp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/tcp_in.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/tcp_out.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/timeouts.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/udp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv4/autoip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv4/dhcp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv4/etharp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv4/icmp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv4/igmp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv4/ip4.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv4/ip4_addr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv4/ip4_frag.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv6/dhcp6.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv6/ethip6.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv6/icmp6.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv6/inet6.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv6/ip6.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv6/ip6_addr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv6/ip6_frag.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv6/mld6.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/core/ipv6/nd6.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/system/OS/sys_arch.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/LwIP/src/apps/mqtt/mqtt.c
)
set(FatFs_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FatFs/src/diskio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FatFs/src/ff.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FatFs/src/ff_gen_drv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FatFs/src/option/syscall.c
)

# Link directories setup
set(MX_LINK_DIRS

)
# Project static libraries
set(MX_LINK_LIBS 
    STM32_Drivers
    ${TOOLCHAIN_LINK_LIBRARIES}
    LibJPEG	FreeRTOS	LwIP	FatFs	
)
# Interface library for includes and symbols
add_library(stm32cubemx INTERFACE)
target_include_directories(stm32cubemx INTERFACE ${MX_Include_Dirs})
target_compile_definitions(stm32cubemx INTERFACE ${MX_Defines_Syms})

# Create STM32_Drivers static library
add_library(STM32_Drivers OBJECT)
target_sources(STM32_Drivers PRIVATE ${STM32_Drivers_Src})
target_link_libraries(STM32_Drivers PUBLIC stm32cubemx)
# Validate that STM32CubeMX code is compatible with C++ standard
if(CMAKE_CXX_STANDARD LESS 11)
    message(ERROR "Generated code requires C++11 or higher")
endif()

# Create LibJPEG static library
add_library(LibJPEG OBJECT)
target_sources(LibJPEG PRIVATE ${LibJPEG_Src})
target_link_libraries(LibJPEG PUBLIC stm32cubemx)

# Create FreeRTOS static library
add_library(FreeRTOS OBJECT)
target_sources(FreeRTOS PRIVATE ${FreeRTOS_Src})
target_link_libraries(FreeRTOS PUBLIC stm32cubemx)

# Create LwIP static library
add_library(LwIP OBJECT)
target_sources(LwIP PRIVATE ${LwIP_Src})
target_link_libraries(LwIP PUBLIC stm32cubemx)

# Create FatFs static library
add_library(FatFs OBJECT)
target_sources(FatFs PRIVATE ${FatFs_Src})
target_link_libraries(FatFs PUBLIC stm32cubemx)

# Add STM32CubeMX generated application sources to the project
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${MX_Application_Src})

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE ${MX_LINK_DIRS})

# Add libraries to the project
target_link_libraries(${CMAKE_PROJECT_NAME} ${MX_LINK_LIBS})

# Add the map file to the list of files to be removed with 'clean' target
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES ${CMAKE_PROJECT_NAME}.map)

# Validate that STM32CubeMX code is compatible with C standard
if((CMAKE_C_STANDARD EQUAL 90) OR (CMAKE_C_STANDARD EQUAL 99))
    message(ERROR "Generated code requires C11 or higher")
endif()
