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

#include "spi_nand_flash.h"
#include "fsl_debug_console.h"
#include "bch.h"

struct nand_bch_control {
    struct bch_code *bch;
    unsigned int    *errloc;
    unsigned char   *eccmask;
};

struct nand_bch_control nbc;

int readInstruction, programInstruction;

spi_nand_config_t nandConfig;

/**
 * @brief read function
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] addr       the address of read operation
 * @param[in] buffer     buffer to collect the output read data
 * @param[in] size       read data size
 */
status_t spi_nand_read(FLEXSPI_Type *base, uint32_t addr, const void *buffer, uint32_t size)
{
    status_t status = kStatus_Success;

    if (nandConfig.ecc_bits == 0) {

        if (nandConfig.continuous_read == true) {

    	    /* enter continuous read mode */
            flexspi_nand_continuous_read(base, readInstruction, addr, buffer, size);
    	} else {

    	    /* enter normal read mode */
    	    spi_nand_read_normal(base, addr, buffer, size);
    	}
    } else {

        /* enter software ecc read mode */
    	spi_nand_read_software_ecc(base, addr, buffer, size);
    }

    return status;
}

/**
 * @brief normal read function
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] addr       the address of read operation
 * @param[in] buffer     buffer to collect the output read data
 * @param[in] size       read data size
 */
status_t spi_nand_read_normal(FLEXSPI_Type *base, uint32_t addr, const void *buffer, uint32_t size)
{
    status_t status = kStatus_Success;
    uint8_t statusReg;

    uint32_t offset = 0;
    uint32_t chunk = 0;
    uint32_t readBytes = 0;

    while (size > 0) {
        /* Read on page_size_bytes boundaries (Default 2048 bytes a page) */
        offset = addr % nandConfig.page_size;
        chunk = (offset + size < nandConfig.page_size) ? size : (nandConfig.page_size - offset);
        readBytes = chunk;

        /* normal read page function */
    	status = flexspi_nand_send_read_command(base, readInstruction, buffer, addr, readBytes);

        if (status != kStatus_Success)
        {
            return status;
        }

        /* get feature for status register bits */
        status = flexspi_nand_get_feature(base, FEATURES_ADDR_STATUS, &statusReg);

        if (status != kStatus_Success)
        {
            return status;
        }

        /* check status register bit ECC_S[1:0] */
        if ((statusReg & SPINAND_STATUS_BIT_ECC_STATUS_MASK) == SPINAND_STATUS_ECC_STATUS_ERR_NO_COR)
        {
            return kStatus_Fail;
        }

        buffer = (uint8_t *)buffer + chunk;

        /* shift to next page */
        addr = (addr + SPINAND_PAGE_OFFSET) & (~SPINAND_PAGE_MASK);
        size -= chunk;
    }
}

/**
 * @brief software ecc read function
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] addr       the address of read operation
 * @param[in] buffer     buffer to collect the output read data
 * @param[in] size       read data size
 */
status_t spi_nand_read_software_ecc(FLEXSPI_Type *base, uint32_t addr, const void *buffer, uint32_t size)
{
    status_t status = kStatus_Success;

    uint32_t offset = 0;
    uint32_t chunk = 0;
    uint32_t readBytes = 0;
    uint8_t recoveryMode = 0;

    while (size > 0) {
        /* Read on page_size_bytes boundaries (Default 2048 bytes a page) */
        offset = addr % nandConfig.page_size;
        chunk = (offset + size < nandConfig.page_size) ? size : (nandConfig.page_size - offset);
        readBytes = chunk;
        uint8_t eccsteps = nandConfig.ecc_steps;
        uint8_t *p = (uint8_t *)nandConfig.page_buf;

        bool recoveryFlag = false;

        for (int i = 0; i < SPINAND_READ_RECOVERY_MODE; i++) {

            recoveryMode += i;

            /* read page first */
            status = flexspi_nand_send_read_command(base, readInstruction,  (void *)nandConfig.page_buf, addr - offset,
                     nandConfig.page_size + nandConfig.oob_size);

            if (status != kStatus_Success)
            {
                return status;
            }

            /* copy the read out ecc code in oob area to variable pointer */
            memcpy(nandConfig.ecc_code, nandConfig.page_buf + nandConfig.page_size + nandConfig.ecc_layout_pos,
                   nandConfig.ecc_bytes * nandConfig.ecc_steps);

            p = (uint8_t *)nandConfig.page_buf;

            /* software ECC check and correct the read data */
            for (uint8_t j = 0 ; eccsteps; eccsteps--, j += nandConfig.ecc_bytes, p += nandConfig.ecc_size) {
                memset(nbc.bch->input_data, 0x0, (1 << nbc.bch->m) / 8);
                memcpy(nbc.bch->input_data + nandConfig.ecc_bytes, p, nandConfig.ecc_size);

                /* transfer the data to software ECC */
                int res = bch_decode(nbc.bch, nbc.bch->input_data, (unsigned int *)(nandConfig.ecc_code + j));

                if (res < 0) {
                    if (nandConfig.read_recovery && i < 5) {

                        status = flexspi_nand_set_feature(base, FEATURES_ADDR_SPEC, &recoveryMode);

                        if (status != kStatus_Success)
                        {
                            return status;
                        }

                        recoveryFlag = true;

                        break;
                    }

                } else if (res >= SPINAND_ECC_THRESHOLD) {
                    PRINTF("The ecc flop bits reach the threshold, please move the data to good block");
                } else if (recoveryFlag == true) {

                    recoveryFlag = false;

                    /* reset to exit the read recovery mode */
                    status = flexspi_nand_reset(base);


                    if (status != kStatus_Success)
                    {
                        return status;
                    }

                }

                /* put the corrected data to new buffer */
                memcpy(p, nbc.bch->input_data + nandConfig.ecc_bytes, nandConfig.ecc_size);
            }

            if (recoveryFlag == false) {
            	break;
            }
       }
        memcpy(buffer, nandConfig.page_buf + offset, readBytes);

        buffer = (uint8_t *)buffer + chunk;

        /* shift to next page */
        addr = (addr + SPINAND_PAGE_OFFSET) & (~SPINAND_PAGE_MASK);
        size -= chunk;
    }

    return status;
}

/**
 * @brief software ecc write data function
 *
 * @param[out] status           the return status
 * @param[in] base              FlexSPI controller base address
 * @param[in] addr              the address of write operation
 * @param[in] buffer            buffer write data
 * @param[in] size              write data size
 */
status_t spi_nand_write_software_ecc(FLEXSPI_Type *base, uint32_t addr, const void *buffer, uint32_t size)
{
    status_t status = kStatus_Success;

    uint32_t offset = 0;
    uint32_t chunk = 0;
    uint32_t writtenBytes = 0;

    while (size > 0) {
        offset = addr % nandConfig.page_size;
        chunk = (offset + size < nandConfig.page_size) ? size : (nandConfig.page_size - offset);
        writtenBytes = chunk;
        uint8_t *p = (uint8_t *)nandConfig.page_buf;
        uint8_t eccsteps = nandConfig.ecc_steps;

        /* prepare data */
        memset(nandConfig.page_buf, 0xff, nandConfig.page_size + nandConfig.oob_size);

        memcpy(nandConfig.page_buf + offset, (uint8_t *)buffer, writtenBytes);

        /* calculate the software ECC */
        for (uint8_t i = 0; eccsteps; eccsteps--, i += nandConfig.ecc_bytes, p += nandConfig.ecc_size) {
            memset(nbc.bch->input_data, 0x0, (1 << nbc.bch->m) / 8);
            memcpy(nbc.bch->input_data + nandConfig.ecc_bytes, p, nandConfig.ecc_size);
            bch_calculate_ecc(nbc.bch->input_data, nandConfig.ecc_calc + i);
        }

        /* prepare ECC code */
        memcpy(nandConfig.page_buf + nandConfig.page_size + nandConfig.ecc_layout_pos, nandConfig.ecc_calc,
               nandConfig.ecc_bytes * nandConfig.ecc_steps);

        writtenBytes = nandConfig.page_size + nandConfig.oob_size;

        /* program the data with ecc code */
        status = flexspi_nand_send_program_command(base, programInstruction, (void *)nandConfig.page_buf,
                 addr - offset, writtenBytes);

        buffer = (const uint8_t *)(buffer) + chunk;

        /* shift to next page */
        addr = (addr + SPINAND_PAGE_OFFSET) & (~SPINAND_PAGE_MASK);
        size -= chunk;
    }
}

/**
 * @brief normal write data function
 *
 * @param[out] status           the return status
 * @param[in] base              FlexSPI controller base address
 * @param[in] addr              the address of write operation
 * @param[in] buffer            buffer write data
 * @param[in] size              write data size
 */
status_t spi_nand_write_normal(FLEXSPI_Type *base, uint32_t addr, const void *buffer, uint32_t size)
{
    status_t status = kStatus_Success;

    uint32_t offset = 0;
    uint32_t chunk = 0;
    uint32_t writtenBytes = 0;

    while (size > 0) {
        /* Write on page_size_bytes boundaries (Default 2048 bytes a page) */
        offset = addr % nandConfig.page_size;
        chunk = (offset + size < nandConfig.page_size) ? size : (nandConfig.page_size - offset);
        writtenBytes = chunk;

        /* normal program page function */
        status = flexspi_nand_send_program_command(base, programInstruction, (void *)buffer, addr, writtenBytes);

        if (status != kStatus_Success)
        {
            return status;
        }

        buffer = (const uint8_t *)(buffer) + chunk;

        /* shift to next page */
        addr = (addr + SPINAND_PAGE_OFFSET) & (~SPINAND_PAGE_MASK);
        size -= chunk;
    }

    return status;
}

/**
 * @brief write data function
 *
 * @param[out] status           the return status
 * @param[in] base              FlexSPI controller base address
 * @param[in] addr              the address of write operation
 * @param[in] buffer            buffer write data
 * @param[in] size              write data size
 */
status_t spi_nand_write(FLEXSPI_Type *base, uint32_t addr, const void *buffer, uint32_t size)
{
    status_t status = kStatus_Success;

    uint32_t offset = 0;
    uint32_t chunk = 0;
    uint32_t writtenBytes = 0;

    if (nandConfig.ecc_bits == 0) {

        /* enter normal write mode */
        spi_nand_write_normal(base, addr, buffer, size);

        if (status != kStatus_Success)
        {
            return status;
        }
    } else {

        /* enter software ecc write mode */
        spi_nand_write_software_ecc(base, addr, buffer, size);

        if (status != kStatus_Success)
        {
            return status;
        }
    }

    return status;
}

/**
 * @brief erase function
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] size       erase size
 */
status_t spi_nand_erase(FLEXSPI_Type *base, uint32_t addr, uint32_t size)
{
    status_t status = kStatus_Success;

    /* align the block erase address */
    if (addr & SPINAND_BLOCK_MASK) {
        return kStatus_InvalidArgument;
    }

    if ((addr + size) > nandConfig.flash_size) {
    	return kStatus_InvalidArgument;
    }

    /* align the erase size */
    if ((size % nandConfig.block_size) != 0) {
        return kStatus_InvalidArgument;
    }

    while (size > 0)
    {
        /* block erase function */
        status = flexspi_nand_erase_block(base, addr);

        if (status != kStatus_Success)
        {
            return status;
        }

        addr += SPINAND_BLOCK_OFFSET;
        size -= nandConfig.block_size;
    }

    return status;
}

/**
 * @brief SPI NAND Flash initialization
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 */
status_t spi_nand_flash_init(FLEXSPI_Type *base)
{
    status_t status = kStatus_Success;
    uint8_t vendorId = 0;

    uint8_t configReg = 0x01;

    /* Set default read/program instructions */
    readInstruction = SPINAND_INST_READ_DEFAULT;

    programInstruction = SPINAND_INST_PROGRAM_DEFAULT;

    /* initialize the FlexSPI controller */
    flexspi_nand_flash_init(base);

    /* clear the block protection */
    status = flexspi_nand_clear_block_protection(base);

    /* read onfi table to configure the SPI-NAND driver */
    read_otp_onfi(base);

    if ((readInstruction == NAND_CMD_LUT_SEQ_IDX_READ_CACHE_114) || (readInstruction == NAND_CMD_LUT_SEQ_IDX_READ_CACHE_144) ||
        (programInstruction == NAND_CMD_LUT_SEQ_IDX_PAGE_LOAD_114))
    {
        status = flexspi_nand_set_quad_enable(base);

        if (status != kStatus_Success)
        {
            return status;
        }
    }

    /* set feature for IO strength register if needed */
    //status = flexspi_nand_set_feature(base, FEATURES_ADDR_IO_STRENGTH, &configReg);

    return status;
}

/**
 * @brief read oob area
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] addr       the address of read oob operation
 * @param[in] buffer     buffer to collect the output oob area data
 * @param[in] size       read data size
 */
status_t spi_nand_read_oob(FLEXSPI_Type *base, uint32_t addr, const void *buffer, uint32_t size)
{
    status_t status = kStatus_Success;

    /* read out the oob area data */
    status = flexspi_nand_send_read_command(base, readInstruction, buffer, addr, size);

    return status;
}

/**
 * @brief write oob area
 *
 * @param[out] status    the return status
 * @param[in] base       FlexSPI controller base address
 * @param[in] addr       the address of program oob area operation
 * @param[in] buffer     buffer for write data
 * @param[in] size       read data size
 */
status_t spi_nand_write_oob(FLEXSPI_Type *base, uint32_t addr, const void *buffer, uint32_t size)
{
    status_t status = kStatus_Success;

    /* write data into device oob area */
    status = flexspi_nand_send_program_command(base, programInstruction, (void *)buffer, addr, size);

    return status;
}

/**
 * @brief BCH initialization function
 *
 * @param[in]  ecc_bits    the Flash device support ECC bits
 */
void bch_ecc_init(uint8_t ecc_bits)
{
    unsigned int m, t, i;
    unsigned char *erased_page;
    unsigned int eccsize = 410;
    unsigned int eccbytes = 0;

    m = fls(1 + 8 * eccsize);
    t = nandConfig.ecc_bits;

    nandConfig.ecc_bytes = eccbytes = ((m * t + 31) / 32) * 4;
    nandConfig.ecc_size = eccsize;
    nandConfig.ecc_steps = nandConfig.page_size / eccsize;
    nandConfig.ecc_layout_pos = 2; // skip the bad block mark for Macronix spi nand

    nbc.bch = bch_init(m, t);
    if (!nbc.bch) {
        return;
    }

    /* verify that eccbytes has the expected value */
    if (nbc.bch->ecc_words * 4 != eccbytes) {
        return;
    }

    nandConfig.page_buf = (uint8_t *)malloc(nandConfig.page_size + nandConfig.oob_size);
    nandConfig.ecc_calc = (uint8_t *)malloc(nandConfig.ecc_steps * nandConfig.ecc_bytes);
    nandConfig.ecc_code = (uint8_t *)malloc(nandConfig.ecc_steps * nandConfig.ecc_bytes);
    nbc.eccmask = (unsigned char *)malloc(eccbytes);
    nbc.errloc = (unsigned int *)malloc(t * sizeof(*nbc.errloc));
    if (!nbc.eccmask || !nbc.errloc) {
        return;
    }
    /*
     * compute and store the inverted ecc of an erased ecc block
     */
    erased_page = (unsigned char *)malloc(eccsize);
    if (!erased_page) {
        return;
    }
    memset(nandConfig.page_buf, 0xff, nandConfig.page_size + nandConfig.oob_size);
    memset(erased_page, 0xff, eccsize);
    memset(nbc.eccmask, 0, eccbytes);

	/* encode for the input data */
    bch_encode(nbc.bch, erased_page, (unsigned int *)nbc.eccmask);
    free(erased_page);

    for (i = 0; i < eccbytes; i++) {
        nbc.eccmask[i] ^= 0xff;
    }
}

/**
 * @brief BCH calculate ecc code for the to-be-programmed data
 *
 * @param[in]  buf    pointer to the data buffer
 * @param[in]  code   pointer to the ecc code
 */
void bch_calculate_ecc(unsigned char *buf, unsigned char *code)
{
    memset(code, 0, nandConfig.ecc_bytes);

    bch_encode(nbc.bch, buf, (unsigned int *)code);
}

/**
 * @brief change read/program IO mode with different instruction
 *
 * @param[out] status           the return status
 * @param[in]  readInst         read instruction index in LUT
 * @param[in]  programInst      program instruction index in LUT
 * @param[in]  base             FlexSPI controller base address
 */
void nand_change_io_mode(FLEXSPI_Type *base, int readInst, int programInst)
{
    status_t status = kStatus_Success;

    readInstruction = readInst;
    programInstruction = programInst;

    if ((readInstruction == NAND_CMD_LUT_SEQ_IDX_READ_CACHE_114) || (readInstruction == NAND_CMD_LUT_SEQ_IDX_READ_CACHE_144) ||
        (programInstruction == NAND_CMD_LUT_SEQ_IDX_PAGE_LOAD_114))
    {
    	status = flexspi_nand_set_quad_enable(base);
    }

    return status;
}

/**
 * @brief read otp onfi table at the initialization stage
 *
 * @param[in] base   FlexSPI controller base address
 */
status_t read_otp_onfi(FLEXSPI_Type *base)
{
    status_t status = kStatus_Success;
    flexspi_transfer_t flashXfer;
    uint8_t onfiTable[SPINAND_ONFI_LENGTH];
    uint8_t securReg = 0, clearReg = 0;

    /* get feature for the secure OTP register */
    status = flexspi_nand_get_feature(base, FEATURES_ADDR_SECURE_OTP, &securReg);

    if (status != kStatus_Success)
    {
        return status;
    }

    securReg = SPINAND_SECURE_BIT_OTP_EN;

    /* set feature for the secure OTP register */
    status = flexspi_nand_set_feature(base, FEATURES_ADDR_SECURE_OTP, &securReg);

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

    if (!(securReg & SPINAND_SECURE_BIT_OTP_EN)) {
        return kStatus_Fail;
    }

    /* read onfi table page */
    status = flexspi_nand_send_read_command(base, NAND_CMD_LUT_SEQ_IDX_READ_CACHE, onfiTable,
             1<<PAGE_SHIFT_12, SPINAND_ONFI_LENGTH);

    if (onfiTable[SPINAND_ONFI_SHIFT_0] == 'O' && onfiTable[SPINAND_ONFI_SHIFT_1] == 'N'  && onfiTable[SPINAND_ONFI_SHIFT_2]
        == 'F' && onfiTable[SPINAND_ONFI_SHIFT_3] == 'I') {
    	nandConfig.page_size = onfiTable[SPINAND_ONFI_SHIFT_80] + (onfiTable[SPINAND_ONFI_SHIFT_81] << SPINAND_ONFI_SHIFT_8)
        + (onfiTable[SPINAND_ONFI_SHIFT_82] << SPINAND_ONFI_SHIFT_16);
        nandConfig.oob_size = onfiTable[SPINAND_ONFI_SHIFT_84] + (onfiTable[SPINAND_ONFI_SHIFT_85] << SPINAND_ONFI_SHIFT_8);
        nandConfig.page_num = onfiTable[SPINAND_ONFI_SHIFT_92] + (onfiTable[SPINAND_ONFI_SHIFT_93] << SPINAND_ONFI_SHIFT_8);
        nandConfig.block_num = onfiTable[SPINAND_ONFI_SHIFT_96] + (onfiTable[SPINAND_ONFI_SHIFT_97] << SPINAND_ONFI_SHIFT_8);
        nandConfig.block_size = nandConfig.page_size * nandConfig.page_num;
        switch (nandConfig.page_size) {
            case 2048 :
                nandConfig.page_shift = PAGE_SHIFT_12;
                break;
            case 4096 :
                nandConfig.page_shift = PAGE_SHIFT_13;
                break;
        }
        switch (nandConfig.page_num) {
            case 64 :
                nandConfig.block_shift = nandConfig.page_shift + SPINAND_ONFI_SHIFT_6;
                break;
            case 128 :
                nandConfig.block_shift = nandConfig.page_shift + SPINAND_ONFI_SHIFT_7;
                break;
            case 256 :
                nandConfig.block_shift = nandConfig.page_shift + SPINAND_ONFI_SHIFT_8;
                break;
        }
        nandConfig.flash_size = nandConfig.block_size * nandConfig.block_num;
        nandConfig.ecc_bits = onfiTable[SPINAND_ONFI_SHIFT_112];

    	/* configure the secure OTP register value */
        if (nandConfig.ecc_bits > 0) {
        	bch_ecc_init(nandConfig.ecc_bits);
        } else {
            securReg |= SPINAND_SECURE_BIT_ECC_EN;
        	/* set feature for the secure OTP register */
            status = flexspi_nand_set_feature(base, FEATURES_ADDR_SECURE_OTP, &securReg);

            if (status != kStatus_Success)
            {
                return status;
            }
        }

        if (onfiTable[SPINAND_ONFI_SHIFT_167] & 0x01) {
            nandConfig.read_recovery = true;
        } else {
            nandConfig.read_recovery = false;
        }

    	/* configure the continuous read global variable */
        if (onfiTable[SPINAND_ONFI_SHIFT_168] & 0x02) {
            nandConfig.continuous_read = true;
        } else {
        	nandConfig.continuous_read = false;
        }
    } else {
        PRINTF("ONFI table not found!");
        return 0;
    }

    /* clear the secure OTP register value */
    status = flexspi_nand_set_feature(base, FEATURES_ADDR_SECURE_OTP, &clearReg);

    return status;
}
