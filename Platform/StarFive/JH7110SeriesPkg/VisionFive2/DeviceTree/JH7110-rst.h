/** @file
  This header provide all JH7110 reset device tree bindings

  Copyright (c) 2023, StarFive Technology Co., Ltd. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __DT_BINDINGS_JH7110_RESET_H__
#define __DT_BINDINGS_JH7110_RESET_H__

/*
 * group[0]: syscrg: assert0
 */
#define RSTN_U0_JTAG2APB_PRESETN      0
#define RSTN_U0_SYS_SYSCON_PRESETN    1
#define RSTN_U0_SYS_IOMUX_PRESETN     2
#define RSTN_U0_U7MC_RST_BUS          3
#define RSTN_U0_U7MC_DEBUG            4
#define RSTN_U0_U7MC_CORE0            5
#define RSTN_U0_U7MC_CORE1            6
#define RSTN_U0_U7MC_CORE2            7
#define RSTN_U0_U7MC_CORE3            8
#define RSTN_U0_U7MC_CORE4            9
#define RSTN_U0_U7MC_CORE0_ST         10
#define RSTN_U0_U7MC_CORE1_ST         11
#define RSTN_U0_U7MC_CORE2_ST         12
#define RSTN_U0_U7MC_CORE3_ST         13
#define RSTN_U0_U7MC_CORE4_ST         14
#define RSTN_U0_U7MC_TRACE_RST0       15
#define RSTN_U0_U7MC_TRACE_RST1       16
#define RSTN_U0_U7MC_TRACE_RST2       17
#define RSTN_U0_U7MC_TRACE_RST3       18
#define RSTN_U0_U7MC_TRACE_RST4       19
#define RSTN_U0_U7MC_TRACE_COM        20
#define RSTN_U0_IMG_GPU_APB           21
#define RSTN_U0_IMG_GPU_DOMA          22
#define RSTN_U0_NOC_BUS_APB_BUS_N     23
#define RSTN_U0_NOC_BUS_AXICFG0_AXI_N 24
#define RSTN_U0_NOC_BUS_CPU_AXI_N     25
#define RSTN_U0_NOC_BUS_DISP_AXI_N    26
#define RSTN_U0_NOC_BUS_GPU_AXI_N     27
#define RSTN_U0_NOC_BUS_ISP_AXI_N     28
#define RSTN_U0_NOC_BUS_DDRC_N        29
#define RSTN_U0_NOC_BUS_STG_AXI_N     30
#define RSTN_U0_NOC_BUS_VDEC_AXI_N    31
/*
 * group[1]: syscrg: assert1
 */
#define RSTN_U0_NOC_BUS_VENC_AXI_N    32
#define RSTN_U0_AXI_CFG1_DEC_AHB      33
#define RSTN_U0_AXI_CFG1_DEC_MAIN     34
#define RSTN_U0_AXI_CFG0_DEC_MAIN     35
#define RSTN_U0_AXI_CFG0_DEC_MAIN_DIV 36
#define RSTN_U0_AXI_CFG0_DEC_HIFI4    37
#define RSTN_U0_DDR_AXI               38
#define RSTN_U0_DDR_OSC               39
#define RSTN_U0_DDR_APB               40
#define RSTN_U0_DOM_ISP_TOP_N         41
#define RSTN_U0_DOM_ISP_TOP_AXI       42
#define RSTN_U0_DOM_VOUT_TOP_SRC      43
#define RSTN_U0_CODAJ12_AXI           44
#define RSTN_U0_CODAJ12_CORE          45
#define RSTN_U0_CODAJ12_APB           46
#define RSTN_U0_WAVE511_AXI           47
#define RSTN_U0_WAVE511_BPU           48
#define RSTN_U0_WAVE511_VCE           49
#define RSTN_U0_WAVE511_APB           50
#define RSTN_U0_VDEC_JPG_ARB_JPG      51
#define RSTN_U0_VDEC_JPG_ARB_MAIN     52
#define RSTN_U0_AXIMEM_128B_AXI       53
#define RSTN_U0_WAVE420L_AXI          54
#define RSTN_U0_WAVE420L_BPU          55
#define RSTN_U0_WAVE420L_VCE          56
#define RSTN_U0_WAVE420L_APB          57
#define RSTN_U1_AXIMEM_128B_AXI       58
#define RSTN_U2_AXIMEM_128B_AXI       59
#define RSTN_U0_INTMEM_ROM_SRAM_ROM   60
#define RSTN_U0_CDNS_QSPI_AHB         61
#define RSTN_U0_CDNS_QSPI_APB         62
#define RSTN_U0_CDNS_QSPI_REF         63
/*
 * group[2]: syscrg: assert2
 */
#define RSTN_U0_DW_SDIO_AHB        64
#define RSTN_U1_DW_SDIO_AHB        65
#define RSTN_U1_DW_GMAC5_AXI64_A_I 66
#define RSTN_U1_DW_GMAC5_AXI64_H_N 67
#define RSTN_U0_MAILBOX_RRESETN    68
#define RSTN_U0_SSP_SPI_APB        69
#define RSTN_U1_SSP_SPI_APB        70
#define RSTN_U2_SSP_SPI_APB        71
#define RSTN_U3_SSP_SPI_APB        72
#define RSTN_U4_SSP_SPI_APB        73
#define RSTN_U5_SSP_SPI_APB        74
#define RSTN_U6_SSP_SPI_APB        75
#define RSTN_U0_DW_I2C_APB         76
#define RSTN_U1_DW_I2C_APB         77
#define RSTN_U2_DW_I2C_APB         78
#define RSTN_U3_DW_I2C_APB         79
#define RSTN_U4_DW_I2C_APB         80
#define RSTN_U5_DW_I2C_APB         81
#define RSTN_U6_DW_I2C_APB         82
#define RSTN_U0_DW_UART_APB        83
#define RSTN_U0_DW_UART_CORE       84
#define RSTN_U1_DW_UART_APB        85
#define RSTN_U1_DW_UART_CORE       86
#define RSTN_U2_DW_UART_APB        87
#define RSTN_U2_DW_UART_CORE       88
#define RSTN_U3_DW_UART_APB        89
#define RSTN_U3_DW_UART_CORE       90
#define RSTN_U4_DW_UART_APB        91
#define RSTN_U4_DW_UART_CORE       92
#define RSTN_U5_DW_UART_APB        93
#define RSTN_U5_DW_UART_CORE       94
#define RSTN_U0_CDNS_SPDIF_APB     95
/*
 * group[3]: syscrg: assert3
 */
#define RSTN_U0_PWMDAC_APB           96
#define RSTN_U0_PDM_4MIC_DMIC        97
#define RSTN_U0_PDM_4MIC_APB         98
#define RSTN_U0_I2SRX_3CH_APB        99
#define RSTN_U0_I2SRX_3CH_BCLK       100
#define RSTN_U0_I2STX_4CH_APB        101
#define RSTN_U0_I2STX_4CH_BCLK       102
#define RSTN_U1_I2STX_4CH_APB        103
#define RSTN_U1_I2STX_4CH_BCLK       104
#define RSTN_U0_TDM16SLOT_AHB        105
#define RSTN_U0_TDM16SLOT_TDM        106
#define RSTN_U0_TDM16SLOT_APB        107
#define RSTN_U0_PWM_8CH_APB          108
#define RSTN_U0_DSKIT_WDT_APB        109
#define RSTN_U0_DSKIT_WDT_CORE       110
#define RSTN_U0_CAN_CTRL_APB         111
#define RSTN_U0_CAN_CTRL_CORE        112
#define RSTN_U0_CAN_CTRL_TIMER       113
#define RSTN_U1_CAN_CTRL_APB         114
#define RSTN_U1_CAN_CTRL_CORE        115
#define RSTN_U1_CAN_CTRL_TIMER       116
#define RSTN_U0_TIMER_APB            117
#define RSTN_U0_TIMER_TIMER0         118
#define RSTN_U0_TIMER_TIMER1         119
#define RSTN_U0_TIMER_TIMER2         120
#define RSTN_U0_TIMER_TIMER3         121
#define RSTN_U0_INT_CTRL_APB         122
#define RSTN_U0_TEMP_SENSOR_APB      123
#define RSTN_U0_TEMP_SENSOR_TEMP     124
#define RSTN_U0_JTAG_CERTIFICATION_N 125
/*
 * group[4]: stgcrg
 */
#define RSTN_U0_STG_SYSCON_PRESETN 128
#define RSTN_U0_HIFI4_CORE         129
#define RSTN_U0_HIFI4_AXI          130
#define RSTN_U0_SEC_TOP_HRESETN    131
#define RSTN_U0_E24_CORE           132
#define RSTN_U0_DW_DMA1P_AXI       133
#define RSTN_U0_DW_DMA1P_AHB       134
#define RSTN_U0_CDN_USB_AXI        135
#define RSTN_U0_CDN_USB_APB        136
#define RSTN_U0_CDN_USB_UTMI_APB   137
#define RSTN_U0_CDN_USB_PWRUP      138
#define RSTN_U0_PLDA_PCIE_AXI_MST0 139
#define RSTN_U0_PLDA_PCIE_AXI_SLV0 140
#define RSTN_U0_PLDA_PCIE_AXI_SLV  141
#define RSTN_U0_PLDA_PCIE_BRG      142
#define RSTN_U0_PLDA_PCIE_CORE     143
#define RSTN_U0_PLDA_PCIE_APB      144
#define RSTN_U1_PLDA_PCIE_AXI_MST0 145
#define RSTN_U1_PLDA_PCIE_AXI_SLV0 146
#define RSTN_U1_PLDA_PCIE_AXI_SLV  147
#define RSTN_U1_PLDA_PCIE_BRG      148
#define RSTN_U1_PLDA_PCIE_CORE     149
#define RSTN_U1_PLDA_PCIE_APB      150
/*
 * group[5]: aoncrg
 */
#define RSTN_U0_DW_GMAC5_AXI64_AXI 160
#define RSTN_U0_DW_GMAC5_AXI64_AHB 161
#define RSTN_U0_AON_IOMUX_PRESETN  162
#define RSTN_U0_PMU_APB            163
#define RSTN_U0_PMU_WKUP           164
#define RSTN_U0_RTC_HMS_APB        165
#define RSTN_U0_RTC_HMS_CAL        166
#define RSTN_U0_RTC_HMS_OSC32K     167
/*
 * group[6]: ispcrg
 */
#define RSTN_U0_ISPV2_TOP_WRAPPER_P   192
#define RSTN_U0_ISPV2_TOP_WRAPPER_C   193
#define RSTN_U0_M31DPHY_HW            194
#define RSTN_U0_M31DPHY_B09_ALWAYS_ON 195
#define RSTN_U0_VIN_N_PCLK            196
#define RSTN_U0_VIN_N_PIXEL_CLK_IF0   197
#define RSTN_U0_VIN_N_PIXEL_CLK_IF1   198
#define RSTN_U0_VIN_N_PIXEL_CLK_IF2   199
#define RSTN_U0_VIN_N_PIXEL_CLK_IF3   200
#define RSTN_U0_VIN_N_SYS_CLK         201
#define RSTN_U0_VIN_P_AXIRD           202
#define RSTN_U0_VIN_P_AXIWR           203
/*
 * group[7]: voutcrg
 */
#define RSTN_U0_DC8200_AXI           224
#define RSTN_U0_DC8200_AHB           225
#define RSTN_U0_DC8200_CORE          226
#define RSTN_U0_CDNS_DSITX_DPI       227
#define RSTN_U0_CDNS_DSITX_APB       228
#define RSTN_U0_CDNS_DSITX_RXESC     229
#define RSTN_U0_CDNS_DSITX_SYS       230
#define RSTN_U0_CDNS_DSITX_TXBYTEHS  231
#define RSTN_U0_CDNS_DSITX_TXESC     232
#define RSTN_U0_HDMI_TX_HDMI         233
#define RSTN_U0_MIPITX_DPHY_SYS      234
#define RSTN_U0_MIPITX_DPHY_TXBYTEHS 235

#define RSTN_JH7110_RESET_END        236

#endif /* __DT_BINDINGS_JH7110_RESET_H__ */
