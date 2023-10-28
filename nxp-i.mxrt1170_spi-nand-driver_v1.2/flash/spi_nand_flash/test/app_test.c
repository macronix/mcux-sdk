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
#include "mx_test.h"

extern spi_nand_config_t nandConfig;
extern void delayUs(uint32_t delay_us);
uint8_t expected[2048] = { 0 };

int io_mode_test[IO_MODE_LENGTH] = {
    NAND_CMD_LUT_SEQ_IDX_READ_CACHE,
    NAND_CMD_LUT_SEQ_IDX_READ_CACHE_122,
    NAND_CMD_LUT_SEQ_IDX_READ_CACHE_112,
    NAND_CMD_LUT_SEQ_IDX_READ_CACHE_ALTERNATIVE,
    NAND_CMD_LUT_SEQ_IDX_READ_CACHE_114,
    NAND_CMD_LUT_SEQ_IDX_READ_CACHE_144,
};

static const char *io_mode[IO_MODE_LENGTH] = {
    "Read From Cache x1",
    "Read From Cache Dual IO 1-2-2",
    "Read From Cache x2",
    "Read From Cache x1(Alternative)",
    "Read From Cache x4",
    "Read From Cache Quad IO 1-4-4",
};

void nand_app_test()
{
    PRINTF("\n**Start app test!\r\n");
    PRINTF("--------------------------------------------------\r\n");

    nand_program_read_small_data_sizes_test();

    nand_unaligned_erase_blocks();

    nand_contiguous_erase_write_read_test();

    nand_oob_read_write_test();

    nand_unaligned_read_test();

    nand_normal_read_test();

    nand_continuous_read_test();

    PRINTF("--------------------------------------------------\r\n");
    PRINTF("**End app test!\r\n");
}

void nand_unaligned_erase_blocks()
{
    PRINTF("\r\n\t***Enter NAND Flash unaligned erase block test!***\r\n");

    uint8_t addr = 0;
    uint8_t unalignedOffset = 1;
    status_t status = kStatus_Success;

    addr += unalignedOffset;

    /* erase the block */
    status = spi_nand_erase(EXAMPLE_FLEXSPI, addr, nandConfig.block_size - unalignedOffset);

    if (status != kStatus_InvalidArgument)
    {
    	ErrorTrap();
    }

    /* erase the block */
    status = spi_nand_erase(EXAMPLE_FLEXSPI, addr, nandConfig.block_size);

    if (status != kStatus_InvalidArgument)
    {
    	ErrorTrap();
    }

    /* erase the block */
    status = spi_nand_erase(EXAMPLE_FLEXSPI, addr, unalignedOffset);

    if (status != kStatus_InvalidArgument)
    {
    	ErrorTrap();
    }

    addr = 0;

    /* erase the block */
    status = spi_nand_erase(EXAMPLE_FLEXSPI, addr, unalignedOffset);

    if (status != kStatus_InvalidArgument)
    {
        ErrorTrap();
    }

    /* erase the block */
    status = spi_nand_erase(EXAMPLE_FLEXSPI, addr, nandConfig.block_size + unalignedOffset);

    if (status != kStatus_InvalidArgument)
    {
        ErrorTrap();
    }

    /* erase the block */
    status = spi_nand_erase(EXAMPLE_FLEXSPI, addr, nandConfig.block_size);

    if (status != kStatus_Success)
    {
        ErrorTrap();
    }

    PRINTF("\r\n\t***NAND Flash unaligned erase block test successful!***\r\n");
}

void nand_oob_read_write_test()
{
    PRINTF("\r\n\t***Enter NAND Flash Read/Write OOB area test!***\r\n");

    status_t status = kStatus_Success;

    for (int i = 0; i < sizeof(expected); i++) {
        expected[i] = 0xaa;
    }

    /* erase the block */
    status = spi_nand_erase(EXAMPLE_FLEXSPI, FLASH_TEST_REGION_OFFSET, nandConfig.block_size);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    for (int i = 0; i < nandConfig.page_num; i++) {

        status = spi_nand_read(EXAMPLE_FLEXSPI, i * SPINAND_PAGE_OFFSET, nandConfig.page_buf, nandConfig.page_size);

    	CompareValue(nandConfig.page_buf, 0xFF, 0, nandConfig.page_size, 0);
    }

    /* write data into device */
    status = spi_nand_write_oob(EXAMPLE_FLEXSPI, FLASH_TEST_REGION_OFFSET, expected, nandConfig.page_size + nandConfig.oob_size);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    /* read out the data */
    status = spi_nand_read_oob(EXAMPLE_FLEXSPI, FLASH_TEST_REGION_OFFSET, nandConfig.page_buf, nandConfig.page_size + nandConfig.oob_size);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    CompareData(nandConfig.page_buf, expected, FLASH_TEST_REGION_OFFSET, nandConfig.page_size + nandConfig.oob_size, 0);

    PRINTF("\r\n\t***NAND Flash Read/Write OOB area test successful!***\r\n");
}

void nand_contiguous_erase_write_read_test()
{
    PRINTF("\r\n\t***Enter NAND Flash Contiguous Erase Write Read test!***\r\n");

    status_t status = kStatus_Success;

    for (int i = 0; i < sizeof(expected); i++) {
    	expected[i] = 0xaa;
    }

    /* erase the block */
    status = spi_nand_erase(EXAMPLE_FLEXSPI, FLASH_TEST_REGION_OFFSET, nandConfig.flash_size);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    for (int i = 0; i < nandConfig.page_num * 2; i++) {

        status = spi_nand_read(EXAMPLE_FLEXSPI, i * SPINAND_PAGE_OFFSET, nandConfig.page_buf, nandConfig.page_size);

    	CompareValue(nandConfig.page_buf, 0xFF, 0, nandConfig.page_size, 0);
    }

    for (int i = 0; i < nandConfig.page_num * 2; i++) {

    	status = spi_nand_write(EXAMPLE_FLEXSPI, FLASH_TEST_REGION_OFFSET + i * SPINAND_PAGE_OFFSET, expected, nandConfig.page_size);

        status = spi_nand_read(EXAMPLE_FLEXSPI, FLASH_TEST_REGION_OFFSET + i * SPINAND_PAGE_OFFSET, nandConfig.page_buf, nandConfig.page_size);

        if (status != kStatus_Success)
        {
            ErrorTrap();
        }

        if (memcmp(expected, nandConfig.page_buf, nandConfig.page_size) != 0)
        {
            PRINTF("\r\n\t***NAND Flash Page Read Failed!***\r\n");
            ErrorTrap();
        }

    }

    PRINTF("\r\n\t***NAND Flash  Contiguous Erase Write Read test successful!***\r\n");
}

void nand_program_read_small_data_sizes_test()
{
    PRINTF("\r\n\t***Enter NAND Flash Program Read Small Data Size test!***\r\n");

    const char writeBuffer[SMALL_DATA_LENGTH] = "1234567";
    char readBuffer[SMALL_DATA_LENGTH] = {};
    status_t status = kStatus_Success;

    for (int i = 1; i <= SMALL_DATA_LENGTH; i++) {
        /* erase the block */
    	status = spi_nand_erase(EXAMPLE_FLEXSPI, FLASH_TEST_REGION_OFFSET, nandConfig.block_size);

        if (status != kStatus_Success)
        {
            ErrorTrap();
        }

        /* write data into device */
        status = spi_nand_write(EXAMPLE_FLEXSPI, FLASH_TEST_REGION_OFFSET, writeBuffer, i);

        if (status != kStatus_Success)
        {
            ErrorTrap();
        }

        /* read out the data */
        status = spi_nand_read(EXAMPLE_FLEXSPI, FLASH_TEST_REGION_OFFSET, readBuffer, i);

        if (status != kStatus_Success)
        {
            ErrorTrap();
        }

        status = memcmp(writeBuffer, readBuffer, i);
    }

    PRINTF("\r\n\t***NAND Flash  Program Read Small Data Size  test successful!***\r\n");
}

void nand_read_write_erase_test()
{
    PRINTF("\r\n\t***Enter NAND Flash Read/Write/Erase test!***\r\n");

    status_t status = kStatus_Success;

    for (int i = 0; i < sizeof(expected); i++) {
        expected[i] = 0xaa;
    }

    /* erase the block */
    status = spi_nand_erase(EXAMPLE_FLEXSPI, FLASH_TEST_REGION_OFFSET, nandConfig.block_size);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    /* read the erased block */
    status = spi_nand_read(EXAMPLE_FLEXSPI, FLASH_TEST_REGION_OFFSET, nandConfig.page_buf, nandConfig.page_size);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    for (int i = 0; i < nandConfig.page_num; i++) {

        status = spi_nand_read(EXAMPLE_FLEXSPI, i * SPINAND_PAGE_OFFSET, nandConfig.page_buf, nandConfig.page_size);

    	CompareValue(nandConfig.page_buf, 0xFF, 0, nandConfig.page_size, 0);
    }

    /* write data into device */
    status = spi_nand_write(EXAMPLE_FLEXSPI, FLASH_TEST_REGION_OFFSET, expected, nandConfig.page_size);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    /* read out the data */
    status = spi_nand_read(EXAMPLE_FLEXSPI, FLASH_TEST_REGION_OFFSET, nandConfig.page_buf, nandConfig.page_size);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    CompareData(nandConfig.page_buf, expected, FLASH_TEST_REGION_OFFSET, nandConfig.page_size, 0);

    PRINTF("\r\n\t***NAND Flash Read/Write/Erase test successful!***\r\n");
}

void nand_unaligned_read_test()
{
    PRINTF("\r\n\t***Enter NAND Flash unaligned read test!***\r\n");

    status_t status = kStatus_Success;

    uint32_t columnOffset = 1024;

    for (int i = 0; i < sizeof(expected); i++) {
        expected[i] = 0xaa;
    }

    /* erase the block */
    status = spi_nand_erase(EXAMPLE_FLEXSPI, FLASH_TEST_REGION_OFFSET, nandConfig.block_size);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    for (int i = 0; i < nandConfig.page_num; i++) {

        status = spi_nand_read(EXAMPLE_FLEXSPI, i * SPINAND_PAGE_OFFSET, nandConfig.page_buf, nandConfig.page_size);

    	CompareValue(nandConfig.page_buf, 0xFF, 0, nandConfig.page_size, 0);
    }

    /* write data into device */
    status = spi_nand_write(EXAMPLE_FLEXSPI,  columnOffset, expected, nandConfig.page_size);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    /* read out the data */
    status = spi_nand_read(EXAMPLE_FLEXSPI,  columnOffset, nandConfig.page_buf, nandConfig.page_size);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    CompareData(nandConfig.page_buf, expected, FLASH_TEST_REGION_OFFSET, nandConfig.page_size, 0);

    PRINTF("\r\n\t***NAND Flash Unaligned Read test successful!***\r\n");
}

void nand_normal_read_test()
{
    PRINTF("\r\n### Enter NAND Flash continuous read test!***\r\n");

    nandConfig.continuous_read = false;

    for (int i = 0; i < IO_MODE_LENGTH; i++) {
        PRINTF("\r\n\t***Enter the normal read mode, the read test command is %s, the program command is Program Load x1 \r\n", io_mode[i]);

        nand_change_io_mode(EXAMPLE_FLEXSPI, io_mode_test[i], NAND_CMD_LUT_SEQ_IDX_PAGE_LOAD);

        nand_read_write_erase_test();

        delayUs(300000);
    }

    PRINTF("\r\n\t***NAND Flash normal read mode test successful!***\r\n");
}

void nand_continuous_read_test()
{
    PRINTF("\r\n\t***Enter NAND Flash continuous read test!***\r\n");

    for (int i = 0; i < IO_MODE_LENGTH; i++) {
        nand_change_io_mode(EXAMPLE_FLEXSPI, io_mode_test[i], NAND_CMD_LUT_SEQ_IDX_PAGE_LOAD_114);

        nand_read_write_erase_test();

        delayUs(300000);
    }

    PRINTF("\r\n\t***NAND Flash continuous read mode test successful!***\r\n");
}
