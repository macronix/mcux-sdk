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
#ifndef FLEXSPI_NAND_FLASH_OPS_H
#define FLEXSPI_NAND_FLASH_OPS_H

#include "fsl_debug_console.h"
#include "fsl_flexspi.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CACHE_MAINTAIN 1

#define PAGE_SHIFT_12 12
#define PAGE_SHIFT_13 13

#define READ_RECOVERY 0
#define SPI_NOR_EN  0
// Get/Set Feature Address Definition
#define FEATURES_ADDR_BLOCK_PROTECTION 0xA0
#define FEATURES_ADDR_SECURE_OTP       0xB0
#define FEATURES_ADDR_STATUS           0xC0
#define FEATURES_ADDR_SPI_NOR_EN       0x60
#define FEATURES_ADDR_IO_STRENGTH      0xE0
#define FEATURES_ADDR_ENPGM            0x10
#define FEATURES_ADDR_SPEC             0x70
#define NAND_CMD_LUT_FOR_IP_CMD        0

// Status Register Bits
#define SPINAND_STATUS_BIT_WIP               0x1  // Write In Progress
#define SPINAND_STATUS_BIT_WEL               0x2  // Write Enable Latch
#define SPINAND_STATUS_BIT_ERASE_FAIL        0x4  // Erase failed
#define SPINAND_STATUS_BIT_PROGRAM_FAIL      0x8  // Program failed
#define SPINAND_STATUS_BIT_ECC_STATUS_MASK   0x30 // ECC status
#define SPINAND_STATUS_ECC_STATUS_NO_ERR     0x00
#define SPINAND_STATUS_ECC_STATUS_ERR_COR    0x10
#define SPINAND_STATUS_ECC_STATUS_ERR_NO_COR 0x20

#define SPINAND_BFT_BIT_ENPGM       0x01
#define SPINAND_BFT_BIT_ENPGM_MASK  0xFE
#define SPINAND_BFT_BIT_RANDEN      0x02
#define SPINAND_BFT_BIT_RANDOPT     0x04

#define SPINAND_IO_STRENGTH_MAX     0x80

#define SPINAND_SPEC_BIT_RD1   0x01
#define SPINAND_SPEC_BIT_RD2   0x02
#define SPINAND_SPEC_BIT_RD3   0x04
#define SPINAND_SPINOR_BIT_EN  0x2

// Secure OTP Register Bits
#define SPINAND_SECURE_BIT_QE       0x01  // Quad enable
#define SPINAND_SECURE_BIT_CONT     0x04  // continuous read enable
#define SPINAND_SECURE_BIT_ECC_EN   0x10  // On-die ECC enable
#define SPINAND_SECURE_BIT_OTP_EN   0x40  //
#define SPINAND_SECURE_BIT_OTP_PROT 0x80  //

// Block Protection Register Bits
#define  SPINAND_BLOCK_PROT_BIT_SP      0x01
#define  SPINAND_BLOCK_PROT_BIT_COMPLE  0x02
#define  SPINAND_BLOCK_PROT_BIT_INVERT  0x04
#define  SPINAND_BLOCK_PROT_BIT_BP0     0x08
#define  SPINAND_BLOCK_PROT_BIT_BP1     0x10
#define  SPINAND_BLOCK_PROT_BIT_BP2     0x20
#define  SPINAND_BLOCK_PROT_BIT_BPRWD   0x80
#define  SPINAND_BLOCK_PROT_BIT_BP_MASK 0x38
#define  SPINAND_BLOCK_PROT_BIT_MASK    0x87
#define  SPINAND_ENPGM_EN               0xf1

#define  SPINAND_BLOCK_PROT_BP_OFFSET     3
#define  SPINAND_BLOCK_PROT_COMPLE_OFFSET 1

#define IS_MEM_READY_MAX_RETRIES 10000

#define SPINAND_PAGE_OFFSET  0x1000
#define SPINAND_BLOCK_OFFSET 0x40000
#define SPINAND_BLOCK_MASK   0x3FFFF
#define SPINAND_PAGE_MASK    0xFFF
#define SPINAND_ECC_SIZE     0x200
#define SPINAND_ECC_BYTE     0x10
#define SPINAND_OOB_SIZE     0x40

#define LUT_SHIFT      13
#define LUT_LENGTH     16
#define LUT_BASE_INDEX 52U
#define LUT_INDEX_LENGTH 4

#define NAND_CMD_LUT_SEQ_IDX_READ_CACHE             0
#define NAND_CMD_LUT_SEQ_IDX_PAGE_LOAD              1
#define NAND_CMD_LUT_SEQ_IDX_READ_CACHE_122         2
#define NAND_CMD_LUT_SEQ_IDX_PAGE_LOAD_114          3
#define NAND_CMD_LUT_SEQ_IDX_READ_CACHE_112         4
#define NAND_CMD_LUT_SEQ_IDX_READ_CACHE_114         5
#define NAND_CMD_LUT_SEQ_IDX_GETFEATURE             6
#define NAND_CMD_LUT_SEQ_IDX_PAGE_READ              7
#define NAND_CMD_LUT_SEQ_IDX_READ_CACHE_ALTERNATIVE 8
#define NAND_CMD_LUT_SEQ_IDX_ERASE                  9
#define NAND_CMD_LUT_SEQ_IDX_READ_CACHE_144         10
#define NAND_CMD_LUT_SEQ_IDX_SETFEATURE             11
#define NAND_CMD_LUT_SEQ_IDX_PROGRAM_EXECUTE        12
#define NAND_CMD_LUT_SEQ_IDX_WRITEENABLE            13
#define NAND_CMD_LUT_SEQ_IDX_PP_LOAD_RANDOM_114     14
#define NAND_CMD_LUT_SEQ_IDX_PAGE_LOAD_RANDOM       15

#define NAND_CMD_LUT_SEQ_IDX_READ_SEQUENTIAL_END   0
#define NAND_CMD_LUT_SEQ_IDX_WRITE_DISABLE         1
#define NAND_CMD_LUT_SEQ_IDX_RESET                 2
#define NAND_CMD_LUT_SEQ_IDX_READ_CACHE_SEQUENTIAL 3
#define NAND_CMD_LUT_SEQ_IDX_READ_CACHE_RANDOM     4
#define NAND_CMD_LUT_SEQ_IDX_READ_STATUS           5
#define NAND_CMD_LUT_SEQ_IDX_READID                6
#define NAND_CMD_LUT_SEQ_IDX_READ_ECCSR            7
#define NAND_CMD_LUT_SEQ_IDX_ECC_WARNING           8
#define NAND_CMD_LUT_SEQ_IDX_WRITE_BBM             9
#define NAND_CMD_LUT_SEQ_IDX_READ_BBM              10
#define NAND_CMD_LUT_SEQ_IDX_DP                    11

/* General SPI NAND Flash instructions */
#define SPINAND_INST_RDID            0x9F
#define SPINAND_INST_RSR1            0x05

#define SPINAND_INST_PAGE_READ       0x13
#define SPINAND_INST_READ_CACHE      0x03
#define SPINAND_INST_READ_CACHE2     0x3B
#define SPINAND_INST_READ_CACHE4     0x6B
#define SPINAND_INST_READ_CACHE144   0xEB
#define SPINAND_INST_READ_CACHE_SEQ  0x31
#define SPINAND_INST_READ_CACHE_END  0x3F
#define SPINAND_INST_READ_CACHE_ALT  0x0B

#define SPINAND_INST_WREN            0x06
#define SPINAND_INST_RESET           0xFF
#define SPINAND_INST_WRDI            0x04
#define SPINAND_INST_PP_LOAD         0x02
#define SPINAND_INST_PP_RAND_LOAD    0x84
#define SPINAND_INST_4PP_LOAD        0x32
#define SPINAND_INST_4PP_RAND_LOAD   0x34
#define SPINAND_INST_PROGRAM_EXEC    0x10
#define SPINAND_INST_BE              0xD8
#define SPINAND_INST_DP              0xB9
#define SPINAND_INST_WRITE_BBM       0xA1
#define SPINAND_INST_ECC_WARNING     0xA9
#define SPINAND_INST_READ_BBM        0xA5

#define SPINAND_INST_GET_FEATURE     0x0F
#define SPINAND_INST_SET_FEATURE     0x1F
#define SPINAND_INST_RESET           0xFF
#define SPINAND_INST_ECC_STAT_READ   0x7C

#define SPINAND_CFG_ADDR_32          0x20
#define SPINAND_CFG_ADDR_24          0x18
#define SPINAND_CFG_ADDR_16          0x10
#define SPINAND_CFG_ADDR_8           0x08
#define SPINAND_CFG_DUMMY_8          0x08
#define SPINAND_CFG_DUMMY_4          0x04

#define EXAMPLE_FLEXSPI                 FLEXSPI2
#define NAND_FLASH_SIZE                 0x40000 /*0x8000000 128MB, the max is 256MB*/
#define EXAMPLE_FLEXSPI_AMBA_BASE       FlexSPI2_AMBA_BASE
#define EXAMPLE_FLEXSPI_CLOCK           kCLOCK_Flexspi2
#define FLASH_PORT                      kFLEXSPI_PortA1
#define EXAMPLE_FLEXSPI_RX_SAMPLE_CLOCK kFLEXSPI_ReadSampleClkLoopbackInternally

#define CUSTOM_LUT_LENGTH            64U
#define SEQ_LUT_LENGTH               48U

/**
 * @brief send page read and read from cache command to read out at most a page data
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] addr       the address of read operation
 * @param[in] buffer     buffer to collect the output read data
 * @param[in] size       read data size
 */

status_t flexspi_nand_send_read_command(FLEXSPI_Type *base, int inst, const void *buffer, uint32_t addr, uint32_t size);

/**
 * @brief enable the continuous read mode
 *
 * @param[out] status                 the return status
 * @param[in] base                    FlexSPI controller base address
 */
status_t flexspi_nand_conti_read_enable(FLEXSPI_Type *base);

/**
 * @brief disable the continuous read mode
 *
 * @param[out] status                 the return status
 * @param[in] base                    FlexSPI controller base address
 */
status_t flexspi_nand_conti_read_disable(FLEXSPI_Type *base);

/**
 * @brief send write enable command to make device writable
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 */
status_t flexspi_nand_write_enable(FLEXSPI_Type *base);

/**
 * @brief program page load cache with random column address
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] addr       the address of program operation
 * @param[in] buffer     buffer program data
 * @param[in] size       program data size
 */
status_t flexspi_nand_pp_load_random(FLEXSPI_Type *base, int instruction, uint32_t addr, const void *buffer, uint32_t size);

/**
 * @brief read onfi table parameter to configure the Flash driver
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 */
status_t flexspi_nand_read_otp_onfi(FLEXSPI_Type *base);

/**
 * @brief read first and last ECC warning page address for host in continuous read mode
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 * @param[in] firstAddr       first ECC warning page address
 * @param[in] lastAddr        last ECC warning page address
 */
status_t flexspi_nand_ecc_warning(FLEXSPI_Type *base, uint32_t *firstAddr, uint32_t *lastAddr);

/**
 * @brief send command to disable the program function
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 */
status_t flexspi_nand_write_disable(FLEXSPI_Type *base);

/**
 * @brief send command to read the status register value
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 * @param[in] addr             register address
 */
status_t flexspi_nand_read_status(FLEXSPI_Type *base, uint8_t *val);

/**
 * @brief establish a BBM link by writing logical and physical block address to device's OTP memory
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] addr       the address of LBA and PBA
 */
status_t flexspi_nand_write_bbm(FLEXSPI_Type *base, uint32_t addr);

/**
 * @brief read at most 40 links of logical and physical block from BBM table
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] size       read data size
 */
status_t flexspi_nand_read_bbm(FLEXSPI_Type *base, uint16_t *buf, uint8_t size);

/**
 * @brief check status register to wait bus ready
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 */
status_t flexspi_nand_wait_bus_busy(FLEXSPI_Type *base);

/**
 * @brief set register to clear block protection
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 */
status_t flexspi_nand_clear_block_protection(FLEXSPI_Type *base);

/**
 * @brief set register to enable quad IO mode
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 */
status_t flexspi_nand_set_quad_enable(FLEXSPI_Type *base);

/**
 * @brief terminate the sequential read mode function
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 */
status_t flexspi_nand_read_cache_sequential_end(FLEXSPI_Type *base);

/**
 * @brief enable the read cache sequential mode function
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 */
status_t flexspi_nand_read_cache_sequential(FLEXSPI_Type *base);

/**
 * @brief erase the device by block
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] addr       the block address of erase operation
 */
status_t flexspi_nand_erase_block(FLEXSPI_Type *base, uint32_t addr);

/**
 * @brief read at most a single page and check the internal ECC status
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] addr       the address of read operation
 * @param[in] buffer     buffer to collect the output read data
 * @param[in] size       read data size
 */
status_t flexspi_nand_read_page(FLEXSPI_Type *base, int inst, uint32_t addr, const void *buffer, uint32_t size);

/**
 * @brief send page program load and page program execute command to program at most a single page size data
 *
 * @param[out] status           the return status
 * @param[in] base              FlexSPI controller base address
 * @param[in] addr              the address of program operation
 * @param[in] buffer            buffer program data
 * @param[in] size              program data size
 * @param[in] instruction       program instruction
 */
status_t flexspi_nand_send_program_command(FLEXSPI_Type *base, int inst, const void *buffer, uint32_t addr, uint32_t size);

/**
 * @brief program a page with cloumn address
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] addr       the address of program operation
 * @param[in] buffer     buffer program data
 * @param[in] size       program data size
 */
status_t flexspi_nand_program_page(FLEXSPI_Type *base, int inst, uint32_t addr, const void *buffer, uint32_t size);

/**
 * @brief read the device vendor ID
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 * @param[in] vendorId         vendor ID of device
 */
status_t flexspi_nand_read_id(FLEXSPI_Type *base, uint8_t *fullId);

/**
 * @brief initialize the FlexSPI controller for SPI NAND Flash
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 */
void flexspi_nand_flash_init(FLEXSPI_Type *base);

/**
 * @brief set FlexSPI controller to transmit the command code and configure the register
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 * @param[in] inst             instruction index in LUT
 * @param[in] addr             send address to FlexSPI
 * @param[in] txBuffer        transmit data buffer
 * @param[in] rxBuffer        receive data buffer
 * @param[in] txLength        transmit data length
 * @param[in] rxLength        receive data length
 */
status_t flexspi_nand_send_general_command(FLEXSPI_Type *base, int inst, uint32_t addr, const char *txBuffer, uint32_t txLength,
		const char *rxBuffer, uint32_t rxLength);

/**
 * @brief get feature with specific status address
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 * @param[in] addr             register address
 */
status_t flexspi_nand_get_feature(FLEXSPI_Type *base, uint8_t addr, uint8_t *val);

/**
 * @brief set feature with specific status address
 *
 * @param[out] status          the return status
 * @param[in] base             FlexSPI controller base address
 * @param[in] val              ECC status register read value
 * @param[in] addr             register address
 */
status_t flexspi_nand_set_feature(FLEXSPI_Type *base, uint8_t addr, uint8_t *val);
#endif
