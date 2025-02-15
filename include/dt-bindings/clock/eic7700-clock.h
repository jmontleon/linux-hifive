// SPDX-License-Identifier: GPL-2.0
/*
 * ESWIN Clk Provider Driver
 *
 * Copyright 2024, Beijing ESWIN Computing Technology Co., Ltd.. All rights reserved.
 * SPDX-License-Identifier: GPL-2.0
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: HuangYiFeng<huangyifeng@eswincomputing.com>
 */

#ifndef __DTS_EIC7700_CLOCK_H
#define __DTS_EIC7700_CLOCK_H

#define EIC7700_NONE_CLOCK	0

/* fixed rate */
#define EIC7700_XTAL_24M                        1
#define EIC7700_XTAL_32K                        2
#define EIC7700_PLL_CPU                         3	/*for cpu clk*/
#define EIC7700_SPLL0_FOUT1                     4
#define EIC7700_SPLL0_FOUT2                     5
#define EIC7700_SPLL0_FOUT3                     6
#define EIC7700_SPLL1_FOUT1                     7
#define EIC7700_SPLL1_FOUT2                     8
#define EIC7700_SPLL1_FOUT3                     9
#define EIC7700_SPLL2_FOUT1                     10
#define EIC7700_SPLL2_FOUT2                     11
#define EIC7700_SPLL2_FOUT3                     12
#define EIC7700_VPLL_FOUT1                      13
#define EIC7700_VPLL_FOUT2                      14
#define EIC7700_VPLL_FOUT3                      15
#define EIC7700_APLL_FOUT1                      16
#define EIC7700_APLL_FOUT2                      17
#define EIC7700_APLL_FOUT3                      18
#define EIC7700_EXT_MCLK                        19
#define EIC7700_PLL_DDR                         20
#define EIC7700_LPDDR_REF_BAK                   21


/* mux clocks */
#define  EIC7700_MUX_U_CPU_ROOT_3MUX1_GFREE                         30	/*for cpu clk*/
#define  EIC7700_MUX_U_CPU_ACLK_2MUX1_GFREE                         31	/*for cpu clk*/
#define  EIC7700_MUX_U_DSP_ACLK_ROOT_2MUX1_GFREE                    32
#define  EIC7700_MUX_U_D2D_ACLK_ROOT_2MUX1_GFREE                    33
#define  EIC7700_MUX_U_MSHCORE_ROOT_3MUX1_0                         34
#define  EIC7700_MUX_U_MSHCORE_ROOT_3MUX1_1                         35
#define  EIC7700_MUX_U_MSHCORE_ROOT_3MUX1_2                         36
#define  EIC7700_MUX_U_NPU_LLCLK_3MUX1_GFREE                        37
#define  EIC7700_MUX_U_NPU_CORE_3MUX1_GFREE                         38
#define  EIC7700_MUX_U_VI_ACLK_ROOT_2MUX1_GFREE                     39
#define  EIC7700_MUX_U_VI_DVP_ROOT_2MUX1_GFREE                      40
#define  EIC7700_MUX_U_VI_DIG_ISP_ROOT_2MUX1_GFREE                  41
#define  EIC7700_MUX_U_VO_ACLK_ROOT_2MUX1_GFREE                     42
#define  EIC7700_MUX_U_VO_PIXEL_ROOT_2MUX1                          43
#define  EIC7700_MUX_U_VO_CEC_2MUX1                                 44
#define  EIC7700_MUX_U_VCDEC_ROOT_2MUX1_GFREE                       45
#define  EIC7700_MUX_U_VCACLK_ROOT_2MUX1_GFREE                      46
#define  EIC7700_MUX_U_RTC_2MUX1                                    47
#define  EIC7700_MUX_U_SYSCFG_CLK_ROOT_2MUX1_GFREE                  48
#define  EIC7700_MUX_U_NOCNSP_XTAL_2MUX1                            49
#define  EIC7700_MUX_U_BOOTSPI_CLK_2MUX1_GFREE                      50
#define  EIC7700_MUX_U_SCPU_CORE_CLK_2MUX1_GFREE                    51
#define  EIC7700_MUX_U_LPCPU_CORE_CLK_2MUX1_GFREE                   52
#define  EIC7700_MUX_GPU_ACLK_XTAL_2MUX1                            53
#define  EIC7700_MUX_U_DSP_ACLK_XTAL_2MUX1                          54
#define  EIC7700_MUX_U_D2D_ACLK_XTAL_2MUX1                          55
#define  EIC7700_MUX_U_HSP_ACLK_XTAL_2MUX1                          56
#define  EIC7700_MUX_U_PCIE_ACLK_XTAL_2MUX1                         57
#define  EIC7700_MUX_U_NPU_ACLK_XTAL_2MUX1                          58
#define  EIC7700_MUX_U_NPU_LLC_XTAL_2MUX1                           59
#define  EIC7700_MUX_U_NPU_CORE_XTAL_2MUX1                          60
#define  EIC7700_MUX_U_VI_ACLK_XTAL_2MUX1                           61
#define  EIC7700_MUX_U_VI_DVP_XTAL_2MUX1                            62
#define  EIC7700_MUX_U_VI_DIG_ISP_XTAL_2MUX1                        63
#define  EIC7700_MUX_U_VI_SHUTTER_XTAL_2MUX1_0                      64
#define  EIC7700_MUX_U_VI_SHUTTER_XTAL_2MUX1_1                      65
#define  EIC7700_MUX_U_VI_SHUTTER_XTAL_2MUX1_2                      66
#define  EIC7700_MUX_U_VI_SHUTTER_XTAL_2MUX1_3                      67
#define  EIC7700_MUX_U_VI_SHUTTER_XTAL_2MUX1_4                      68
#define  EIC7700_MUX_U_VI_SHUTTER_XTAL_2MUX1_5                      69
#define  EIC7700_MUX_U_VO_ACLK_XTAL_2MUX1                           70
#define  EIC7700_MUX_U_IESMCLK_XTAL_2MUX1                           71
#define  EIC7700_MUX_U_VO_PIXEL_XTAL_2MUX1                          72
#define  EIC7700_MUX_U_VO_MCLK_2MUX_EXT_MCLK                        73
#define  EIC7700_MUX_U_VC_ACLK_XTAL_2MUX1                           74
#define  EIC7700_MUX_U_JD_XTAL_2MUX1                                75
#define  EIC7700_MUX_U_JE_XTAL_2MUX1                                76
#define  EIC7700_MUX_U_VE_XTAL_2MUX1                                77
#define  EIC7700_MUX_U_VD_XTAL_2MUX1                                78
#define  EIC7700_MUX_U_SATA_PHY_2MUX1                               79
#define  EIC7700_MUX_U_AONDMA_AXI2MUX1_GFREE                        80
#define  EIC7700_MUX_U_CRYPTO_XTAL_2MUX1                            81
#define  EIC7700_MUX_U_RMII_REF_2MUX                                82
#define  EIC7700_MUX_U_ETH_CORE_2MUX1                               83
#define  EIC7700_MUX_U_VI_DW_ROOT_2MUX1                             84
#define  EIC7700_MUX_U_VI_DW_XTAL_2MUX1                             85
#define  EIC7700_MUX_U_NPU_E31_3MUX1_GFREE                          86
#define  EIC7700_MUX_U_DDR_ACLK_ROOT_2MUX1_GFREE                    87

/* divider clocks */
#define EIC7700_DIVDER_U_SYS_CFG_DIV_DYNM                           100
#define EIC7700_DIVDER_U_NOC_NSP_DIV_DYNM                           101
#define EIC7700_DIVDER_U_BOOTSPI_DIV_DYNM                           102
#define EIC7700_DIVDER_U_SCPU_CORE_DIV_DYNM                         103
#define EIC7700_DIVDER_U_LPCPU_CORE_DIV_DYNM                        104
#define EIC7700_DIVDER_U_GPU_ACLK_DIV_DYNM                          105
#define EIC7700_DIVDER_U_DSP_ACLK_DIV_DYNM                          106
#define EIC7700_DIVDER_U_D2D_ACLK_DIV_DYNM                          107
#define EIC7700_DIVDER_U_HSP_ACLK_DIV_DYNM                          108
#define EIC7700_DIVDER_U_ETH_TXCLK_DIV_DYNM_0                       109
#define EIC7700_DIVDER_U_ETH_TXCLK_DIV_DYNM_1                       110
#define EIC7700_DIVDER_U_MSHC_CORE_DIV_DYNM_0                       111
#define EIC7700_DIVDER_U_MSHC_CORE_DIV_DYNM_1                       112
#define EIC7700_DIVDER_U_MSHC_CORE_DIV_DYNM_2                       113
#define EIC7700_DIVDER_U_PCIE_ACLK_DIV_DYNM                         114
#define EIC7700_DIVDER_U_NPU_ACLK_DIV_DYNM                          115
#define EIC7700_DIVDER_U_NPU_LLC_SRC0_DIV_DYNM                      116
#define EIC7700_DIVDER_U_NPU_LLC_SRC1_DIV_DYNM                      117
#define EIC7700_DIVDER_U_NPU_CORECLK_DIV_DYNM                       118
#define EIC7700_DIVDER_U_VI_ACLK_DIV_DYNM                           119
#define EIC7700_DIVDER_U_VI_DVP_DIV_DYNM                            120
#define EIC7700_DIVDER_U_VI_DIG_ISP_DIV_DYNM                        121
#define EIC7700_DIVDER_U_VI_SHUTTER_DIV_DYNM_0                      122
#define EIC7700_DIVDER_U_VI_SHUTTER_DIV_DYNM_1                      123
#define EIC7700_DIVDER_U_VI_SHUTTER_DIV_DYNM_2                      124
#define EIC7700_DIVDER_U_VI_SHUTTER_DIV_DYNM_3                      125
#define EIC7700_DIVDER_U_VI_SHUTTER_DIV_DYNM_4                      126
#define EIC7700_DIVDER_U_VI_SHUTTER_DIV_DYNM_5                      127
#define EIC7700_DIVDER_U_VO_ACLK_DIV_DYNM                           128
#define EIC7700_DIVDER_U_IESMCLK_DIV_DYNM                           129
#define EIC7700_DIVDER_U_VO_PIXEL_DIV_DYNM                          130
#define EIC7700_DIVDER_U_VO_MCLK_DIV_DYNM                           131
#define EIC7700_DIVDER_U_VC_ACLK_DIV_DYNM                           132
#define EIC7700_DIVDER_U_JD_DIV_DYNM                                133
#define EIC7700_DIVDER_U_JE_DIV_DYNM                                134
#define EIC7700_DIVDER_U_VE_DIV_DYNM                                135
#define EIC7700_DIVDER_U_VD_DIV_DYNM                                136
#define EIC7700_DIVDER_U_G2D_DIV_DYNM                               137
#define EIC7700_DIVDER_U_AONDMA_AXI_DIV_DYNM                        138
#define EIC7700_DIVDER_U_CRYPTO_DIV_DYNM                            139
#define EIC7700_DIVDER_U_VI_DW_DIV_DYNM                             140
#define EIC7700_DIVDER_U_NPU_E31_DIV_DYNM                           141
#define EIC7700_DIVDER_U_SATA_PHY_REF_DIV_DYNM                      142
#define EIC7700_DIVDER_U_DSP_0_ACLK_DIV_DYNM                        143
#define EIC7700_DIVDER_U_DSP_1_ACLK_DIV_DYNM                        144
#define EIC7700_DIVDER_U_DSP_2_ACLK_DIV_DYNM                        145
#define EIC7700_DIVDER_U_DSP_3_ACLK_DIV_DYNM                        146
#define EIC7700_DIVDER_U_DDR_ACLK_DIV_DYNM                          147
#define EIC7700_DIVDER_U_AON_RTC_DIV_DYNM                           148
#define EIC7700_DIVDER_U_U84_RTC_TOGGLE_DIV_DYNM                    149
#define EIC7700_DIVDER_U_VO_CEC_DIV_DYNM                            150

/* gate clocks */
#define EIC7700_GATE_CLK_CPU_EXT_SRC_CORE_CLK_0                 200
#define EIC7700_GATE_CLK_CPU_EXT_SRC_CORE_CLK_1                 201
#define EIC7700_GATE_CLK_CPU_EXT_SRC_CORE_CLK_2                 202
#define EIC7700_GATE_CLK_CPU_EXT_SRC_CORE_CLK_3                 203
#define EIC7700_GATE_CLK_CPU_TRACE_CLK_0                        204
#define EIC7700_GATE_CLK_CPU_TRACE_CLK_1                        205
#define EIC7700_GATE_CLK_CPU_TRACE_CLK_2                        206
#define EIC7700_GATE_CLK_CPU_TRACE_CLK_3                        207
#define EIC7700_GATE_CLK_CPU_DEBUG_CLK                          208
#define EIC7700_GATE_CLK_CPU_TRACE_COM_CLK                      209
#define EIC7700_GATE_CLK_CPU_CLK                                210
#define EIC7700_GATE_CLK_SPLL0_FOUT2                            211
#define EIC7700_GATE_CLK_VPLL_FOUT2                             212
#define EIC7700_GATE_CLK_VPLL_FOUT3                             213
#define EIC7700_GATE_CLK_APLL_FOUT1                             214
#define EIC7700_GATE_CLK_APLL_FOUT2                             215
#define EIC7700_GATE_CLK_APLL_FOUT3                             216
#define EIC7700_GATE_EXT_MCLK                                   217
#define EIC7700_GATE_CLK_1M                                     218
#define EIC7700_GATE_CLK_SYS_CFG                                219
#define EIC7700_GATE_CLK_MIPI_TXESC                             220
#define EIC7700_GATE_NOC_CFG_CLK                                221
#define EIC7700_GATE_NOC_NSP_CLK                                222
#define EIC7700_GATE_CLK_BOOTSPI                                223
#define EIC7700_GATE_CLK_BOOTSPI_CFG                            224
#define EIC7700_GATE_CLK_U84_CORE_LP                            225
#define EIC7700_GATE_CLK_SCPU_CORE                              226
#define EIC7700_GATE_CLK_SCPU_BUS                               227
#define EIC7700_GATE_CLK_LPCPU_CORE                             228
#define EIC7700_GATE_CLK_LPCPU_BUS                              229
#define EIC7700_GATE_GPU_ACLK                                   230
#define EIC7700_GATE_GPU_GRAY_CLK                               231
#define EIC7700_GATE_GPU_CFG_CLK                                232
#define EIC7700_GATE_CLK_DSP_ROOT                               233
#define EIC7700_GATE_DSPT_ACLK                                  234
#define EIC7700_GATE_DSPT_CFG_CLK                               235
#define EIC7700_GATE_CLK_D2DDR_ACLK                             236
#define EIC7700_GATE_D2D_ACLK                                   237
#define EIC7700_GATE_D2D_CFG_CLK                                238
#define EIC7700_GATE_CLK_HSP_ACLK                               239
#define EIC7700_GATE_CLK_HSP_CFGCLK                             240
#define EIC7700_GATE_TCU_ACLK                                   241
#define EIC7700_GATE_TCU_CFG_CLK                                242
#define EIC7700_GATE_DDRT_CFG_CLK                               243
#define EIC7700_GATE_DDRT1_CFG_CLK                              244
#define EIC7700_GATE_DDRT0_P0_ACLK                              245
#define EIC7700_GATE_DDRT0_P1_ACLK                              246
#define EIC7700_GATE_DDRT0_P2_ACLK                              247
#define EIC7700_GATE_DDRT0_P3_ACLK                              248
#define EIC7700_GATE_DDRT0_P4_ACLK                              249
#define EIC7700_GATE_DDRT1_P0_ACLK                              250
#define EIC7700_GATE_DDRT1_P1_ACLK                              251
#define EIC7700_GATE_DDRT1_P2_ACLK                              252
#define EIC7700_GATE_DDRT1_P3_ACLK                              253
#define EIC7700_GATE_DDRT1_P4_ACLK                              254
#define EIC7700_GATE_HSP_ACLK                                   255
#define EIC7700_GATE_HSP_CFG_CLK                                256
#define EIC7700_GATE_HSP_SATA_RBC_CLK                           257
#define EIC7700_GATE_HSP_SATA_OOB_CLK                           258
#define EIC7700_GATE_HSP_SATA_PMALIVE_CLK                       259
#define EIC7700_GATE_HSP_ETH_APP_CLK                            260
#define EIC7700_GATE_HSP_ETH_CSR_CLK                            261
#define EIC7700_GATE_HSP_ETH0_CORE_CLK                          262
#define EIC7700_GATE_HSP_ETH1_CORE_CLK                          263
#define EIC7700_GATE_HSP_MSHC0_CORE_CLK                         264
#define EIC7700_GATE_HSP_MSHC1_CORE_CLK                         265
#define EIC7700_GATE_HSP_MSHC2_CORE_CLK                         266
#define EIC7700_GATE_HSP_MSHC0_TMR_CLK                          267
#define EIC7700_GATE_HSP_MSHC1_TMR_CLK                          268
#define EIC7700_GATE_HSP_MSHC2_TMR_CLK                          269
#define EIC7700_GATE_HSP_USB0_SUSPEND_CLK                       270
#define EIC7700_GATE_HSP_USB1_SUSPEND_CLK                       271
#define EIC7700_GATE_PCIET_ACLK                                 272
#define EIC7700_GATE_PCIET_CFG_CLK                              273
#define EIC7700_GATE_PCIET_CR_CLK                               274
#define EIC7700_GATE_PCIET_AUX_CLK                              275
#define EIC7700_GATE_NPU_ACLK                                   276
#define EIC7700_GATE_NPU_CFG_CLK                                277
#define EIC7700_GATE_CLK_NPU_LLC_SRC0                           278
#define EIC7700_GATE_CLK_NPU_LLC_SRC1                           279
#define EIC7700_GATE_NPU_LLC_ACLK                               280
#define EIC7700_GATE_CLK_NPU_CORE_ST1                           281
#define EIC7700_GATE_NPU_CLK                                    282
#define EIC7700_GATE_NPU_E31_CLK                                283
#define EIC7700_GATE_CLK_VI_ACLK_ST1                            284
#define EIC7700_GATE_VI_ACLK                                    285
#define EIC7700_GATE_VI_DVP_CLK                                 286
#define EIC7700_GATE_VI_CFG_CLK                                 287
#define EIC7700_GATE_VI_DIG_DW_CLK                              288
#define EIC7700_GATE_VI_DIG_ISP_CLK                             289
#define EIC7700_GATE_VI_SHUTTER_0                               290
#define EIC7700_GATE_VI_SHUTTER_1                               291
#define EIC7700_GATE_VI_SHUTTER_2                               292
#define EIC7700_GATE_VI_SHUTTER_3                               293
#define EIC7700_GATE_VI_SHUTTER_4                               294
#define EIC7700_GATE_VI_SHUTTER_5                               295
#define EIC7700_GATE_VI_PHY_TXCLKESC                            296
#define EIC7700_GATE_VI_PHY_CFG                                 297
#define EIC7700_GATE_VO_ACLK                                    298
#define EIC7700_GATE_VO_CFG_CLK                                 299
#define EIC7700_GATE_VO_HDMI_IESMCLK                            300
#define EIC7700_GATE_VO_PIXEL_CLK                               301
#define EIC7700_GATE_VO_I2S_MCLK                                302
#define EIC7700_GATE_VO_CR_CLK                                  303
#define EIC7700_GATE_VO_CEC_CLK                                 304
#define EIC7700_GATE_CLK_VC_ROOT                                305
#define EIC7700_GATE_VC_ACLK                                    306
#define EIC7700_GATE_VC_CFG_CLK                                 307
#define EIC7700_GATE_VC_JE_CLK                                  308
#define EIC7700_GATE_VC_JD_CLK                                  309
#define EIC7700_GATE_VC_VE_CLK                                  310
#define EIC7700_GATE_VC_VD_CLK                                  311
#define EIC7700_GATE_G2D_CFG_CLK                                312
#define EIC7700_GATE_CLK_G2D_ST2                                313
#define EIC7700_GATE_G2D_CLK                                    314
#define EIC7700_GATE_G2D_ACLK                                   315
#define EIC7700_GATE_CLK_PVT_INNER                              316
#define EIC7700_GATE_PVT_CLK_0                                  317
#define EIC7700_GATE_PVT_CLK_1                                  318
#define EIC7700_GATE_PVT_CLK_2                                  319
#define EIC7700_GATE_PVT_CLK_3                                  320
#define EIC7700_GATE_PVT_CLK_4                                  321
#define EIC7700_GATE_CLK_AONDMA_CFG                             322
#define EIC7700_GATE_CLK_AONDMA_AXI_ST3                         323
#define EIC7700_GATE_AONDMA_ACLK                                324
#define EIC7700_GATE_AON_ACLK                                   325
#define EIC7700_GATE_TIMER_CLK_0                                326
#define EIC7700_GATE_TIMER_CLK_1                                327
#define EIC7700_GATE_TIMER_CLK_2                                328
#define EIC7700_GATE_TIMER_CLK_3                                329
#define EIC7700_GATE_TIMER_PCLK_0                               330
#define EIC7700_GATE_TIMER_PCLK_1                               331
#define EIC7700_GATE_TIMER_PCLK_2                               332
#define EIC7700_GATE_TIMER_PCLK_3                               333
#define EIC7700_GATE_TIMER3_CLK8                                334
#define EIC7700_GATE_CLK_RTC_CFG                                335
#define EIC7700_GATE_CLK_RTC                                    336
#define EIC7700_GATE_HSP_RMII_REF_0                             337
#define EIC7700_GATE_HSP_RMII_REF_1                             338
#define EIC7700_GATE_CLK_PKA_CFG                                339
#define EIC7700_GATE_CLK_SPACC_CFG                              340
#define EIC7700_GATE_CLK_CRYPTO                                 341
#define EIC7700_GATE_CLK_TRNG_CFG                               342
#define EIC7700_GATE_CLK_OTP_CFG                                343
#define EIC7700_GATE_CLMM_CFG_CLK                               344
#define EIC7700_GATE_CLMM_DEB_CLK                               345
#define EIC7700_GATE_CLK_MAILBOX_0                              346
#define EIC7700_GATE_CLK_MAILBOX_1                              347
#define EIC7700_GATE_CLK_MAILBOX_2                              348
#define EIC7700_GATE_CLK_MAILBOX_3                              349
#define EIC7700_GATE_CLK_MAILBOX_4                              350
#define EIC7700_GATE_CLK_MAILBOX_5                              351
#define EIC7700_GATE_CLK_MAILBOX_6                              352
#define EIC7700_GATE_CLK_MAILBOX_7                              353
#define EIC7700_GATE_CLK_MAILBOX_8                              354
#define EIC7700_GATE_CLK_MAILBOX_9                              355
#define EIC7700_GATE_CLK_MAILBOX_10                             356
#define EIC7700_GATE_CLK_MAILBOX_11                             357
#define EIC7700_GATE_CLK_MAILBOX_12                             358
#define EIC7700_GATE_CLK_MAILBOX_13                             359
#define EIC7700_GATE_CLK_MAILBOX_14                             360
#define EIC7700_GATE_CLK_MAILBOX_15                             361
#define EIC7700_GATE_CLK_APLL_TEST_OUT                          362
#define EIC7700_GATE_CLK_CPLL_TEST_OUT                          363
#define EIC7700_GATE_CLK_HSP_DFT150M                            364
#define EIC7700_GATE_CLK_HSP_DFT300M                            365
#define EIC7700_GATE_CLK_HSP_DFT600M                            366
#define EIC7700_GATE_CLK_VI_DFT400M                             367
#define EIC7700_GATE_CLK_VI_DFT500M                             368
#define EIC7700_GATE_CLK_VO_DFT300M                             369
#define EIC7700_GATE_CLK_VO_DFT600M                             370
#define EIC7700_GATE_CLK_D2D_DFT300M                            371
#define EIC7700_GATE_CLK_D2D_DFT600M                            372
#define EIC7700_GATE_CLK_PCIE_DFT125M                           373
#define EIC7700_GATE_CLK_PCIE_DFT200M                           374
#define EIC7700_GATE_CLK_DDR_PLL_BYP_CLK                        375
#define EIC7700_GATE_CLK_DDR_RX_TEST_CLK                        376
#define EIC7700_GATE_LSP_I2C0_PCLK                              377
#define EIC7700_GATE_LSP_I2C1_PCLK                              378
#define EIC7700_GATE_LSP_I2C2_PCLK                              379
#define EIC7700_GATE_LSP_I2C3_PCLK                              380
#define EIC7700_GATE_LSP_I2C4_PCLK                              381
#define EIC7700_GATE_LSP_I2C5_PCLK                              382
#define EIC7700_GATE_LSP_I2C6_PCLK                              383
#define EIC7700_GATE_LSP_I2C7_PCLK                              384
#define EIC7700_GATE_LSP_I2C8_PCLK                              385
#define EIC7700_GATE_LSP_I2C9_PCLK                              386
#define EIC7700_GATE_LSP_WDT0_PCLK                              387
#define EIC7700_GATE_LSP_WDT1_PCLK                              388
#define EIC7700_GATE_LSP_WDT2_PCLK                              389
#define EIC7700_GATE_LSP_WDT3_PCLK                              390
#define EIC7700_GATE_LSP_SSI0_PCLK                              391
#define EIC7700_GATE_LSP_SSI1_PCLK                              392
#define EIC7700_GATE_LSP_PVT_PCLK                               393
#define EIC7700_GATE_AON_I2C0_PCLK                              394
#define EIC7700_GATE_AON_I2C1_PCLK                              395
#define EIC7700_GATE_LSP_UART0_PCLK                             396
#define EIC7700_GATE_LSP_UART1_PCLK                             397
#define EIC7700_GATE_LSP_UART2_PCLK                             398
#define EIC7700_GATE_LSP_UART3_PCLK                             399
#define EIC7700_GATE_LSP_UART4_PCLK                             400
#define EIC7700_GATE_LSP_TIMER_PCLK                             401
#define EIC7700_GATE_LSP_FAN_PCLK                               402
#define EIC7700_GATE_LSP_PVT0_CLK                               403
#define EIC7700_GATE_LSP_PVT1_CLK                               404
#define EIC7700_GATE_RESERVED_1                                 405
#define EIC7700_GATE_RESERVED_2                                 406
#define EIC7700_GATE_RESERVED_3                                 407
#define EIC7700_GATE_VC_JE_PCLK                                 408
#define EIC7700_GATE_VC_JD_PCLK                                 409
#define EIC7700_GATE_VC_VE_PCLK                                 410
#define EIC7700_GATE_VC_VD_PCLK                                 411
#define EIC7700_GATE_VC_MON_PCLK                                412
#define EIC7700_GATE_HSP_DMA0_CLK                               413

/*fixed factor clocks*/
#define EIC7700_FIXED_FACTOR_U_CPU_DIV2                         450
#define EIC7700_FIXED_FACTOR_U_CLK_1M_DIV24                     451
#define EIC7700_FIXED_FACTOR_U_MIPI_TXESC_DIV10                 452
#define EIC7700_FIXED_FACTOR_U_U84_CORE_LP_DIV2                 453
#define EIC7700_FIXED_FACTOR_U_SCPU_BUS_DIV2                    454
#define EIC7700_FIXED_FACTOR_U_LPCPU_BUS_DIV2                   455
#define EIC7700_FIXED_FACTOR_U_PCIE_CR_DIV2                     456
#define EIC7700_FIXED_FACTOR_U_PCIE_AUX_DIV4                    457
#define EIC7700_FIXED_FACTOR_U_PVT_DIV20                        458
#define EIC7700_FIXED_FACTOR_U_DFT100M_DIV4                     459
#define EIC7700_FIXED_FACTOR_U_DFT125M_DIV2                     460
#define EIC7700_FIXED_FACTOR_U_DFT150M_DIV2                     461
#define EIC7700_FIXED_FACTOR_U_DFT100M_DIV2                     462
#define EIC7700_FIXED_FACTOR_U_DFT500M_DIV3                     463
#define EIC7700_FIXED_FACTOR_U_DFT500M_DIV2                     464
#define EIC7700_FIXED_FACTOR_SPLL0_TEST_DIV8                    465
#define EIC7700_FIXED_FACTOR_SPLL1_TEST_DIV6                    466
#define EIC7700_FIXED_FACTOR_SPLL2_TEST_DIV4                    467
#define EIC7700_FIXED_FACTOR_U_HSP_RMII_REF_DIV6                468
#define EIC7700_FIXED_FACTOR_U_DRR_DIV8                         469


/*clocks list for consumer*/
#define EIC7700_CLK_CPU_EXT_SRC_CORE_CLK_0                      500
#define EIC7700_CLK_CPU_EXT_SRC_CORE_CLK_1                      501
#define EIC7700_CLK_CPU_EXT_SRC_CORE_CLK_2                      502
#define EIC7700_CLK_CPU_EXT_SRC_CORE_CLK_3                      503
#define EIC7700_CLK_CPU_TRACE_CLK_0                             504
#define EIC7700_CLK_CPU_TRACE_CLK_1                             505
#define EIC7700_CLK_CPU_TRACE_CLK_2                             506
#define EIC7700_CLK_CPU_TRACE_CLK_3                             507
#define EIC7700_CLK_CPU_DEBUG_CLK                               508
#define EIC7700_CLK_CPU_TRACE_COM_CLK                           509
#define EIC7700_CLK_CPU_CLK                                     510
#define EIC7700_CLK_CLK_1M                                      511
#define EIC7700_CLK_CLK_SYS_CFG                                 512
#define EIC7700_CLK_CLK_MIPI_TXESC                              513
#define EIC7700_CLK_NOC_CFG_CLK                                 514
#define EIC7700_CLK_NOC_NSP_CLK                                 515
#define EIC7700_CLK_CLK_BOOTSPI                                 516
#define EIC7700_CLK_CLK_BOOTSPI_CFG                             517
#define EIC7700_CLK_CLK_U84_CORE_LP                             518
#define EIC7700_CLK_CLK_SCPU_CORE                               519
#define EIC7700_CLK_CLK_SCPU_BUS                                520
#define EIC7700_CLK_CLK_LPCPU_CORE                              521
#define EIC7700_CLK_CLK_LPCPU_BUS                               522
#define EIC7700_CLK_GPU_ACLK                                    523
#define EIC7700_CLK_GPU_GRAY_CLK                                524
#define EIC7700_CLK_GPU_CFG_CLK                                 525
#define EIC7700_CLK_CLK_DSP_ROOT                                526
#define EIC7700_CLK_DSPT_ACLK                                   527
#define EIC7700_CLK_DSPT_CFG_CLK                                528
#define EIC7700_CLK_CLK_D2DDR_ACLK                              529
#define EIC7700_CLK_D2D_ACLK                                    530
#define EIC7700_CLK_D2D_CFG_CLK                                 531
#define EIC7700_CLK_TCU_ACLK                                    532
#define EIC7700_CLK_TCU_CFG_CLK                                 533
#define EIC7700_CLK_DDRT_CFG_CLK                                534
#define EIC7700_CLK_DDRT0_P0_ACLK                               535
#define EIC7700_CLK_DDRT0_P1_ACLK                               536
#define EIC7700_CLK_DDRT0_P2_ACLK                               537
#define EIC7700_CLK_DDRT0_P3_ACLK                               538
#define EIC7700_CLK_DDRT0_P4_ACLK                               539
#define EIC7700_CLK_DDRT1_P0_ACLK                               540
#define EIC7700_CLK_DDRT1_P1_ACLK                               541
#define EIC7700_CLK_DDRT1_P2_ACLK                               542
#define EIC7700_CLK_DDRT1_P3_ACLK                               543
#define EIC7700_CLK_DDRT1_P4_ACLK                               544
#define EIC7700_CLK_HSP_ACLK                                    545
#define EIC7700_CLK_HSP_CFG_CLK                                 546
#define EIC7700_CLK_HSP_SATA_RBC_CLK                            547
#define EIC7700_CLK_HSP_SATA_OOB_CLK                            548
#define EIC7700_CLK_HSP_SATA_PMALIVE_CLK                        549
#define EIC7700_CLK_HSP_ETH_APP_CLK                             550
#define EIC7700_CLK_HSP_ETH_CSR_CLK                             551
#define EIC7700_CLK_HSP_ETH0_CORE_CLK                           552
#define EIC7700_CLK_HSP_ETH1_CORE_CLK                           553
#define EIC7700_CLK_HSP_MSHC0_CORE_CLK                          554
#define EIC7700_CLK_HSP_MSHC1_CORE_CLK                          555
#define EIC7700_CLK_HSP_MSHC2_CORE_CLK                          556
#define EIC7700_CLK_HSP_MSHC0_TMR_CLK                           557
#define EIC7700_CLK_HSP_MSHC1_TMR_CLK                           558
#define EIC7700_CLK_HSP_MSHC2_TMR_CLK                           559
#define EIC7700_CLK_HSP_USB0_SUSPEND_CLK                        560
#define EIC7700_CLK_HSP_USB1_SUSPEND_CLK                        561
#define EIC7700_CLK_PCIET_ACLK                                  562
#define EIC7700_CLK_PCIET_CFG_CLK                               563
#define EIC7700_CLK_PCIET_CR_CLK                                564
#define EIC7700_CLK_PCIET_AUX_CLK                               565
#define EIC7700_CLK_NPU_ACLK                                    566
#define EIC7700_CLK_NPU_CFG_CLK                                 567
#define EIC7700_CLK_CLK_NPU_LLC_SRC0                            568
#define EIC7700_CLK_CLK_NPU_LLC_SRC1                            569
#define EIC7700_CLK_NPU_LLC_ACLK                                570
#define EIC7700_CLK_CLK_NPU_CORE_ST1                            571
#define EIC7700_CLK_NPU_CLK                                     572
#define EIC7700_CLK_NPU_E31_CLK                                 573
#define EIC7700_CLK_CLK_VI_ACLK_ST1                             574
#define EIC7700_CLK_VI_ACLK                                     575
#define EIC7700_CLK_VI_DVP_CLK                                  576
#define EIC7700_CLK_VI_CFG_CLK                                  577
#define EIC7700_CLK_VI_DIG_DW_CLK                               578
#define EIC7700_CLK_VI_DIG_ISP_CLK                              579
#define EIC7700_CLK_VI_SHUTTER_0                                580
#define EIC7700_CLK_VI_SHUTTER_1                                581
#define EIC7700_CLK_VI_SHUTTER_2                                582
#define EIC7700_CLK_VI_SHUTTER_3                                583
#define EIC7700_CLK_VI_SHUTTER_4                                584
#define EIC7700_CLK_VI_SHUTTER_5                                585
#define EIC7700_CLK_VI_PHY_TXCLKESC                             586
#define EIC7700_CLK_VI_PHY_CFG                                  587
#define EIC7700_CLK_VO_ACLK                                     588
#define EIC7700_CLK_VO_CFG_CLK                                  589
#define EIC7700_CLK_VO_HDMI_IESMCLK                             590
#define EIC7700_CLK_VO_PIXEL_CLK                                591
#define EIC7700_CLK_VO_I2S_MCLK                                 592
#define EIC7700_CLK_VO_CR_CLK                                   593
#define EIC7700_CLK_VO_CEC_CLK                                  594
#define EIC7700_CLK_CLK_VC_ROOT                                 595
#define EIC7700_CLK_VC_ACLK                                     596
#define EIC7700_CLK_VC_CFG_CLK                                  597
#define EIC7700_CLK_VC_JE_CLK                                   598
#define EIC7700_CLK_VC_JD_CLK                                   599
#define EIC7700_CLK_VC_VE_CLK                                   600
#define EIC7700_CLK_VC_VD_CLK                                   601
#define EIC7700_CLK_G2D_CFG_CLK                                 602
#define EIC7700_CLK_CLK_G2D_ST2                                 603
#define EIC7700_CLK_G2D_CLK                                     604
#define EIC7700_CLK_G2D_ACLK                                    605
#define EIC7700_CLK_CLK_RESERVED                                606
#define EIC7700_CLK_PVT_CLK_0                                   607
#define EIC7700_CLK_PVT_CLK_1                                   608
#define EIC7700_CLK_HSP_RMII_REF_0                              609
#define EIC7700_CLK_HSP_RMII_REF_1                              610
#define EIC7700_CLK_HSP_SATA_PHY_REF                            611
#define EIC7700_CLK_AONDMA_CFG                                  612
#define EIC7700_CLK_CLK_AONDMA_AXI_ST3                          613
#define EIC7700_CLK_AONDMA_ACLK                                 614
#define EIC7700_CLK_AON_ACLK                                    615
#define EIC7700_CLK_TIMER_CLK_0                                 616      //AON timer
#define EIC7700_CLK_TIMER_CLK_1                                 617
#define EIC7700_CLK_TIMER_CLK_2                                 618
#define EIC7700_CLK_TIMER_CLK_3                                 619
#define EIC7700_CLK_TIMER_PCLK_0                                620
#define EIC7700_CLK_TIMER_PCLK_1                                621
#define EIC7700_CLK_TIMER_PCLK_2                                622
#define EIC7700_CLK_TIMER_PCLK_3                                623
#define EIC7700_CLK_TIMER3_CLK8                                 624
#define EIC7700_CLK_CLK_RTC_CFG                                 625  // AON rtc
#define EIC7700_CLK_CLK_RTC                                     626  // AON rtc
#define EIC7700_CLK_CLK_U84_RTC_TOGGLE                          627
#define EIC7700_CLK_UNUSED_1                                    628
#define EIC7700_CLK_CLK_PKA_CFG                                 629
#define EIC7700_CLK_CLK_SPACC_CFG                               630
#define EIC7700_CLK_CLK_CRYPTO                                  631
#define EIC7700_CLK_CLK_TRNG_CFG                                632
#define EIC7700_CLK_CLK_OTP_CFG                                 633
#define EIC7700_CLK_CLMM_CFG_CLK                                634
#define EIC7700_CLK_CLMM_DEB_CLK                                635
#define EIC7700_CLK_DDR_PLL_BYP_CLK                             636
#define EIC7700_CLK_DDR_PLL_REF_AND_DFI_CLK                     637
#define EIC7700_CLK_DDR_RX_TEST_CLK                             638
#define EIC7700_CLK_MAILBOX_0                                   638
#define EIC7700_CLK_MAILBOX_1                                   639
#define EIC7700_CLK_MAILBOX_2                                   640
#define EIC7700_CLK_MAILBOX_3                                   641
#define EIC7700_CLK_MAILBOX_4                                   642
#define EIC7700_CLK_MAILBOX_5                                   643
#define EIC7700_CLK_MAILBOX_6                                   644
#define EIC7700_CLK_MAILBOX_7                                   645
#define EIC7700_CLK_MAILBOX_8                                   646
#define EIC7700_CLK_MAILBOX_9                                   647
#define EIC7700_CLK_MAILBOX_10                                  648
#define EIC7700_CLK_MAILBOX_11                                  649
#define EIC7700_CLK_MAILBOX_12                                  650
#define EIC7700_CLK_MAILBOX_13                                  651
#define EIC7700_CLK_MAILBOX_14                                  652
#define EIC7700_CLK_MAILBOX_15                                  653
#define EIC7700_CLK_LSP_I2C0_PCLK                               654
#define EIC7700_CLK_LSP_I2C1_PCLK                               655
#define EIC7700_CLK_LSP_I2C2_PCLK                               656
#define EIC7700_CLK_LSP_I2C3_PCLK                               657
#define EIC7700_CLK_LSP_I2C4_PCLK                               658
#define EIC7700_CLK_LSP_I2C5_PCLK                               659
#define EIC7700_CLK_LSP_I2C6_PCLK                               660
#define EIC7700_CLK_LSP_I2C7_PCLK                               661
#define EIC7700_CLK_LSP_I2C8_PCLK                               662
#define EIC7700_CLK_LSP_I2C9_PCLK                               663
#define EIC7700_CLK_LSP_WDT0_PCLK                               664
#define EIC7700_CLK_LSP_WDT1_PCLK                               665
#define EIC7700_CLK_LSP_WDT2_PCLK                               666
#define EIC7700_CLK_LSP_WDT3_PCLK                               667
#define EIC7700_CLK_LSP_SSI0_PCLK                               668
#define EIC7700_CLK_LSP_SSI1_PCLK                               669
#define EIC7700_CLK_LSP_PVT_PCLK                                670
#define EIC7700_CLK_AON_I2C0_PCLK                               671
#define EIC7700_CLK_AON_I2C1_PCLK                               672
#define EIC7700_CLK_LSP_UART0_PCLK                              673
#define EIC7700_CLK_LSP_UART1_PCLK                              674
#define EIC7700_CLK_LSP_UART2_PCLK                              675
#define EIC7700_CLK_LSP_UART3_PCLK                              676
#define EIC7700_CLK_LSP_UART4_PCLK                              677
#define EIC7700_CLK_LSP_TIMER_PCLK                              678         //LSP timer
#define EIC7700_CLK_LSP_FAN_PCLK                                679
#define EIC7700_CLK_DSP_ACLK_0                                  680
#define EIC7700_CLK_DSP_ACLK_1                                  681
#define EIC7700_CLK_DSP_ACLK_2                                  682
#define EIC7700_CLK_DSP_ACLK_3                                  683

#define EIC7700_CLK_VC_JE_PCLK                                  685
#define EIC7700_CLK_VC_JD_PCLK                                  686
#define EIC7700_CLK_VC_VE_PCLK                                  687
#define EIC7700_CLK_VC_VD_PCLK                                  688
#define EIC7700_CLK_VC_MON_PCLK                                 689

#define EIC7700_CLK_HSP_DMA0_CLK                                690

#define EIC7700_NR_CLKS                                         700

/* run frquency */
#define CLK_FREQ_1800M					1800000000
#define CLK_FREQ_1700M					1700000000
#define CLK_FREQ_1600M					1600000000
#define CLK_FREQ_1500M					1500000000
#define CLK_FREQ_1400M					1400000000
#define CLK_FREQ_1300M					1300000000
#define CLK_FREQ_1200M					1200000000
#define CLK_FREQ_1000M					1000000000
#define CLK_FREQ_900M					900000000
#define CLK_FREQ_800M					800000000
#define CLK_FREQ_700M					700000000
#define CLK_FREQ_600M					600000000
#define CLK_FREQ_500M					500000000
#define CLK_FREQ_400M					400000000
#define CLK_FREQ_200M					200000000
#define CLK_FREQ_100M					100000000
#define CLK_FREQ_24M					24000000

#define APLL_HIGH_FREQ					983040000
#define APLL_LOW_FREQ 					225792000

#endif /*endif __DTS_EIC7700_CLOCK_H*/
