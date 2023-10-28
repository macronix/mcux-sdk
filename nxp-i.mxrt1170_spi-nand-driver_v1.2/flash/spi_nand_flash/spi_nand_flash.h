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
#ifndef SPI_NAND_FLASH_H
#define SPI_NAND_FLASH_H

#include "fsl_flexspi.h"
#include "flexspi_nand_flash_ops.h"
#include "fsl_debug_console.h"
#include <stdbool.h>

#define SPINAND_INST_READ_DEFAULT          NAND_CMD_LUT_SEQ_IDX_READ_CACHE
//#define SPINAND_INST_READ_DEFAULT          NAND_CMD_LUT_SEQ_IDX_READ_CACHE_112
//#define SPINAND_INST_READ_DEFAULT          NAND_CMD_LUT_SEQ_IDX_READ_CACHE_114
//#define SPINAND_INST_READ_DEFAULT          NAND_CMD_LUT_SEQ_IDX_READ_CACHE_144
#define SPINAND_INST_PROGRAM_DEFAULT       NAND_CMD_LUT_SEQ_IDX_PAGE_LOAD
//#define SPINAND_INST_PROGRAM_DEFAULT       NAND_CMD_LUT_SEQ_IDX_PAGE_LOAD_114

#define SPINAND_BLOCK_SIZE            0x20000U
#define SPINAND_PAGE_SIZE             2048
#define SPINAND_ECC_THRESHOLD         8
#define SPINAND_READ_RECOVERY_MODE    5
#define SPINAND_VENDOR_ID             0xc2

#define SPINAND_ONFI_LENGTH  256

#define SPINAND_ONFI_SHIFT_0 0
#define SPINAND_ONFI_SHIFT_1 1
#define SPINAND_ONFI_SHIFT_2 2
#define SPINAND_ONFI_SHIFT_3 3
#define SPINAND_ONFI_SHIFT_6 6
#define SPINAND_ONFI_SHIFT_7 7
#define SPINAND_ONFI_SHIFT_8 8
#define SPINAND_ONFI_SHIFT_80 80
#define SPINAND_ONFI_SHIFT_16 16
#define SPINAND_ONFI_SHIFT_81 81
#define SPINAND_ONFI_SHIFT_82 82
#define SPINAND_ONFI_SHIFT_92 92
#define SPINAND_ONFI_SHIFT_84 84
#define SPINAND_ONFI_SHIFT_85 85
#define SPINAND_ONFI_SHIFT_93 93
#define SPINAND_ONFI_SHIFT_96 96
#define SPINAND_ONFI_SHIFT_97 97
#define SPINAND_ONFI_SHIFT_112 112
#define SPINAND_ONFI_SHIFT_167 167
#define SPINAND_ONFI_SHIFT_168 168

typedef struct spi_nand_config {
    uint8_t *ecc_code;

    uint32_t page_size, block_size, flash_size;

    uint8_t page_shift, block_shift;

    uint16_t block_num, page_num, oob_size;

    uint8_t ecc_bits, ecc_bytes, ecc_steps, ecc_layout_pos;

    uint32_t  ecc_size;

    uint8_t *ecc_calc;

    uint8_t *page_buf;

    bool read_recovery;

    bool continuous_read;
}spi_nand_config_t;

/**
 * @brief change read/program IO mode with different instruction
 *
 * @param[out] status           the return status
 * @param[in]  readInst     read instruction index in LUT
 * @param[in]  programInst  program instruction index in LUT
 * @param[in]  base          FlexSPI controller base address
 */
void nand_change_io_mode(FLEXSPI_Type *base, int readInst, int programInst);

/**
 * @brief normal read function
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] addr       the address of read operation
 * @param[in] buffer     buffer to collect the output read data
 * @param[in] size       read data size
 */

status_t spi_nand_read_normal(FLEXSPI_Type *base, uint32_t addr, const void *buffer, uint32_t size);

/**
 * @brief software ECC read function
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] addr       the address of read operation
 * @param[in] buffer     buffer to collect the output read data
 * @param[in] size       read data size
 */
status_t spi_nand_read_software_ecc(FLEXSPI_Type *base, uint32_t addr, const void *buffer, uint32_t size);

/**
 * @brief BCH initialization function
 *
 * @param[in]  ecc_bits    the Flash device support ECC bits
 */
void bch_ecc_init(uint8_t ecc_bits);

/**
 * @brief BCH calculate ecc code for the to-be-programmed data
 *
 * @param[in]  buf    pointer to the data buffer
 * @param[in]  code   pointer to the ecc code
 */
void bch_calculate_ecc(unsigned char *buf, unsigned char *code);

/**
 * @brief read otp onfi table at the initialization stage
 *
 * @param[in] base   FlexSPI controller base address
 */
status_t read_otp_onfi(FLEXSPI_Type *base);

/**
 * @brief erase function
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] size       erase size
 */
status_t spi_nand_erase(FLEXSPI_Type *base, uint32_t addr, uint32_t size);

/**
 * @brief read function
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] addr       the address of read operation
 * @param[in] buffer     buffer to collect the output read data
 * @param[in] size       read data size
 */
status_t spi_nand_read(FLEXSPI_Type *base, uint32_t addr, const void *buffer, uint32_t size);

/**
 * @brief write data function
 *
 * @param[out] status           the return status
 * @param[in] base              FlexSPI controller base address
 * @param[in] addr              the address of write operation
 * @param[in] buffer            buffer write data
 * @param[in] size              write data size
 */
status_t spi_nand_write(FLEXSPI_Type *base, uint32_t addr, const void *buffer, uint32_t size);

/**
 * @brief SPI NAND Flash initialization
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 */
status_t spi_nand_flash_init(FLEXSPI_Type *base);

/**
 * @brief read oob area
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] addr       the address of read oob operation
 * @param[in] buffer     buffer to collect the output oob area data
 * @param[in] size       read data size
 */
status_t spi_nand_read_oob(FLEXSPI_Type *base, uint32_t addr, const void *buffer, uint32_t size);

/**
 * @brief write oob area
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] addr       the address of program oob area operation
 * @param[in] buffer     buffer for write data
 * @param[in] size       read data size
 */
status_t spi_nand_write_oob(FLEXSPI_Type *base, uint32_t addr, const void *buffer, uint32_t size);
#endif
