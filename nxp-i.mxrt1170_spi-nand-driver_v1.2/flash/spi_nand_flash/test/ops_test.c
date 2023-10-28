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

extern const uint32_t seqLUT[SEQ_LUT_LENGTH];
extern const uint32_t customLUT[CUSTOM_LUT_LENGTH];
extern spi_nand_config_t nandConfig;
extern void delayUs(uint32_t delay_us);
uint8_t writeBuffer[2048] = { 0 };

extern ErrorTrap(void);

void nand_ops_test()
{
    PRINTF("\n**Start ops test!\r\n");
    PRINTF("--------------------------------------------------\r\n");

    nand_read_id_test();
    delayUs(300000);

    nand_get_set_feature_test();
    delayUs(300000);

    nand_read_status_test();
    delayUs(300000);

    nand_reset_test();
    delayUs(300000);

    nand_dp_test();
    delayUs(300000);

    nand_cache_read_sequential_random_test();
    delayUs(300000);

    PRINTF("--------------------------------------------------\r\n");
    PRINTF("**End ops test!\r\n");
}

void nand_read_id_test()
{
    PRINTF("\r\n\t***Enter NAND Flash Read ID test!***\r\n");

    status_t status = kStatus_Success;

    uint8_t vendorId = 0;
    uint8_t FlashID = SPINAND_VENDOR_ID;

    /* read id */
    status = flexspi_nand_read_id(EXAMPLE_FLEXSPI, &vendorId);

	PRINTF("\t\t    Read ID=> %02X\r\n", vendorId);
	PRINTF("\t\t   Expected=> %02X\r\n", FlashID);

    if (vendorId != FlashID){
        PRINTF("\r\n\t***Nand Flash Read ID test Failed, %x!***\r\n", vendorId);
        ErrorTrap();
    } else {
        PRINTF("\r\n\t***Nand Flash Read ID test successful! the vendor ID is 0x%x***\r\n", vendorId);
    }
}

void nand_get_set_feature_test()
{
    PRINTF("\r\n\t***Enter NAND Flash get/set feature test!***\r\n");

    status_t status = kStatus_Success;

    uint8_t stReg1, stReg2;

    /* get feature for bp register bits */
    status = flexspi_nand_get_feature(EXAMPLE_FLEXSPI, FEATURES_ADDR_BLOCK_PROTECTION, &stReg1);

    PRINTF("\t\tGet Feature block protection output=>%02X\r\n", stReg1);

    if (stReg1 & SPINAND_BLOCK_PROT_BIT_BP_MASK){

    	stReg2 = stReg1 & ~SPINAND_BLOCK_PROT_BIT_MASK;

        /* set feature for bp register bits */
        status = flexspi_nand_set_feature(EXAMPLE_FLEXSPI, FEATURES_ADDR_BLOCK_PROTECTION, &stReg2);

        PRINTF("\r\n\t***the set feature value is 0x%x***\r\n", stReg2);

        if (status != kStatus_Success)
        {
            ErrorTrap();
        }

        /* get feature for bp register bits */
        status = flexspi_nand_get_feature(EXAMPLE_FLEXSPI, FEATURES_ADDR_BLOCK_PROTECTION, &stReg2);

        PRINTF("\t\t block un-protection!\r\n");

        PRINTF("\t\tSet Feature output=>%02X\r\n", stReg2);

        if (status != kStatus_Success)
        {
            ErrorTrap();
        }

        /* compare the read out value */
        if (stReg2 != (stReg1 & ~SPINAND_BLOCK_PROT_BIT_MASK)){
            PRINTF("\r\n\t***Nand Flash get/set feature test Failed!***\r\n");
            ErrorTrap();
        }
    } else {
    	stReg2 = stReg1 | SPINAND_BLOCK_PROT_BIT_BP_MASK;

        /* set feature for bp register bits */
        status = flexspi_nand_set_feature(EXAMPLE_FLEXSPI, FEATURES_ADDR_BLOCK_PROTECTION, &stReg2);

        PRINTF("\r\n\t***the set feature value is 0x%x***\r\n", stReg2);

        if (status != kStatus_Success)
        {
            ErrorTrap();
        }

        /* get feature for bp register bits */
        status = flexspi_nand_get_feature(EXAMPLE_FLEXSPI, FEATURES_ADDR_BLOCK_PROTECTION, &stReg2);

        PRINTF("\t\t block protection!\r\n");

        PRINTF("\t\tSet Feature output=>%02X\r\n", stReg2);

        if (status != kStatus_Success)
        {
            ErrorTrap();
        }

        /* compare the read status value */
        if (stReg2 != (stReg1 | SPINAND_BLOCK_PROT_BIT_BP_MASK)){
            PRINTF("\r\n\t***Nand Flash get/set feature test Failed!***\r\n");
            ErrorTrap();
        }
    }

    status = flexspi_nand_set_feature(EXAMPLE_FLEXSPI, FEATURES_ADDR_BLOCK_PROTECTION, &stReg1);

    PRINTF("\r\n\t***Nand Flash get/set feature test successful!***\r\n");
}

void nand_read_status_test()
{
    PRINTF("\r\n\t***Enter NAND Flash Read Status test!***\r\n");

    status_t status = kStatus_Success;
    uint8_t stReg1, stReg2;

    /* write disable by sending command */
    status = flexspi_nand_write_disable(EXAMPLE_FLEXSPI);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    /* write enable by sending command */
    status = flexspi_nand_write_enable(EXAMPLE_FLEXSPI);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    /* read status by sending command  */
    status = flexspi_nand_read_status(EXAMPLE_FLEXSPI, &stReg1);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    /* write disable by sending command */
    status = flexspi_nand_write_disable(EXAMPLE_FLEXSPI);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    /* compare the read status value */
    if ((stReg1 & SPINAND_STATUS_BIT_WEL) != SPINAND_STATUS_BIT_WEL){
        PRINTF("\r\n\t***Nand Flash Read status test Failed!, %x***\r\n", stReg1);
        ErrorTrap();
    }

    PRINTF("\t\tRead Status output=>%02X\r\n", stReg1 & SPINAND_STATUS_BIT_WEL);

    PRINTF("\t\t   expected=>%02X\r\n", SPINAND_STATUS_BIT_WEL);

    PRINTF("\r\n\t***Nand Flash Read status test successful!***\r\n");
}

void nand_write_disable_test()
{
    PRINTF("\r\n\t***Enter NAND Flash write disable test!***\r\n");

    status_t status = kStatus_Success;
    uint8_t stReg1, stReg2;

    /* write enable by sending command */
    status = flexspi_nand_write_enable(EXAMPLE_FLEXSPI);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    /* write disable by sending command */
    status = flexspi_nand_write_disable(EXAMPLE_FLEXSPI);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    /* read status by get feature  */
    status = flexspi_nand_get_feature(EXAMPLE_FLEXSPI, FEATURES_ADDR_STATUS, &stReg1);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    PRINTF("\r\n\t***the get feature value is 0x%x***\r\n", stReg1);

    /* compare the read status value */
    if ((stReg1 & SPINAND_STATUS_BIT_WEL) != SPINAND_STATUS_BIT_WEL){
        PRINTF("\r\n\t***Nand Flash write disable test Failed!***\r\n");
        ErrorTrap();
    }

    PRINTF("\t\tWRDI output=>%02X\r\n", stReg1 & SPINAND_STATUS_BIT_WEL);
    PRINTF("\t\t   expected=>%02X\r\n", 0x00);

    PRINTF("\r\n\t***Nand Flash write disable test successful!***\r\n");
}

void nand_write_enable_test()
{
    PRINTF("\r\n\t***Enter NAND Flash write enable test!***\r\n");

    status_t status = kStatus_Success;
    uint8_t stReg1, stReg2;

    /* write disable by sending command */
    status = flexspi_nand_write_disable(EXAMPLE_FLEXSPI);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    /* write enable by sending command */
    status = flexspi_nand_write_enable(EXAMPLE_FLEXSPI);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    /* read status by get feature  */
    status = flexspi_nand_get_feature(EXAMPLE_FLEXSPI, FEATURES_ADDR_STATUS, &stReg1);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    PRINTF("\r\n\t***the get feature value is 0x%x***\r\n", stReg1);

    /* write disable by sending command */
    status = flexspi_nand_write_disable(EXAMPLE_FLEXSPI);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    /* compare the read status value */
    if ((stReg1 & SPINAND_STATUS_BIT_WEL) != SPINAND_STATUS_BIT_WEL){
        PRINTF("\r\n\t***Nand Flash Write enable test Failed!***\r\n");
        ErrorTrap();
    }

    PRINTF("\t\tWREN output=>%02X\r\n", stReg1 & SPINAND_STATUS_BIT_WEL);

    PRINTF("\t\t   expected=>%02X\r\n", SPINAND_STATUS_BIT_WEL);

    PRINTF("\r\n\t***Nand Flash Write enable test successful!***\r\n");
}

void nand_reset_test()
{
    PRINTF("\r\n\t***Enter NAND Flash reset test!***\r\n");

    status_t status = kStatus_Success;
    uint8_t stReg1, stReg2;

    /* write enable by sending command */
    status = flexspi_nand_write_enable(EXAMPLE_FLEXSPI);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    /* get feature for status register */
    status = flexspi_nand_get_feature(EXAMPLE_FLEXSPI, FEATURES_ADDR_STATUS, &stReg1);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    PRINTF("\r\n\t***the get feature value is 0x%x***\r\n", stReg1);

    /* reset the device */
    status = flexspi_nand_reset(EXAMPLE_FLEXSPI);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    /* get feature for status register */
    status = flexspi_nand_get_feature(EXAMPLE_FLEXSPI, FEATURES_ADDR_STATUS, &stReg2);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    PRINTF("\r\n\t***the get feature value is 0x%x***\r\n", stReg1);

    /* compare the read out status register */
    if ((stReg2 & SPINAND_STATUS_BIT_WEL) != 0){
        PRINTF("\r\n\t***Nand Flash reset test Failed!***\r\n");
        ErrorTrap();
    }

    PRINTF("\t\tWREN output=>%02X\r\n", stReg1 & SPINAND_STATUS_BIT_WEL);

    PRINTF("\t\t   expected=>%02X\r\n", SPINAND_STATUS_BIT_WEL);

    PRINTF("\r\n\t***Nand Flash reset test successful!***\r\n");
}

void nand_dp_test()
{
    PRINTF("\r\n\t***Enter NAND Flash deep power down mode test!***\r\n");

    status_t status = kStatus_Success;
    uint8_t stReg1, stReg2;
    uint8_t vendorId = 0;

    /* enter deep power down mode */
    status = flexspi_nand_deep_down(EXAMPLE_FLEXSPI);

    /* read id */
    status = flexspi_nand_read_id(EXAMPLE_FLEXSPI, vendorId);

	PRINTF("\t\t    Read ID=> %02X\r\n", vendorId);
	PRINTF("\t\t   Expected=> %02X\r\n", 0);

    if (vendorId != 0){
        PRINTF("\r\n\t***Enter Deep Power Down Mode Failed!***\r\n");
        ErrorTrap();
    }

    PRINTF("\r\n\t***Nand Flash Deep Power Down test successful!***\r\n");
}

void nand_cache_read_sequential_random_test()
{
    PRINTF("\r\n\t***Enter NAND Flash Cache Read Sequential/Random test!***\r\n");

    status_t status = kStatus_Success;
    uint8_t stReg1, stReg2;
    uint8_t writeBuf[2048] = { 0 };

    for (int i = 0; i < nandConfig.page_size; i++) {
        writeBuffer[i] = 0xbb;
        writeBuf [i] = 0xaa;
    }

    status = spi_nand_erase(EXAMPLE_FLEXSPI, FLASH_TEST_REGION_OFFSET, nandConfig.block_size);

    for (int i = 0; i < nandConfig.page_num; i++) {

        status = spi_nand_read(EXAMPLE_FLEXSPI, i * SPINAND_PAGE_OFFSET, nandConfig.page_buf, nandConfig.page_size);

    	CompareValue(nandConfig.page_buf, 0xFF, 0, nandConfig.page_size, 0);
    }

    memset(writeBuffer, 0xaa, sizeof(writeBuffer));

    status = spi_nand_write(EXAMPLE_FLEXSPI, FLASH_TEST_REGION_OFFSET, writeBuffer, nandConfig.page_size);

    status = spi_nand_write(EXAMPLE_FLEXSPI, FLASH_TEST_REGION_OFFSET + SPINAND_PAGE_OFFSET, writeBuf, nandConfig.page_size);

    /* send page read command */
    status = flexspi_nand_send_general_command(EXAMPLE_FLEXSPI, NAND_CMD_LUT_SEQ_IDX_PAGE_READ,
             FLASH_TEST_REGION_OFFSET >> PAGE_SHIFT_12, NULL, 0, NULL, 0);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    /* check the status register to wait bus busy */
    status = flexspi_nand_wait_bus_busy(EXAMPLE_FLEXSPI);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(EXAMPLE_FLEXSPI, NAND_CMD_LUT_FOR_IP_CMD, &seqLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_CACHE_SEQUENTIAL],
                      LUT_LENGTH);

//    status = flexspi_nand_send_general_command(EXAMPLE_FLEXSPI, NAND_CMD_LUT_SEQ_IDX_READ_CACHE_RANDOM,
//               FLASH_TEST_REGION_OFFSET + SPINAND_PAGE_OFFSET, NULL, 0, NULL, 0);

    status = flexspi_nand_send_general_command(EXAMPLE_FLEXSPI, NAND_CMD_LUT_FOR_IP_CMD, 0, NULL, 0, NULL, 0);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(EXAMPLE_FLEXSPI, NAND_CMD_LUT_FOR_IP_CMD, &customLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_FOR_IP_CMD], LUT_LENGTH);

    /* check the status register to wait bus busy */
    status = flexspi_nand_wait_bus_busy(EXAMPLE_FLEXSPI);

    if (status != kStatus_Success)
    {
    	ErrorTrap();
    }

    status = flexspi_nand_send_general_command(EXAMPLE_FLEXSPI, NAND_CMD_LUT_SEQ_IDX_READ_CACHE, FLASH_TEST_REGION_OFFSET,
             NULL, 0, nandConfig.page_buf, nandConfig.page_size);

    CompareData(nandConfig.page_buf, writeBuffer, FLASH_TEST_REGION_OFFSET, nandConfig.page_size, 0);

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(EXAMPLE_FLEXSPI, NAND_CMD_LUT_FOR_IP_CMD, &seqLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_SEQ_IDX_READ_SEQUENTIAL_END],
                      LUT_LENGTH);

    status = flexspi_nand_send_general_command(EXAMPLE_FLEXSPI, NAND_CMD_LUT_FOR_IP_CMD, FLASH_TEST_REGION_OFFSET, NULL, 0, NULL, 0);

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(EXAMPLE_FLEXSPI, NAND_CMD_LUT_FOR_IP_CMD, &customLUT[LUT_INDEX_LENGTH * NAND_CMD_LUT_FOR_IP_CMD], LUT_LENGTH);

    /* check the status register to wait bus busy */
    status = flexspi_nand_wait_bus_busy(EXAMPLE_FLEXSPI);

    status = flexspi_nand_send_general_command(EXAMPLE_FLEXSPI, NAND_CMD_LUT_SEQ_IDX_READ_CACHE, FLASH_TEST_REGION_OFFSET,
             NULL, 0, nandConfig.page_buf, nandConfig.page_size);

    CompareData(nandConfig.page_buf, writeBuf, FLASH_TEST_REGION_OFFSET, nandConfig.page_size, 0);

    PRINTF("\r\n\t***Nand Flash Cache Read Sequential/Random test successful!***\r\n");
}
