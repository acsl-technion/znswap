// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Linaro Ltd
 * Author: Jun Nie <jun.nie@linaro.org>
 * With reference of msm8916 interconnect driver of Georgi Djakov.
 */

#include <linux/clk.h>
#include <linux/device.h>
#include <linux/interconnect-provider.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>

#include <dt-bindings/interconnect/qcom,msm8939.h>

#include "smd-rpm.h"
#include "icc-rpm.h"

enum {
	MSM8939_BIMC_SNOC_MAS = 1,
	MSM8939_BIMC_SNOC_SLV,
	MSM8939_MASTER_AMPSS_M0,
	MSM8939_MASTER_LPASS,
	MSM8939_MASTER_BLSP_1,
	MSM8939_MASTER_DEHR,
	MSM8939_MASTER_GRAPHICS_3D,
	MSM8939_MASTER_JPEG,
	MSM8939_MASTER_MDP_PORT0,
	MSM8939_MASTER_MDP_PORT1,
	MSM8939_MASTER_CPP,
	MSM8939_MASTER_CRYPTO_CORE0,
	MSM8939_MASTER_SDCC_1,
	MSM8939_MASTER_SDCC_2,
	MSM8939_MASTER_QDSS_BAM,
	MSM8939_MASTER_QDSS_ETR,
	MSM8939_MASTER_SNOC_CFG,
	MSM8939_MASTER_SPDM,
	MSM8939_MASTER_TCU0,
	MSM8939_MASTER_USB_HS1,
	MSM8939_MASTER_USB_HS2,
	MSM8939_MASTER_VFE,
	MSM8939_MASTER_VIDEO_P0,
	MSM8939_SNOC_MM_INT_0,
	MSM8939_SNOC_MM_INT_1,
	MSM8939_SNOC_MM_INT_2,
	MSM8939_PNOC_INT_0,
	MSM8939_PNOC_INT_1,
	MSM8939_PNOC_MAS_0,
	MSM8939_PNOC_MAS_1,
	MSM8939_PNOC_SLV_0,
	MSM8939_PNOC_SLV_1,
	MSM8939_PNOC_SLV_2,
	MSM8939_PNOC_SLV_3,
	MSM8939_PNOC_SLV_4,
	MSM8939_PNOC_SLV_8,
	MSM8939_PNOC_SLV_9,
	MSM8939_PNOC_SNOC_MAS,
	MSM8939_PNOC_SNOC_SLV,
	MSM8939_SNOC_QDSS_INT,
	MSM8939_SLAVE_AMPSS_L2,
	MSM8939_SLAVE_APSS,
	MSM8939_SLAVE_LPASS,
	MSM8939_SLAVE_BIMC_CFG,
	MSM8939_SLAVE_BLSP_1,
	MSM8939_SLAVE_BOOT_ROM,
	MSM8939_SLAVE_CAMERA_CFG,
	MSM8939_SLAVE_CATS_128,
	MSM8939_SLAVE_OCMEM_64,
	MSM8939_SLAVE_CLK_CTL,
	MSM8939_SLAVE_CRYPTO_0_CFG,
	MSM8939_SLAVE_DEHR_CFG,
	MSM8939_SLAVE_DISPLAY_CFG,
	MSM8939_SLAVE_EBI_CH0,
	MSM8939_SLAVE_GRAPHICS_3D_CFG,
	MSM8939_SLAVE_IMEM_CFG,
	MSM8939_SLAVE_IMEM,
	MSM8939_SLAVE_MPM,
	MSM8939_SLAVE_MSG_RAM,
	MSM8939_SLAVE_MSS,
	MSM8939_SLAVE_PDM,
	MSM8939_SLAVE_PMIC_ARB,
	MSM8939_SLAVE_PNOC_CFG,
	MSM8939_SLAVE_PRNG,
	MSM8939_SLAVE_QDSS_CFG,
	MSM8939_SLAVE_QDSS_STM,
	MSM8939_SLAVE_RBCPR_CFG,
	MSM8939_SLAVE_SDCC_1,
	MSM8939_SLAVE_SDCC_2,
	MSM8939_SLAVE_SECURITY,
	MSM8939_SLAVE_SNOC_CFG,
	MSM8939_SLAVE_SPDM,
	MSM8939_SLAVE_SRVC_SNOC,
	MSM8939_SLAVE_TCSR,
	MSM8939_SLAVE_TLMM,
	MSM8939_SLAVE_USB_HS1,
	MSM8939_SLAVE_USB_HS2,
	MSM8939_SLAVE_VENUS_CFG,
	MSM8939_SNOC_BIMC_0_MAS,
	MSM8939_SNOC_BIMC_0_SLV,
	MSM8939_SNOC_BIMC_1_MAS,
	MSM8939_SNOC_BIMC_1_SLV,
	MSM8939_SNOC_BIMC_2_MAS,
	MSM8939_SNOC_BIMC_2_SLV,
	MSM8939_SNOC_INT_0,
	MSM8939_SNOC_INT_1,
	MSM8939_SNOC_INT_BIMC,
	MSM8939_SNOC_PNOC_MAS,
	MSM8939_SNOC_PNOC_SLV,
};

static const struct clk_bulk_data msm8939_bus_clocks[] = {
	{ .id = "bus" },
	{ .id = "bus_a" },
};

DEFINE_QNODE(bimc_snoc_mas, MSM8939_BIMC_SNOC_MAS, 8, -1, -1, MSM8939_BIMC_SNOC_SLV);
DEFINE_QNODE(bimc_snoc_slv, MSM8939_BIMC_SNOC_SLV, 16, -1, 2, MSM8939_SNOC_INT_0, MSM8939_SNOC_INT_1);
DEFINE_QNODE(mas_apss, MSM8939_MASTER_AMPSS_M0, 16, -1, -1, MSM8939_SLAVE_EBI_CH0, MSM8939_BIMC_SNOC_MAS, MSM8939_SLAVE_AMPSS_L2);
DEFINE_QNODE(mas_audio, MSM8939_MASTER_LPASS, 4, -1, -1, MSM8939_PNOC_MAS_0);
DEFINE_QNODE(mas_blsp_1, MSM8939_MASTER_BLSP_1, 4, -1, -1, MSM8939_PNOC_MAS_1);
DEFINE_QNODE(mas_dehr, MSM8939_MASTER_DEHR, 4, -1, -1, MSM8939_PNOC_MAS_0);
DEFINE_QNODE(mas_gfx, MSM8939_MASTER_GRAPHICS_3D, 16, -1, -1, MSM8939_SLAVE_EBI_CH0, MSM8939_BIMC_SNOC_MAS, MSM8939_SLAVE_AMPSS_L2);
DEFINE_QNODE(mas_jpeg, MSM8939_MASTER_JPEG, 16, -1, -1, MSM8939_SNOC_MM_INT_0, MSM8939_SNOC_MM_INT_2);
DEFINE_QNODE(mas_mdp0, MSM8939_MASTER_MDP_PORT0, 16, -1, -1, MSM8939_SNOC_MM_INT_1, MSM8939_SNOC_MM_INT_2);
DEFINE_QNODE(mas_mdp1, MSM8939_MASTER_MDP_PORT1, 16, -1, -1, MSM8939_SNOC_MM_INT_0, MSM8939_SNOC_MM_INT_2);
DEFINE_QNODE(mas_cpp, MSM8939_MASTER_CPP, 16, -1, -1, MSM8939_SNOC_MM_INT_0, MSM8939_SNOC_MM_INT_2);
DEFINE_QNODE(mas_pcnoc_crypto_0, MSM8939_MASTER_CRYPTO_CORE0, 8, -1, -1, MSM8939_PNOC_INT_1);
DEFINE_QNODE(mas_pcnoc_sdcc_1, MSM8939_MASTER_SDCC_1, 8, -1, -1, MSM8939_PNOC_INT_1);
DEFINE_QNODE(mas_pcnoc_sdcc_2, MSM8939_MASTER_SDCC_2, 8, -1, -1, MSM8939_PNOC_INT_1);
DEFINE_QNODE(mas_qdss_bam, MSM8939_MASTER_QDSS_BAM, 8, -1, -1, MSM8939_SNOC_QDSS_INT);
DEFINE_QNODE(mas_qdss_etr, MSM8939_MASTER_QDSS_ETR, 8, -1, -1, MSM8939_SNOC_QDSS_INT);
DEFINE_QNODE(mas_snoc_cfg, MSM8939_MASTER_SNOC_CFG, 4, 20, -1, MSM8939_SLAVE_SRVC_SNOC);
DEFINE_QNODE(mas_spdm, MSM8939_MASTER_SPDM, 4, -1, -1, MSM8939_PNOC_MAS_0);
DEFINE_QNODE(mas_tcu0, MSM8939_MASTER_TCU0, 16, -1, -1, MSM8939_SLAVE_EBI_CH0, MSM8939_BIMC_SNOC_MAS, MSM8939_SLAVE_AMPSS_L2);
DEFINE_QNODE(mas_usb_hs1, MSM8939_MASTER_USB_HS1, 4, -1, -1, MSM8939_PNOC_MAS_1);
DEFINE_QNODE(mas_usb_hs2, MSM8939_MASTER_USB_HS2, 4, -1, -1, MSM8939_PNOC_MAS_1);
DEFINE_QNODE(mas_vfe, MSM8939_MASTER_VFE, 16, -1, -1, MSM8939_SNOC_MM_INT_1, MSM8939_SNOC_MM_INT_2);
DEFINE_QNODE(mas_video, MSM8939_MASTER_VIDEO_P0, 16, -1, -1, MSM8939_SNOC_MM_INT_0, MSM8939_SNOC_MM_INT_2);
DEFINE_QNODE(mm_int_0, MSM8939_SNOC_MM_INT_0, 16, -1, -1, MSM8939_SNOC_BIMC_2_MAS);
DEFINE_QNODE(mm_int_1, MSM8939_SNOC_MM_INT_1, 16, -1, -1, MSM8939_SNOC_BIMC_1_MAS);
DEFINE_QNODE(mm_int_2, MSM8939_SNOC_MM_INT_2, 16, -1, -1, MSM8939_SNOC_INT_0);
DEFINE_QNODE(pcnoc_int_0, MSM8939_PNOC_INT_0, 8, -1, -1, MSM8939_PNOC_SNOC_MAS, MSM8939_PNOC_SLV_0, MSM8939_PNOC_SLV_1, MSM8939_PNOC_SLV_2, MSM8939_PNOC_SLV_3, MSM8939_PNOC_SLV_4, MSM8939_PNOC_SLV_8, MSM8939_PNOC_SLV_9);
DEFINE_QNODE(pcnoc_int_1, MSM8939_PNOC_INT_1, 8, -1, -1, MSM8939_PNOC_SNOC_MAS);
DEFINE_QNODE(pcnoc_m_0, MSM8939_PNOC_MAS_0, 8, -1, -1, MSM8939_PNOC_INT_0);
DEFINE_QNODE(pcnoc_m_1, MSM8939_PNOC_MAS_1, 8, -1, -1, MSM8939_PNOC_SNOC_MAS);
DEFINE_QNODE(pcnoc_s_0, MSM8939_PNOC_SLV_0, 4, -1, -1, MSM8939_SLAVE_CLK_CTL, MSM8939_SLAVE_TLMM, MSM8939_SLAVE_TCSR, MSM8939_SLAVE_SECURITY, MSM8939_SLAVE_MSS);
DEFINE_QNODE(pcnoc_s_1, MSM8939_PNOC_SLV_1, 4, -1, -1, MSM8939_SLAVE_IMEM_CFG, MSM8939_SLAVE_CRYPTO_0_CFG, MSM8939_SLAVE_MSG_RAM, MSM8939_SLAVE_PDM, MSM8939_SLAVE_PRNG);
DEFINE_QNODE(pcnoc_s_2, MSM8939_PNOC_SLV_2, 4, -1, -1, MSM8939_SLAVE_SPDM, MSM8939_SLAVE_BOOT_ROM, MSM8939_SLAVE_BIMC_CFG, MSM8939_SLAVE_PNOC_CFG, MSM8939_SLAVE_PMIC_ARB);
DEFINE_QNODE(pcnoc_s_3, MSM8939_PNOC_SLV_3, 4, -1, -1, MSM8939_SLAVE_MPM, MSM8939_SLAVE_SNOC_CFG, MSM8939_SLAVE_RBCPR_CFG, MSM8939_SLAVE_QDSS_CFG, MSM8939_SLAVE_DEHR_CFG);
DEFINE_QNODE(pcnoc_s_4, MSM8939_PNOC_SLV_4, 4, -1, -1, MSM8939_SLAVE_VENUS_CFG, MSM8939_SLAVE_CAMERA_CFG, MSM8939_SLAVE_DISPLAY_CFG);
DEFINE_QNODE(pcnoc_s_8, MSM8939_PNOC_SLV_8, 4, -1, -1, MSM8939_SLAVE_USB_HS1, MSM8939_SLAVE_SDCC_1, MSM8939_SLAVE_BLSP_1);
DEFINE_QNODE(pcnoc_s_9, MSM8939_PNOC_SLV_9, 4, -1, -1, MSM8939_SLAVE_SDCC_2, MSM8939_SLAVE_LPASS, MSM8939_SLAVE_USB_HS2);
DEFINE_QNODE(pcnoc_snoc_mas, MSM8939_PNOC_SNOC_MAS, 8, 29, -1, MSM8939_PNOC_SNOC_SLV);
DEFINE_QNODE(pcnoc_snoc_slv, MSM8939_PNOC_SNOC_SLV, 8, -1, 45, MSM8939_SNOC_INT_0, MSM8939_SNOC_INT_BIMC, MSM8939_SNOC_INT_1);
DEFINE_QNODE(qdss_int, MSM8939_SNOC_QDSS_INT, 8, -1, -1, MSM8939_SNOC_INT_0, MSM8939_SNOC_INT_BIMC);
DEFINE_QNODE(slv_apps_l2, MSM8939_SLAVE_AMPSS_L2, 16, -1, -1, 0);
DEFINE_QNODE(slv_apss, MSM8939_SLAVE_APSS, 4, -1, 20, 0);
DEFINE_QNODE(slv_audio, MSM8939_SLAVE_LPASS, 4, -1, -1, 0);
DEFINE_QNODE(slv_bimc_cfg, MSM8939_SLAVE_BIMC_CFG, 4, -1, -1, 0);
DEFINE_QNODE(slv_blsp_1, MSM8939_SLAVE_BLSP_1, 4, -1, -1, 0);
DEFINE_QNODE(slv_boot_rom, MSM8939_SLAVE_BOOT_ROM, 4, -1, -1, 0);
DEFINE_QNODE(slv_camera_cfg, MSM8939_SLAVE_CAMERA_CFG, 4, -1, -1, 0);
DEFINE_QNODE(slv_cats_0, MSM8939_SLAVE_CATS_128, 16, -1, 106, 0);
DEFINE_QNODE(slv_cats_1, MSM8939_SLAVE_OCMEM_64, 8, -1, 107, 0);
DEFINE_QNODE(slv_clk_ctl, MSM8939_SLAVE_CLK_CTL, 4, -1, -1, 0);
DEFINE_QNODE(slv_crypto_0_cfg, MSM8939_SLAVE_CRYPTO_0_CFG, 4, -1, -1, 0);
DEFINE_QNODE(slv_dehr_cfg, MSM8939_SLAVE_DEHR_CFG, 4, -1, -1, 0);
DEFINE_QNODE(slv_display_cfg, MSM8939_SLAVE_DISPLAY_CFG, 4, -1, -1, 0);
DEFINE_QNODE(slv_ebi_ch0, MSM8939_SLAVE_EBI_CH0, 16, -1, 0, 0);
DEFINE_QNODE(slv_gfx_cfg, MSM8939_SLAVE_GRAPHICS_3D_CFG, 4, -1, -1, 0);
DEFINE_QNODE(slv_imem_cfg, MSM8939_SLAVE_IMEM_CFG, 4, -1, -1, 0);
DEFINE_QNODE(slv_imem, MSM8939_SLAVE_IMEM, 8, -1, 26, 0);
DEFINE_QNODE(slv_mpm, MSM8939_SLAVE_MPM, 4, -1, -1, 0);
DEFINE_QNODE(slv_msg_ram, MSM8939_SLAVE_MSG_RAM, 4, -1, -1, 0);
DEFINE_QNODE(slv_mss, MSM8939_SLAVE_MSS, 4, -1, -1, 0);
DEFINE_QNODE(slv_pdm, MSM8939_SLAVE_PDM, 4, -1, -1, 0);
DEFINE_QNODE(slv_pmic_arb, MSM8939_SLAVE_PMIC_ARB, 4, -1, -1, 0);
DEFINE_QNODE(slv_pcnoc_cfg, MSM8939_SLAVE_PNOC_CFG, 4, -1, -1, 0);
DEFINE_QNODE(slv_prng, MSM8939_SLAVE_PRNG, 4, -1, -1, 0);
DEFINE_QNODE(slv_qdss_cfg, MSM8939_SLAVE_QDSS_CFG, 4, -1, -1, 0);
DEFINE_QNODE(slv_qdss_stm, MSM8939_SLAVE_QDSS_STM, 4, -1, 30, 0);
DEFINE_QNODE(slv_rbcpr_cfg, MSM8939_SLAVE_RBCPR_CFG, 4, -1, -1, 0);
DEFINE_QNODE(slv_sdcc_1, MSM8939_SLAVE_SDCC_1, 4, -1, -1, 0);
DEFINE_QNODE(slv_sdcc_2, MSM8939_SLAVE_SDCC_2, 4, -1, -1, 0);
DEFINE_QNODE(slv_security, MSM8939_SLAVE_SECURITY, 4, -1, -1, 0);
DEFINE_QNODE(slv_snoc_cfg, MSM8939_SLAVE_SNOC_CFG, 4, -1, -1, 0);
DEFINE_QNODE(slv_spdm, MSM8939_SLAVE_SPDM, 4, -1, -1, 0);
DEFINE_QNODE(slv_srvc_snoc, MSM8939_SLAVE_SRVC_SNOC, 8, -1, 29, 0);
DEFINE_QNODE(slv_tcsr, MSM8939_SLAVE_TCSR, 4, -1, -1, 0);
DEFINE_QNODE(slv_tlmm, MSM8939_SLAVE_TLMM, 4, -1, -1, 0);
DEFINE_QNODE(slv_usb_hs1, MSM8939_SLAVE_USB_HS1, 4, -1, -1, 0);
DEFINE_QNODE(slv_usb_hs2, MSM8939_SLAVE_USB_HS2, 4, -1, -1, 0);
DEFINE_QNODE(slv_venus_cfg, MSM8939_SLAVE_VENUS_CFG, 4, -1, -1, 0);
DEFINE_QNODE(snoc_bimc_0_mas, MSM8939_SNOC_BIMC_0_MAS, 16, 3, -1, MSM8939_SNOC_BIMC_0_SLV);
DEFINE_QNODE(snoc_bimc_0_slv, MSM8939_SNOC_BIMC_0_SLV, 16, -1, 24, MSM8939_SLAVE_EBI_CH0);
DEFINE_QNODE(snoc_bimc_1_mas, MSM8939_SNOC_BIMC_1_MAS, 16, 76, -1, MSM8939_SNOC_BIMC_1_SLV);
DEFINE_QNODE(snoc_bimc_1_slv, MSM8939_SNOC_BIMC_1_SLV, 16, -1, 104, MSM8939_SLAVE_EBI_CH0);
DEFINE_QNODE(snoc_bimc_2_mas, MSM8939_SNOC_BIMC_2_MAS, 16, -1, -1, MSM8939_SNOC_BIMC_2_SLV);
DEFINE_QNODE(snoc_bimc_2_slv, MSM8939_SNOC_BIMC_2_SLV, 16, -1, -1, MSM8939_SLAVE_EBI_CH0);
DEFINE_QNODE(snoc_int_0, MSM8939_SNOC_INT_0, 8, 99, 130, MSM8939_SLAVE_QDSS_STM, MSM8939_SLAVE_IMEM, MSM8939_SNOC_PNOC_MAS);
DEFINE_QNODE(snoc_int_1, MSM8939_SNOC_INT_1, 8, 100, 131, MSM8939_SLAVE_APSS, MSM8939_SLAVE_CATS_128, MSM8939_SLAVE_OCMEM_64);
DEFINE_QNODE(snoc_int_bimc, MSM8939_SNOC_INT_BIMC, 8, 101, 132, MSM8939_SNOC_BIMC_1_MAS);
DEFINE_QNODE(snoc_pcnoc_mas, MSM8939_SNOC_PNOC_MAS, 8, -1, -1, MSM8939_SNOC_PNOC_SLV);
DEFINE_QNODE(snoc_pcnoc_slv, MSM8939_SNOC_PNOC_SLV, 8, -1, -1, MSM8939_PNOC_INT_0);

static struct qcom_icc_node *msm8939_snoc_nodes[] = {
	[BIMC_SNOC_SLV] = &bimc_snoc_slv,
	[MASTER_QDSS_BAM] = &mas_qdss_bam,
	[MASTER_QDSS_ETR] = &mas_qdss_etr,
	[MASTER_SNOC_CFG] = &mas_snoc_cfg,
	[PCNOC_SNOC_SLV] = &pcnoc_snoc_slv,
	[SLAVE_APSS] = &slv_apss,
	[SLAVE_CATS_128] = &slv_cats_0,
	[SLAVE_OCMEM_64] = &slv_cats_1,
	[SLAVE_IMEM] = &slv_imem,
	[SLAVE_QDSS_STM] = &slv_qdss_stm,
	[SLAVE_SRVC_SNOC] = &slv_srvc_snoc,
	[SNOC_BIMC_0_MAS] = &snoc_bimc_0_mas,
	[SNOC_BIMC_1_MAS] = &snoc_bimc_1_mas,
	[SNOC_BIMC_2_MAS] = &snoc_bimc_2_mas,
	[SNOC_INT_0] = &snoc_int_0,
	[SNOC_INT_1] = &snoc_int_1,
	[SNOC_INT_BIMC] = &snoc_int_bimc,
	[SNOC_PCNOC_MAS] = &snoc_pcnoc_mas,
	[SNOC_QDSS_INT] = &qdss_int,
};

static struct qcom_icc_desc msm8939_snoc = {
	.nodes = msm8939_snoc_nodes,
	.num_nodes = ARRAY_SIZE(msm8939_snoc_nodes),
};

static struct qcom_icc_node *msm8939_snoc_mm_nodes[] = {
	[MASTER_VIDEO_P0] = &mas_video,
	[MASTER_JPEG] = &mas_jpeg,
	[MASTER_VFE] = &mas_vfe,
	[MASTER_MDP_PORT0] = &mas_mdp0,
	[MASTER_MDP_PORT1] = &mas_mdp1,
	[MASTER_CPP] = &mas_cpp,
	[SNOC_MM_INT_0] = &mm_int_0,
	[SNOC_MM_INT_1] = &mm_int_1,
	[SNOC_MM_INT_2] = &mm_int_2,
};

static struct qcom_icc_desc msm8939_snoc_mm = {
	.nodes = msm8939_snoc_mm_nodes,
	.num_nodes = ARRAY_SIZE(msm8939_snoc_mm_nodes),
};

static struct qcom_icc_node *msm8939_bimc_nodes[] = {
	[BIMC_SNOC_MAS] = &bimc_snoc_mas,
	[MASTER_AMPSS_M0] = &mas_apss,
	[MASTER_GRAPHICS_3D] = &mas_gfx,
	[MASTER_TCU0] = &mas_tcu0,
	[SLAVE_AMPSS_L2] = &slv_apps_l2,
	[SLAVE_EBI_CH0] = &slv_ebi_ch0,
	[SNOC_BIMC_0_SLV] = &snoc_bimc_0_slv,
	[SNOC_BIMC_1_SLV] = &snoc_bimc_1_slv,
	[SNOC_BIMC_2_SLV] = &snoc_bimc_2_slv,
};

static struct qcom_icc_desc msm8939_bimc = {
	.nodes = msm8939_bimc_nodes,
	.num_nodes = ARRAY_SIZE(msm8939_bimc_nodes),
};

static struct qcom_icc_node *msm8939_pcnoc_nodes[] = {
	[MASTER_BLSP_1] = &mas_blsp_1,
	[MASTER_DEHR] = &mas_dehr,
	[MASTER_LPASS] = &mas_audio,
	[MASTER_CRYPTO_CORE0] = &mas_pcnoc_crypto_0,
	[MASTER_SDCC_1] = &mas_pcnoc_sdcc_1,
	[MASTER_SDCC_2] = &mas_pcnoc_sdcc_2,
	[MASTER_SPDM] = &mas_spdm,
	[MASTER_USB_HS1] = &mas_usb_hs1,
	[MASTER_USB_HS2] = &mas_usb_hs2,
	[PCNOC_INT_0] = &pcnoc_int_0,
	[PCNOC_INT_1] = &pcnoc_int_1,
	[PCNOC_MAS_0] = &pcnoc_m_0,
	[PCNOC_MAS_1] = &pcnoc_m_1,
	[PCNOC_SLV_0] = &pcnoc_s_0,
	[PCNOC_SLV_1] = &pcnoc_s_1,
	[PCNOC_SLV_2] = &pcnoc_s_2,
	[PCNOC_SLV_3] = &pcnoc_s_3,
	[PCNOC_SLV_4] = &pcnoc_s_4,
	[PCNOC_SLV_8] = &pcnoc_s_8,
	[PCNOC_SLV_9] = &pcnoc_s_9,
	[PCNOC_SNOC_MAS] = &pcnoc_snoc_mas,
	[SLAVE_BIMC_CFG] = &slv_bimc_cfg,
	[SLAVE_BLSP_1] = &slv_blsp_1,
	[SLAVE_BOOT_ROM] = &slv_boot_rom,
	[SLAVE_CAMERA_CFG] = &slv_camera_cfg,
	[SLAVE_CLK_CTL] = &slv_clk_ctl,
	[SLAVE_CRYPTO_0_CFG] = &slv_crypto_0_cfg,
	[SLAVE_DEHR_CFG] = &slv_dehr_cfg,
	[SLAVE_DISPLAY_CFG] = &slv_display_cfg,
	[SLAVE_GRAPHICS_3D_CFG] = &slv_gfx_cfg,
	[SLAVE_IMEM_CFG] = &slv_imem_cfg,
	[SLAVE_LPASS] = &slv_audio,
	[SLAVE_MPM] = &slv_mpm,
	[SLAVE_MSG_RAM] = &slv_msg_ram,
	[SLAVE_MSS] = &slv_mss,
	[SLAVE_PDM] = &slv_pdm,
	[SLAVE_PMIC_ARB] = &slv_pmic_arb,
	[SLAVE_PCNOC_CFG] = &slv_pcnoc_cfg,
	[SLAVE_PRNG] = &slv_prng,
	[SLAVE_QDSS_CFG] = &slv_qdss_cfg,
	[SLAVE_RBCPR_CFG] = &slv_rbcpr_cfg,
	[SLAVE_SDCC_1] = &slv_sdcc_1,
	[SLAVE_SDCC_2] = &slv_sdcc_2,
	[SLAVE_SECURITY] = &slv_security,
	[SLAVE_SNOC_CFG] = &slv_snoc_cfg,
	[SLAVE_SPDM] = &slv_spdm,
	[SLAVE_TCSR] = &slv_tcsr,
	[SLAVE_TLMM] = &slv_tlmm,
	[SLAVE_USB_HS1] = &slv_usb_hs1,
	[SLAVE_USB_HS2] = &slv_usb_hs2,
	[SLAVE_VENUS_CFG] = &slv_venus_cfg,
	[SNOC_PCNOC_SLV] = &snoc_pcnoc_slv,
};

static struct qcom_icc_desc msm8939_pcnoc = {
	.nodes = msm8939_pcnoc_nodes,
	.num_nodes = ARRAY_SIZE(msm8939_pcnoc_nodes),
};

static int msm8939_qnoc_probe(struct platform_device *pdev)
{
	return qnoc_probe(pdev, sizeof(msm8939_bus_clocks),
			  ARRAY_SIZE(msm8939_bus_clocks), msm8939_bus_clocks);
}

static const struct of_device_id msm8939_noc_of_match[] = {
	{ .compatible = "qcom,msm8939-bimc", .data = &msm8939_bimc },
	{ .compatible = "qcom,msm8939-pcnoc", .data = &msm8939_pcnoc },
	{ .compatible = "qcom,msm8939-snoc", .data = &msm8939_snoc },
	{ .compatible = "qcom,msm8939-snoc-mm", .data = &msm8939_snoc_mm },
	{ }
};
MODULE_DEVICE_TABLE(of, msm8939_noc_of_match);

static struct platform_driver msm8939_noc_driver = {
	.probe = msm8939_qnoc_probe,
	.remove = qnoc_remove,
	.driver = {
		.name = "qnoc-msm8939",
		.of_match_table = msm8939_noc_of_match,
	},
};
module_platform_driver(msm8939_noc_driver);
MODULE_AUTHOR("Jun Nie <jun.nie@linaro.org>");
MODULE_DESCRIPTION("Qualcomm MSM8939 NoC driver");
MODULE_LICENSE("GPL v2");
