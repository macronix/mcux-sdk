/*
 * Copyright (c) 2022-2023 Macronix International Co. LTD. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "flexspi_nand_flash_ops.h"

#if (defined CACHE_MAINTAIN) && (CACHE_MAINTAIN == 1)
#include "fsl_cache.h"
#endif

#if (defined CACHE_MAINTAIN) && (CACHE_MAINTAIN == 1)
typedef struct _flexspi_cache_status
{
    volatile bool DCacheEnableFlag;
    volatile bool ICacheEnableFlag;
} flexspi_cache_status_t;
#endif

/*${prototype:start}*/
void BOARD_InitHardware(void);
static inline void flexspi_clock_init(void)
{
    /*Clock setting for flexspi1*/
    CLOCK_SetRootClockDiv(kCLOCK_Root_Flexspi2, 2);
    CLOCK_SetRootClockMux(kCLOCK_Root_Flexspi2, 0);
}

#if (defined CACHE_MAINTAIN) && (CACHE_MAINTAIN == 1)
void flexspi_nor_disable_cache(flexspi_cache_status_t *cacheStatus);
#endif

void flexspi_nor_enable_cache(flexspi_cache_status_t cacheStatus);
/*******************************************************************************
 * Variables
 *****************************************************************************/
flexspi_device_config_t deviceConfig = {
    .flexspiRootClk       = 12000000,
    .flashSize            = NAND_FLASH_SIZE,
    .CSIntervalUnit       = kFLEXSPI_CsIntervalUnit1SckCycle,
    .CSInterval           = 2,
    .CSHoldTime           = 3,
    .CSSetupTime          = 3,
    .dataValidTime        = 0,
    .columnspace          = 12,
    .enableWordAddress    = 0,
    .AWRSeqIndex          = 0,
    .AWRSeqNumber         = 0,
    .ARDSeqIndex          = NAND_CMD_LUT_SEQ_IDX_PAGE_READ,
    .ARDSeqNumber         = 1,
    .AHBWriteWaitUnit     = kFLEXSPI_AhbWriteWaitUnit2AhbCycle,
    .AHBWriteWaitInterval = 0,
};

void delayUs(uint32_t delay_us)
{
    uint32_t s_tickPerMicrosecond = CLOCK_GetFreq(kCLOCK_CpuClk) / 1000000U;

    // Make sure this value is greater than 0
    if (!s_tickPerMicrosecond)
    {
        s_tickPerMicrosecond = 1;
    }
    delay_us = delay_us * s_tickPerMicrosecond;
    while (delay_us)
    {
        __NOP();
        delay_us--;
    }
}

const uint32_t seqLUT[SEQ_LUT_LENGTH] = {
    /* read cache random */
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_CACHE_RANDOM] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_STATUS_BIT_ECC_STATUS_MASK, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, SPINAND_CFG_ADDR_24),
    /* write BBM */
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_WRITE_BBM] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_WRITE_BBM, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, SPINAND_CFG_ADDR_32),
    /* enter deep power down */
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_DP] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_DP, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
    /* read BBM */
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_BBM] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_READ_BBM, kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_1PAD, SPINAND_CFG_DUMMY_8),
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_BBM + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, SPINAND_CFG_ADDR_24, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
    /* read status*/
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_STATUS] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_RSR1, kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0),
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_CACHE_SEQUENTIAL] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_READ_CACHE_SEQ, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
    /* read cache sequential end */
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_SEQUENTIAL_END] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_READ_CACHE_END, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
    /* read ECCSR */
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_ECCSR] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_ECC_STAT_READ, kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_1PAD, SPINAND_CFG_DUMMY_8),
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_ECCSR + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
    /* reset */
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_RESET] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_RESET, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
    /* write disable*/
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_WRITE_DISABLE] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_WRDI, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
    /* read ECC warning page address */
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_ECC_WARNING] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_ECC_WARNING, kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_1PAD, SPINAND_CFG_DUMMY_8),
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_ECC_WARNING + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, SPINAND_CFG_ADDR_24, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
    /* read ID */
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READID] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_RDID,kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_1PAD, SPINAND_CFG_DUMMY_8),
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READID + 1] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
};

const uint32_t customLUT[CUSTOM_LUT_LENGTH] = {
    /* page load random data*/
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_PAGE_LOAD_RANDOM] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_PP_RAND_LOAD, kFLEXSPI_Command_CADDR_SDR, kFLEXSPI_1PAD, SPINAND_CFG_ADDR_16),
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_PAGE_LOAD_RANDOM + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_1PAD, 0, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_WRITEENABLE] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_WREN, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
    /* page load random data 114*/
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_PP_LOAD_RANDOM_114] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_4PP_RAND_LOAD, kFLEXSPI_Command_CADDR_SDR, kFLEXSPI_1PAD, SPINAND_CFG_ADDR_16),
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_PP_LOAD_RANDOM_114 + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_4PAD, 0, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
    /* get feature */
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_GETFEATURE] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_GET_FEATURE, kFLEXSPI_Command_CADDR_SDR, kFLEXSPI_1PAD, SPINAND_CFG_ADDR_8),
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_GETFEATURE + 1] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
    /* set feature */
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_SETFEATURE] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_SET_FEATURE, kFLEXSPI_Command_CADDR_SDR, kFLEXSPI_1PAD, SPINAND_CFG_ADDR_8),
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_SETFEATURE + 1] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_1PAD, 0x01, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    /* page read */
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_PAGE_READ] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_PAGE_READ, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, SPINAND_CFG_ADDR_24),
    /*read cache 122*/
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_CACHE_122] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0xBB, kFLEXSPI_Command_CADDR_SDR, kFLEXSPI_2PAD, SPINAND_CFG_ADDR_16),
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_CACHE_122 + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_2PAD, SPINAND_CFG_DUMMY_4, kFLEXSPI_Command_READ_SDR, kFLEXSPI_2PAD, 0),
    /* page load 11LUT_INDEX_LENGTH */
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_PAGE_LOAD_114] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_4PP_LOAD, kFLEXSPI_Command_CADDR_SDR, kFLEXSPI_1PAD, SPINAND_CFG_ADDR_16),
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_PAGE_LOAD_114 + 1] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_4PAD, 0, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
    /* read cache 112*/
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_CACHE_112] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_READ_CACHE2, kFLEXSPI_Command_CADDR_SDR, kFLEXSPI_1PAD, SPINAND_CFG_ADDR_16),
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_CACHE_112 + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_1PAD, SPINAND_CFG_DUMMY_8,  kFLEXSPI_Command_READ_SDR, kFLEXSPI_2PAD, 0),
    /* read cache 114  */
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_CACHE_114] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_READ_CACHE4, kFLEXSPI_Command_CADDR_SDR, kFLEXSPI_1PAD, SPINAND_CFG_ADDR_16),
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_CACHE_114 + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_1PAD, SPINAND_CFG_DUMMY_8, kFLEXSPI_Command_READ_SDR, kFLEXSPI_4PAD, 0),
    /* read cache 144  */
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_CACHE_144] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_READ_CACHE144, kFLEXSPI_Command_CADDR_SDR, kFLEXSPI_4PAD, SPINAND_CFG_ADDR_16),
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_CACHE_144 + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_4PAD, SPINAND_CFG_DUMMY_4, kFLEXSPI_Command_READ_SDR, kFLEXSPI_4PAD, 0),
#if SPI_NOR_EN
    /* read cache */
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_CACHE] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_READ_CACHE, kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_1PAD, SPINAND_CFG_DUMMY_8),
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_CACHE + 1] = FLEXSPI_LUT_SEQ(
    	kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, SPINAND_CFG_ADDR_16, kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0),

    /* read cache alternative */
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_CACHE_ALTERNATIVE] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_READ_CACHE_ALT, kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_1PAD, SPINAND_CFG_DUMMY_8),
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_CACHE_ALTERNATIVE + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, SPINAND_CFG_ADDR_16, kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_1PAD, SPINAND_CFG_DUMMY_8),
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_CACHE_ALTERNATIVE + 2] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
#else
    /* read cache */
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_CACHE] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_READ_CACHE, kFLEXSPI_Command_CADDR_SDR, kFLEXSPI_1PAD, SPINAND_CFG_ADDR_16),
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_CACHE + 1] = FLEXSPI_LUT_SEQ(
    	kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_1PAD, SPINAND_CFG_DUMMY_8, kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0),

	/* read cache alternative */
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_CACHE_ALTERNATIVE] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_READ_CACHE_ALT, kFLEXSPI_Command_CADDR_SDR, kFLEXSPI_1PAD, SPINAND_CFG_ADDR_16),
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_CACHE_ALTERNATIVE + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_1PAD, SPINAND_CFG_DUMMY_8, kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0),
#endif
    /* page load */
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_PAGE_LOAD] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_PP_LOAD, kFLEXSPI_Command_CADDR_SDR, kFLEXSPI_1PAD, SPINAND_CFG_ADDR_16),
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_PAGE_LOAD + 1] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_1PAD, 0, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    /* program execute */
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_PROGRAM_EXECUTE] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_PROGRAM_EXEC, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, SPINAND_CFG_ADDR_24),

    /* block erase */
    [LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_ERASE] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINAND_INST_BE, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, SPINAND_CFG_ADDR_24),
};

/**
 * @brief erase the device by block
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] addr       the block address of erase operation
 */
status_t flexspi_nand_erase_block(FLEXSPI_Type *base, uint32_t addr)
{
    status_t status = kStatus_Success;

    /* write enable*/
    status = flexspi_nand_write_enable(base);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* send block erase command */
    status = flexspi_nand_send_general_command(base, NAND_CMD_LUT_SEQ_IDX_ERASE, addr, NULL, 0, NULL, 0);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* check the status register to wait bus busy */
    status = flexspi_nand_wait_bus_busy(base);

    return status;
}

/**
 * @brief read at most a single page and check the internal ECC status
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] inst       read instruction send to controller
 * @param[in] addr       the address of read operation
 * @param[in] buffer     buffer to collect the output read data
 * @param[in] size       read data size
 */
status_t flexspi_nand_send_read_command(FLEXSPI_Type *base, int inst, const void *buffer, uint32_t addr, uint32_t size)
{
    status_t status = kStatus_Success;

    /* send page read command */
    status = flexspi_nand_send_general_command(base, NAND_CMD_LUT_SEQ_IDX_PAGE_READ, addr, NULL, 0, NULL, 0);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* check the status register to wait bus busy */
    status = flexspi_nand_wait_bus_busy(base);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* send read from cache command*/
    status = flexspi_nand_send_general_command(base, inst, addr, NULL, 0, buffer, size);

    return status;
}
/**
 * @brief sequential read function can reduce the latency of moving data between pages
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] inst       read instruction send to controller
 * @param[in] addr       the address of read operation
 * @param[in] buffer     buffer to collect the output read data
 * @param[in] size       read data size
 */
status_t flexspi_nand_sequential_read(FLEXSPI_Type *base, int inst, uint32_t addr, const void *buffer, uint32_t size)
{
    status_t status = kStatus_Success;

    uint32_t offset = 0;
    uint32_t chunk = 0;
    uint32_t readBytes = 0;

    /* send page read command */
    status = flexspi_nand_send_general_command(base, NAND_CMD_LUT_SEQ_IDX_PAGE_READ, addr, NULL, 0, NULL, 0);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* check the status register to wait bus busy */
    status = flexspi_nand_wait_bus_busy(base);

    if (status != kStatus_Success)
    {
        return status;
    }

    while (size > 0) {

        /* Update LUT table. */
        FLEXSPI_UpdateLUT(EXAMPLE_FLEXSPI, NAND_CMD_LUT_FOR_IP_CMD, &seqLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_CACHE_SEQUENTIAL],
                          LUT_LENGTH);

        /* Send read cache sequential command */
        status = flexspi_nand_send_general_command(base, NAND_CMD_LUT_FOR_IP_CMD, 0, NULL, 0, NULL, 0);

        if (status != kStatus_Success)
        {
            return status;
        }

        /* Update LUT table. */
        FLEXSPI_UpdateLUT(EXAMPLE_FLEXSPI, NAND_CMD_LUT_FOR_IP_CMD, &customLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_FOR_IP_CMD], LUT_LENGTH);

        /* send read from cache command*/
        status = flexspi_nand_send_general_command(base, inst, addr, NULL, 0, buffer, readBytes);

        if (status != kStatus_Success)
        {
            return status;
        }

        /* check the status register to wait bus busy */
        status = flexspi_nand_wait_bus_busy(base);

        if (status != kStatus_Success)
        {
            return status;
        }

        buffer = (uint8_t *)buffer + chunk;

        /* shift to next page */
        addr = (addr + SPINAND_PAGE_OFFSET) & (~SPINAND_PAGE_MASK);

        size -= chunk;
    }

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(EXAMPLE_FLEXSPI, NAND_CMD_LUT_FOR_IP_CMD, &seqLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_CACHE_SEQUENTIAL],
                      LUT_LENGTH);

    /* Send read from cache command */
    status = flexspi_nand_send_general_command(base, NAND_CMD_LUT_FOR_IP_CMD, 0, NULL, 0, NULL, 0);

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(EXAMPLE_FLEXSPI, NAND_CMD_LUT_FOR_IP_CMD, &customLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_FOR_IP_CMD], LUT_LENGTH);

    /* check the status register to wait bus busy */
    status = flexspi_nand_wait_bus_busy(base);

    return status;
}

/**
 * @brief program page load cache with random column address
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] inst       program instruction send to controller
 * @param[in] addr       the address of program operation
 * @param[in] buffer     buffer program data
 * @param[in] size       program data size
 */
status_t flexspi_nand_pp_load_random(FLEXSPI_Type *base, int inst, uint32_t addr, const void *buffer, uint32_t size)
{
    status_t status = kStatus_Success;

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(EXAMPLE_FLEXSPI, NAND_CMD_LUT_FOR_IP_CMD, &seqLUT[LUT_INDEX_LENGTH * inst], LUT_LENGTH);

    /* send page load command */
    status = flexspi_nand_send_general_command(base, NAND_CMD_LUT_FOR_IP_CMD, (addr & SPINAND_PAGE_MASK), buffer, size, NULL, 0);

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(base, NAND_CMD_LUT_FOR_IP_CMD, &customLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_FOR_IP_CMD], LUT_LENGTH);

    return status;
}

/**
 * @brief program a page with cloumn address
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] inst       program instruction send to controller
 * @param[in] addr       the address of program operation
 * @param[in] buffer     buffer program data
 * @param[in] size       program data size
 */
status_t flexspi_nand_send_program_command(FLEXSPI_Type *base, int inst, const void *buffer, uint32_t addr, uint32_t size)
{
    status_t status = kStatus_Success;

    /* write enable */
    status = flexspi_nand_write_enable(base);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* send page load command */
    status = flexspi_nand_send_general_command(base, inst, addr, buffer, size, NULL, 0);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* send program execute command*/
    status = flexspi_nand_send_general_command(base, NAND_CMD_LUT_SEQ_IDX_PROGRAM_EXECUTE, addr, NULL, 0, NULL, 0);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* check the status register to wait bus busy */
    status = flexspi_nand_wait_bus_busy(base);

    return status;
}
/**
 * @brief establish a BBM link by writing logical and physical block address to device's OTP memory
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] addr       the address of LBA and PBA
 */
status_t flexspi_nand_write_bbm(FLEXSPI_Type *base, uint32_t addr)
{
    status_t status = kStatus_Success;

    uint8_t enpgmReg = SPINAND_ENPGM_EN;

    /* get feature for register bits */
    status = flexspi_nand_get_feature(base, FEATURES_ADDR_ENPGM, &enpgmReg);

    if (status != kStatus_Success)
    {
        return status;
    }

    enpgmReg = enpgmReg | SPINAND_BFT_BIT_ENPGM;

    /* set feature for register bits */
    status = flexspi_nand_set_feature(base, FEATURES_ADDR_ENPGM, &enpgmReg);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* write enable */
    status = flexspi_nand_write_enable(base);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(base, NAND_CMD_LUT_FOR_IP_CMD, &seqLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_WRITE_BBM], LUT_LENGTH);

    /* send write bbm link command */
    status = flexspi_nand_send_general_command(base, NAND_CMD_LUT_FOR_IP_CMD, addr, NULL, 0, NULL, 0);

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(base, NAND_CMD_LUT_FOR_IP_CMD, &customLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_FOR_IP_CMD], LUT_LENGTH);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* program execute */
    status = flexspi_nand_send_general_command(base, NAND_CMD_LUT_SEQ_IDX_PROGRAM_EXECUTE, 0, NULL, 0, NULL, 0);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* check the status register to wait bus busy */
    status = flexspi_nand_wait_bus_busy(EXAMPLE_FLEXSPI);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* check the status register to wait bus busy */
    enpgmReg = enpgmReg & SPINAND_BFT_BIT_ENPGM_MASK;

    /* set feature for register bits */
    status = flexspi_nand_set_feature(base, 0xf0, &enpgmReg);

    return status;
}

/**
 * @brief read at most 40 links of logical and physical block from BBM table
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] size       read data size
 */
status_t flexspi_nand_read_bbm(FLEXSPI_Type *base, uint16_t *buf, uint8_t size)
{
    status_t status = kStatus_Success;

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(base, NAND_CMD_LUT_FOR_IP_CMD, &seqLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_BBM], LUT_LENGTH);

    /* read BBM table */
    status = flexspi_nand_send_general_command(base, NAND_CMD_LUT_FOR_IP_CMD, 0, NULL, 0, buf, size);

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(base, NAND_CMD_LUT_FOR_IP_CMD, &customLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_FOR_IP_CMD], LUT_LENGTH);

    return status;
}

/**
 * @brief read first and last ECC warning page address for host in continuous read mode
 *
 * @param[out] status         the return status
 * @param[in] base            FlexSPI controller base address
 * @param[in] firstAddr       first ECC warning page address
 * @param[in] lastAddr        last ECC warning page address
 */
status_t flexspi_nand_ecc_warning(FLEXSPI_Type *base, uint32_t *firstAddr, uint32_t *lastAddr)
{
    status_t status = kStatus_Success;
    uint32_t buf[2];

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(base, NAND_CMD_LUT_FOR_IP_CMD, &seqLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_ECC_WARNING], LUT_LENGTH);

    /* read ECC warning page address */
    status = flexspi_nand_send_general_command(base, NAND_CMD_LUT_FOR_IP_CMD, 0, NULL, 0, buf, 4);

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(base, NAND_CMD_LUT_FOR_IP_CMD, &customLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_FOR_IP_CMD], LUT_LENGTH);

    *firstAddr = buf[0];

    *lastAddr = buf[1];

    return status;
}

/**
 * @brief read the device vendor ID
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 * @param[in] vendorId         vendor ID of device
 */
status_t flexspi_nand_read_id(FLEXSPI_Type *base, uint8_t *vendorId)
{
    status_t status = kStatus_Success;

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(base, NAND_CMD_LUT_FOR_IP_CMD, &seqLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READID], LUT_LENGTH);

    /* send read id command */
    status = flexspi_nand_send_general_command(base, NAND_CMD_LUT_FOR_IP_CMD, 0, NULL, 0, vendorId, 1);

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(base, NAND_CMD_LUT_FOR_IP_CMD, &customLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_FOR_IP_CMD], LUT_LENGTH);

    return status;
}

/**
 * @brief initialize the FlexSPI controller for SPI NAND Flash
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 */
void flexspi_nand_flash_init(FLEXSPI_Type *base)
{
    flexspi_config_t config;
    /* To store custom's LUT table in local. */
    uint32_t tempLUT[CUSTOM_LUT_LENGTH] = {0x00U};

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
    flexspi_cache_status_t cacheStatus;
    flexspi_nand_disable_cache(&cacheStatus);
#endif

    /* Copy LUT information from flash region into RAM region, because LUT update maybe corrupt read sequence(LUT[0])
     * and load wrong LUT table from FLASH region. */
    memcpy(tempLUT, customLUT, sizeof(tempLUT));

    flexspi_clock_init();

    /*Get FLEXSPI default settings and configure the flexspi. */
    FLEXSPI_GetDefaultConfig(&config);

    /*Set AHB buffer size for reading data through AHB bus. */
    config.ahbConfig.enableAHBPrefetch    = true;
    config.ahbConfig.enableAHBBufferable  = true;
    config.ahbConfig.enableReadAddressOpt = true;
    config.ahbConfig.enableAHBCachable    = true;
    config.rxSampleClock                  = EXAMPLE_FLEXSPI_RX_SAMPLE_CLOCK;
    FLEXSPI_Init(base, &config);

    /* Configure flash settings according to serial flash feature. */
    FLEXSPI_SetFlashConfig(base, &deviceConfig, FLASH_PORT);

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(base, 0, tempLUT, CUSTOM_LUT_LENGTH);

    /* Do software reset. */
    FLEXSPI_SoftwareReset(base);

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
    flexspi_nand_enable_cache(cacheStatus);
#endif
}

/**
 * @brief set register to clear block protection
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 */
status_t flexspi_nand_clear_block_protection(FLEXSPI_Type *base)
{
    status_t status = kStatus_Success;

    uint8_t bpReg = 0;

    /* check the status register to wait bus busy */
    status = flexspi_nand_wait_bus_busy(base);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* get feature for block protection register value */
    status = flexspi_nand_get_feature(base, FEATURES_ADDR_BLOCK_PROTECTION, &bpReg);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* configure the register value */
    bpReg &= ~SPINAND_BLOCK_PROT_BIT_BP_MASK;

    /* set feature for block protection register value */
    status = flexspi_nand_set_feature(base, FEATURES_ADDR_BLOCK_PROTECTION, &bpReg);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* check the status register to wait bus busy */
    status = flexspi_nand_wait_bus_busy(base);

    return status;
}

/**
 * @brief set register to enable quad IO mode
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 */
status_t flexspi_nand_set_quad_enable(FLEXSPI_Type *base)
{
    status_t status = kStatus_Success;

    uint8_t securReg = 0;
    /* check the status register to wait bus busy */
    status = flexspi_nand_wait_bus_busy(base);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* get feature for the secure OTP register */
    status = flexspi_nand_get_feature(base, FEATURES_ADDR_SECURE_OTP, &securReg);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* assign secure register value */
    securReg |= SPINAND_SECURE_BIT_QE;

    /* set feature for the secure OTP register */
    status = flexspi_nand_set_feature(base, FEATURES_ADDR_SECURE_OTP, &securReg);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* check the status register to wait bus busy */
    status = flexspi_nand_wait_bus_busy(base);

    return status;
}

/**
 * @brief continuous read function can read page continuously with just first read latency
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] inst       read instruction send to controller
 * @param[in] addr       the address of read operation
 * @param[in] buffer     buffer to collect the output read data
 * @param[in] size       read data size
 */
status_t flexspi_nand_continuous_read(FLEXSPI_Type *base, int inst, uint32_t addr, const void *buffer, uint32_t size)
{
    status_t status = kStatus_Success;

    /* enable the continuous read mode */
    status = flexspi_nand_conti_read_enable(base);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* send page read command */
    status = flexspi_nand_send_general_command(base, NAND_CMD_LUT_SEQ_IDX_PAGE_READ, addr, NULL, 0, NULL, 0);

    if (status != kStatus_Success)
    {
        return status;
    }

    status = flexspi_nand_wait_bus_busy(base);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* send read from cache command*/
    status = flexspi_nand_send_general_command(base, inst, addr, NULL, 0, buffer, size);

    if (status != kStatus_Success)
    {
        return status;
    }

    status = flexspi_nand_conti_read_disable(base);

    return status;
}

/**
 * @brief send command to reset the device, the status register will be cleared
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 */
status_t flexspi_nand_reset(FLEXSPI_Type *base)
{
    status_t status = kStatus_Success;

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(base, NAND_CMD_LUT_FOR_IP_CMD, &seqLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_RESET], LUT_LENGTH);

    /* send reset command */
    status = flexspi_nand_send_general_command(base, NAND_CMD_LUT_FOR_IP_CMD, 0, NULL, 0, NULL, 0);

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(base, NAND_CMD_LUT_FOR_IP_CMD, &customLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_FOR_IP_CMD], LUT_LENGTH);

    /* wait 500 micro seconds to enter deep power down mode */
    delayUs(500);

    return status;
}

/**
 * @brief check status register to wait bus ready
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 */
status_t flexspi_nand_wait_bus_busy(FLEXSPI_Type *base)
{
    bool isBusy;
    uint8_t readValue;
    status_t status = kStatus_Success;

    do
    {
        /* get feature for status regisger */
        status = flexspi_nand_get_feature(base, FEATURES_ADDR_STATUS, &readValue);

        if (status != kStatus_Success)
        {
            return status;
        }

        /* compare the register value with pre-defined value */
        if (readValue & SPINAND_STATUS_BIT_WIP)
        {
            isBusy = true;
        } else {
            isBusy = false;
        }

    } while (isBusy);

    return status;
}

/**
 * @brief send write enable command to make device writable
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 */
status_t flexspi_nand_write_enable(FLEXSPI_Type *base)
{
    status_t status = kStatus_Success;

    /* send write enable command*/
    status = flexspi_nand_send_general_command(base, NAND_CMD_LUT_SEQ_IDX_WRITEENABLE, 0, NULL, 0, NULL, 0);

    return status;
}

/**
 * @brief send command to disable the program function
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 */
status_t flexspi_nand_write_disable(FLEXSPI_Type *base)
{
    status_t status = kStatus_Success;

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(base, NAND_CMD_LUT_FOR_IP_CMD, &seqLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_WRITE_DISABLE], LUT_LENGTH);

    /* send write disable command*/
    status = flexspi_nand_send_general_command(base, NAND_CMD_LUT_FOR_IP_CMD, 0, NULL, 0, NULL, 0);

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(base, NAND_CMD_LUT_FOR_IP_CMD, &customLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_FOR_IP_CMD], LUT_LENGTH);

    return status;
}

/**
 * @brief get feature with specific status address
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 * @param[in] addr             register address
 * @param[in] val              register read value
 *
 */
status_t flexspi_nand_get_feature(FLEXSPI_Type *base, uint8_t addr, uint8_t *val)
{
    status_t status = kStatus_Success;

    /* send get feature command*/
    status = flexspi_nand_send_general_command(base, NAND_CMD_LUT_SEQ_IDX_GETFEATURE, addr, NULL, 0, val, 1);

    return status;
}

/**
 * @brief send command to read the status register value
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 * @param[in] val              read out status register value
 *
 */
status_t flexspi_nand_read_status(FLEXSPI_Type *base, uint8_t *val)
{
    status_t status = kStatus_Success;

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(base, NAND_CMD_LUT_FOR_IP_CMD, &seqLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_STATUS], LUT_LENGTH);

    /* send read status command */
    status = flexspi_nand_send_general_command(base, NAND_CMD_LUT_FOR_IP_CMD, 0, NULL, 0, val, 1);

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(base, NAND_CMD_LUT_FOR_IP_CMD, &customLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_FOR_IP_CMD], LUT_LENGTH);

    return status;
}

/**
 * @brief read ECC status register
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 * @param[in] val              ECC status register read value
 */
status_t flexspi_nand_read_ecc_status(FLEXSPI_Type *base, uint8_t *val)
{
    status_t status = kStatus_Success;

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(base, NAND_CMD_LUT_FOR_IP_CMD, &seqLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_ECCSR], LUT_LENGTH);

    /* send read ECC status register command*/
    status = flexspi_nand_send_general_command(base, NAND_CMD_LUT_FOR_IP_CMD, 0, NULL, 0, val, 1);

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(base, NAND_CMD_LUT_FOR_IP_CMD, &customLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_FOR_IP_CMD], LUT_LENGTH);

    return status;
}

/**
 * @brief set feature with specific status address
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 * @param[in] val              register read value
 * @param[in] addr             register address
 */
status_t flexspi_nand_set_feature(FLEXSPI_Type *base, uint8_t addr, uint8_t *val)
{
    status_t status = kStatus_Success;

    /* send set feature command*/
    status = flexspi_nand_send_general_command(base, NAND_CMD_LUT_SEQ_IDX_SETFEATURE, addr, val, 1, NULL, 0);

    return status;
}

/**
 * @brief enable the continuous read mode
 *
 * @param[out] status                 the return status
 * @param[in] base                    FlexSPI controller base address
 */
status_t flexspi_nand_conti_read_enable(FLEXSPI_Type *base)
{
    uint8_t securReg = 0;
    status_t status = kStatus_Success;

    /* check the status register to wait bus busy */
    status = flexspi_nand_wait_bus_busy(base);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* get feature for secure OTP register */
    status = flexspi_nand_get_feature(base, FEATURES_ADDR_SECURE_OTP, &securReg);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* configure the secure OTP register value */
    securReg |= SPINAND_SECURE_BIT_CONT;

    /* set feature for secure OTP register */
    status = flexspi_nand_set_feature(base, FEATURES_ADDR_SECURE_OTP, &securReg);

    return status;
}

/**
 * @brief disable the continuous read mode
 *
 * @param[out] status                 the return status
 * @param[in] base                    FlexSPI controller base address
 */
status_t flexspi_nand_conti_read_disable(FLEXSPI_Type *base)
{
    uint8_t securReg = 0;
    status_t status = kStatus_Success;

    /* check the status register to wait bus busy */
    status = flexspi_nand_wait_bus_busy(base);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* get feature for secure OTP register */
    status = flexspi_nand_get_feature(base, FEATURES_ADDR_SECURE_OTP, &securReg);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* configure the secure OTP register value */
    securReg &= ~SPINAND_SECURE_BIT_CONT;

    /* set feature for secure OTP register */
    status = flexspi_nand_set_feature(base, FEATURES_ADDR_SECURE_OTP, &securReg);

    return status;
}

/**
 * @brief set FlexSPI controller to transmit the command code and configure the register
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 * @param[in] inst             instruction index in LUT
 * @param[in] addr             send address to FlexSPI
 * @param[in] txBuffer         transmit data buffer
 * @param[in] rxBuffer         receive data buffer
 * @param[in] txLength         transmit data length
 * @param[in] rxLength         receive data length
 */
status_t flexspi_nand_send_general_command(FLEXSPI_Type *base, int inst, uint32_t addr, const char *txBuffer, uint32_t txLength,
		const char *rxBuffer, uint32_t rxLength)
{
    flexspi_transfer_t flashXfer;
    status_t status = kStatus_Success;
    flexspi_command_type_t cmdType = 0;
    uint32_t size;
    uint32_t *data = 0;

    /* configure the cmdType parameter */
    if ((txBuffer == NULL || txLength == 0) && (rxBuffer == NULL || rxLength == 0))
    {
        cmdType = kFLEXSPI_Command;
    } else {
        if (txBuffer != NULL && txLength)
        {
            size = txLength;
            data = (uint32_t *)txBuffer;
            cmdType = kFLEXSPI_Write;
        }

        if (rxBuffer != NULL && rxLength)
        {
            size = rxLength;
            data = (uint32_t *)rxBuffer;
            cmdType = kFLEXSPI_Read;
        }
    }

    /* configure the controller register value */
    flashXfer.deviceAddress = addr;
    flashXfer.port          = FLASH_PORT;
    flashXfer.cmdType       = cmdType;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = inst;
    flashXfer.data          = data;
    flashXfer.dataSize      = size;

    /* set controller */
    status = FLEXSPI_TransferBlocking(base, &flashXfer);

    /* software reset the FlexSPI controller*/
    FLEXSPI_SoftwareReset(base);

    return status;
}

/**
 * @brief read onfi table parameter to configure the Flash driver
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 */
#if (defined CACHE_MAINTAIN) && (CACHE_MAINTAIN == 1)
void flexspi_nand_disable_cache(flexspi_cache_status_t *cacheStatus)
{
#if (defined __CORTEX_M) && (__CORTEX_M == 7U)
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    /* Disable D cache. */
    if (SCB_CCR_DC_Msk == (SCB_CCR_DC_Msk & SCB->CCR))
    {
        SCB_DisableDCache();
        cacheStatus->DCacheEnableFlag = true;
    }
#endif /* __DCACHE_PRESENT */

#if defined(__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1U)
    /* Disable I cache. */
    if (SCB_CCR_IC_Msk == (SCB_CCR_IC_Msk & SCB->CCR))
    {
        SCB_DisableICache();
        cacheStatus->ICacheEnableFlag = true;
    }
#endif /* __ICACHE_PRESENT */

#elif (defined FSL_FEATURE_SOC_LMEM_COUNT) && (FSL_FEATURE_SOC_LMEM_COUNT != 0U)
    /* Disable code bus cache and system bus cache */
    if (LMEM_PCCCR_ENCACHE_MASK == (LMEM_PCCCR_ENCACHE_MASK & LMEM->PCCCR))
    {
        L1CACHE_DisableCodeCache();
        cacheStatus->codeCacheEnableFlag = true;
    }
    if (LMEM_PSCCR_ENCACHE_MASK == (LMEM_PSCCR_ENCACHE_MASK & LMEM->PSCCR))
    {
        L1CACHE_DisableSystemCache();
        cacheStatus->systemCacheEnableFlag = true;
    }

#elif (defined FSL_FEATURE_SOC_CACHE64_CTRL_COUNT) && (FSL_FEATURE_SOC_CACHE64_CTRL_COUNT != 0U)
    /* Disable cache */
    CACHE64_DisableCache(EXAMPLE_CACHE);
    cacheStatus->CacheEnableFlag = true;
#endif
}

/**
 * @brief enable the I cache and D cache if defined
 *
 * @param[in] cacheStatus        FlexSPI controller cache status
 */
void flexspi_nand_enable_cache(flexspi_cache_status_t cacheStatus)
{
#if (defined __CORTEX_M) && (__CORTEX_M == 7U)
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    if (cacheStatus.DCacheEnableFlag)
    {
        /* Enable D cache. */
        SCB_EnableDCache();
    }
#endif /* __DCACHE_PRESENT */

#if defined(__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1U)
    if (cacheStatus.ICacheEnableFlag)
    {
        /* Enable I cache. */
        SCB_EnableICache();
    }
#endif /* __ICACHE_PRESENT */

#elif (defined FSL_FEATURE_SOC_LMEM_COUNT) && (FSL_FEATURE_SOC_LMEM_COUNT != 0U)
    if (cacheStatus.codeCacheEnableFlag)
    {
        /* Enable code cache. */
        L1CACHE_EnableCodeCache();
    }

    if (cacheStatus.systemCacheEnableFlag)
    {
        /* Enable system cache. */
        L1CACHE_EnableSystemCache();
    }

#elif (defined FSL_FEATURE_SOC_CACHE64_CTRL_COUNT) && (FSL_FEATURE_SOC_CACHE64_CTRL_COUNT != 0U)
    if (cacheStatus.CacheEnableFlag)
    {
        /* Enable cache. */
        CACHE64_EnableCache(EXAMPLE_CACHE);
    }
#endif
}
#endif

/**
 * @brief enter deep power down mode to place the device into a minimum power consumption state
 *
 * @param[out] status      the return status
 * @param[in] base         FlexSPI controller base address
 */
status_t flexspi_nand_deep_down(FLEXSPI_Type *base)
{
    status_t status = kStatus_Success;

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(base, NAND_CMD_LUT_FOR_IP_CMD, &seqLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_DP], LUT_LENGTH);

    /* send deep power down command */
    status = flexspi_nand_send_general_command(base, NAND_CMD_LUT_FOR_IP_CMD, 0, NULL, 0, NULL, 0);

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(base, NAND_CMD_LUT_FOR_IP_CMD, &customLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_FOR_IP_CMD], LUT_LENGTH);

    /* wait at most 6000 micro seconds to enter deep power down mode */
    delayUs(6000);

    return status;
}

/**
 * @brief enable the read protocol of SPI NOR like for Read From Cache commands
 *
 * @param[out] status      the return status
 * @param[in] base         FlexSPI controller base address
 */
status_t flexspi_nand_spi_nor_enable(FLEXSPI_Type *base)
{
    status_t status = kStatus_Success;

    uint8_t enpgmReg = 0, spinorReg = 0;

    /* check the status register to wait bus busy */
    status = flexspi_nand_wait_bus_busy(base);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* get feature for the register */
    status = flexspi_nand_get_feature(base, FEATURES_ADDR_ENPGM, &enpgmReg);

    if (status != kStatus_Success)
    {
        return status;
    }

    enpgmReg = enpgmReg | SPINAND_BFT_BIT_ENPGM;

    /* set feature for the register */
    status = flexspi_nand_set_feature(base, FEATURES_ADDR_ENPGM, &enpgmReg);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* get feature for the register */
    status = flexspi_nand_get_feature(base, FEATURES_ADDR_SPI_NOR_EN, &spinorReg);

    if (status != kStatus_Success)
    {
        return status;
    }

    spinorReg = spinorReg | SPINAND_SPINOR_BIT_EN;

    /* set feature for the register */
    status = flexspi_nand_set_feature(base, FEATURES_ADDR_SPI_NOR_EN, &spinorReg);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* write enable */
    status = flexspi_nand_write_enable(base);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* send program execute command */
    status = flexspi_nand_send_general_command(base, NAND_CMD_LUT_SEQ_IDX_PROGRAM_EXECUTE, 0, NULL, 0, NULL, 0);

    if (status != kStatus_Success)
    {
        return status;
    }

    enpgmReg = enpgmReg & SPINAND_BFT_BIT_ENPGM_MASK;

    /* set feature for the register */
    status = flexspi_nand_set_feature(base, FEATURES_ADDR_ENPGM, &enpgmReg);

    return status;
}
