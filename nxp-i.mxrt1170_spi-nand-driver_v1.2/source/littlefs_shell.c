/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "littlefs_shell.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SHELL_Printf PRINTF

/*******************************************************************************
 * Variables
 ******************************************************************************/

lfs_t lfs;
struct lfs_config cfg;
int lfs_mounted;



SDK_ALIGN(uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);
shell_handle_t s_shellHandle;

extern serial_handle_t g_serialHandle;

/*******************************************************************************
 * Code
 ******************************************************************************/


shell_status_t lfs_format_handler(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    int res;

    if (lfs_mounted)
    {
        SHELL_Printf("LFS is mounted, please unmount it first.\r\n");
        return kStatus_SHELL_Success;
    }

    if (argc != 2 || strcmp(argv[1], "yes"))
    {
        SHELL_Printf("Are you sure? Please issue command \"format yes\" to proceed.\r\n");
        return kStatus_SHELL_Success;
    }

    res = lfs_format(&lfs, &cfg);
    if (res)
    {
        PRINTF("\rError formatting LFS: %d\r\n", res);
    }

    return kStatus_SHELL_Success;
}

shell_status_t lfs_mount_handler(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    int res;

    if (lfs_mounted)
    {
        SHELL_Printf("LFS already mounted\r\n");
        return kStatus_SHELL_Success;
    }

    res = lfs_mount(&lfs, &cfg);
    if (res)
    {
        PRINTF("\rError mounting LFS\r\n");
    }
    else
    {
        lfs_mounted = 1;
    }

    return kStatus_SHELL_Success;
}

shell_status_t lfs_unmount_handler(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    int res;

    if (!lfs_mounted)
    {
        SHELL_Printf("LFS not mounted\r\n");
        return kStatus_SHELL_Success;
    }

    res = lfs_unmount(&lfs);
    if (res)
    {
        PRINTF("\rError unmounting LFS: %i\r\n", res);
    }

    lfs_mounted = 0;
    return kStatus_SHELL_Success;
}

shell_status_t lfs_cd_handler(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    SHELL_Printf(
        "There is no concept of current directory in this example.\r\nPlease always specify the full path.\r\n");
    return kStatus_SHELL_Success;
}

shell_status_t lfs_ls_handler(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    int res;
    char *path;
    lfs_dir_t dir;
    struct lfs_info info;
    int files;
    int dirs;

    if (!lfs_mounted)
    {
        SHELL_Printf("LFS not mounted\r\n");
        return kStatus_SHELL_Success;
    }

    if (argc > 2)
    {
        SHELL_Printf("Invalid number of parameters\r\n");
        return kStatus_SHELL_Success;
    }

    if (argc < 2)
    {
        path = "/";
    }
    else
    {
        path = argv[1];
    }

    /* open the directory */
    res = lfs_dir_open(&lfs, &dir, path);
    if (res)
    {
        PRINTF("\rError opening directory: %i\r\n", res);
        return kStatus_SHELL_Success;
    }

    PRINTF(" Directory of %s\r\n", path);
    files = 0;
    dirs  = 0;

    /* iterate until end of directory */
    while ((res = lfs_dir_read(&lfs, &dir, &info)) != 0)
    {
        if (res < 0)
        {
            /* break the loop in case of an error */
            PRINTF("\rError reading directory: %i\r\n", res);
            break;
        }

        if (info.type == LFS_TYPE_REG)
        {
            SHELL_Printf("%8d %s\r\n", info.size, info.name);
            files++;
        }
        else if (info.type == LFS_TYPE_DIR)
        {
            SHELL_Printf("%     DIR %s\r\n", info.name);
            dirs++;
        }
        else
        {
            SHELL_Printf("%???\r\n");
        }
    }

    res = lfs_dir_close(&lfs, &dir);
    if (res)
    {
        PRINTF("\rError closing directory: %i\r\n", res);
        return kStatus_SHELL_Success;
    }

    PRINTF(" %d File(s), %d Dir(s)\r\n", files, dirs);

    return kStatus_SHELL_Success;
}

shell_status_t lfs_rm_handler(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    int res;

    if (!lfs_mounted)
    {
        SHELL_Printf("LFS not mounted\r\n");
        return kStatus_SHELL_Success;
    }

    res = lfs_remove(&lfs, argv[1]);

    if (res)
    {
        PRINTF("\rError while removing: %i\r\n", res);
    }

    return kStatus_SHELL_Success;
}

shell_status_t lfs_mkdir_handler(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    int res;

    if (!lfs_mounted)
    {
        SHELL_Printf("LFS not mounted\r\n");
        return kStatus_SHELL_Success;
    }

    res = lfs_mkdir(&lfs, argv[1]);

    if (res)
    {
        PRINTF("\rError creating directory: %i\r\n", res);
    }

    return kStatus_SHELL_Success;
}

shell_status_t lfs_write_handler(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    int res;
    lfs_file_t file;

    if (!lfs_mounted)
    {
        SHELL_Printf("LFS not mounted\r\n");
        return kStatus_SHELL_Success;
    }

    res = lfs_file_open(&lfs, &file, argv[1], LFS_O_WRONLY | LFS_O_APPEND | LFS_O_CREAT);
    if (res)
    {
        PRINTF("\rError opening file: %i\r\n", res);
        return kStatus_SHELL_Success;
    }

    res = lfs_file_write(&lfs, &file, argv[2], strlen(argv[2]));
    if (res > 0)
        res = lfs_file_write(&lfs, &file, "\r\n", 2);

    if (res < 0)
    {
        PRINTF("\rError writing file: %i\r\n", res);
    }

    res = lfs_file_close(&lfs, &file);
    if (res)
    {
        PRINTF("\rError closing file: %i\r\n", res);
    }

    return kStatus_SHELL_Success;
}

shell_status_t lfs_cat_handler(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    int res;
    lfs_file_t file;
    uint8_t buf[16];

    if (!lfs_mounted)
    {
        SHELL_Printf("LFS not mounted\r\n");
        return kStatus_SHELL_Success;
    }

    res = lfs_file_open(&lfs, &file, argv[1], LFS_O_RDONLY);
    if (res)
    {
        PRINTF("\rError opening file: %i\r\n", res);
        return kStatus_SHELL_Success;
    }

    do
    {
        res = lfs_file_read(&lfs, &file, buf, sizeof(buf));
        if (res < 0)
        {
            PRINTF("\rError reading file: %i\r\n", res);
            break;
        }
        SHELL_Write(s_shellHandle, (char *)buf, res);
    } while (res);

    res = lfs_file_close(&lfs, &file);
    if (res)
    {
        PRINTF("\rError closing file: %i\r\n", res);
    }

    return kStatus_SHELL_Success;
}
