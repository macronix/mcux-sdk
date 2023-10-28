/*
 * Copyright 2018-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "lfs_mflash.h"
#include "fsl_debug_console.h"
#include "peripherals.h"

#include "spi_nand_flash.h"

#define SPI_NAND 1
#define NFTL 1

#ifdef NFTL
#include "nftl.h"
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/

struct lfs_mflash_ctx LittleFS_ctx = {LITTLEFS_START_ADDR};

/*******************************************************************************
 * Code
 ******************************************************************************/

int lfs_mflash_read(const struct lfs_config *lfsc, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
	status_t status = kStatus_Success;
    struct lfs_mflash_ctx *ctx;
    uint32_t flash_addr;

    assert(lfsc);
    ctx = (struct lfs_mflash_ctx *)lfsc->context;
    assert(ctx);

    flash_addr = ctx->start_addr + block * lfsc->block_size + off;

#if SPI_NAND
#if NFTL
    	status = nftl_flash_read(NFTL_PARTITION0, block, off, buffer, size);
#else
    	status = spi_nand_read(EXAMPLE_FLEXSPI, flash_addr / 0x800 * 0x1000, buffer, size);
#endif
#else

    if (mflash_drv_read(flash_addr, buffer, size) != kStatus_Success)
        return LFS_ERR_IO;
#endif

    return LFS_ERR_OK;
}

int lfs_mflash_prog(
    const struct lfs_config *lfsc, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
    status_t status = kStatus_Success;
    struct lfs_mflash_ctx *ctx;
    uint32_t flash_addr;

    assert(lfsc);
    ctx = (struct lfs_mflash_ctx *)lfsc->context;
    assert(ctx);

    flash_addr = ctx->start_addr + block * lfsc->block_size + off;


#if SPI_NAND
#if NFTL
    	status = nftl_flash_write(NFTL_PARTITION0, block, off, buffer, size);
#else
    	status = spi_nand_write(EXAMPLE_FLEXSPI, flash_addr / 0x800 * 0x1000, buffer, size);
#endif
#else

    assert(mflash_drv_is_page_aligned(size));

    for (uint32_t page_ofs = 0; page_ofs < size; page_ofs += MFLASH_PAGE_SIZE)
    {
        status = mflash_drv_page_program(flash_addr + page_ofs, (void *)((uintptr_t)buffer + page_ofs));
        if (status != kStatus_Success)
            break;
    }
#endif

    if (status != kStatus_Success)
        return LFS_ERR_IO;

    return LFS_ERR_OK;
}

int lfs_mflash_erase(const struct lfs_config *lfsc, lfs_block_t block)
{
    status_t status = kStatus_Success;
    struct lfs_mflash_ctx *ctx;
    uint32_t flash_addr;

    assert(lfsc);
    ctx = (struct lfs_mflash_ctx *)lfsc->context;
    assert(ctx);

    flash_addr = ctx->start_addr + block * lfsc->block_size;

#if SPI_NAND
#if NFTL
    	status = nftl_flash_erase(NFTL_PARTITION0, block);
#else
    	status = spi_nand_erase(EXAMPLE_FLEXSPI, flash_addr / 0x800 * 0x1000, lfsc->block_size);
#endif
#else

    for (uint32_t sector_ofs = 0; sector_ofs < lfsc->block_size; sector_ofs += MFLASH_SECTOR_SIZE)
    {
        status = mflash_drv_sector_erase(flash_addr + sector_ofs);
        if (status != kStatus_Success)
            break;
    }
#endif

    if (status != kStatus_Success)
        return LFS_ERR_IO;

    return LFS_ERR_OK;
}

int lfs_mflash_sync(const struct lfs_config *lfsc)
{
    return LFS_ERR_OK;
}

int lfs_get_default_config(struct lfs_config *lfsc)
{
    *lfsc = LittleFS_config; /* copy pre-initialized lfs config structure */
    return 0;
}

int lfs_storage_init(const struct lfs_config *lfsc)
{
    status_t status = LFS_ERR_OK;

#if SPI_NAND
#if NFTL
    nftl_init();
#else
    status = spi_nand_flash_init(EXAMPLE_FLEXSPI);
#endif
#else
    /* initialize mflash */
    status = mflash_drv_init();
#endif

    return status;
}
