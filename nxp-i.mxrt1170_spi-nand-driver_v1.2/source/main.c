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
#include "littlefs_shell.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
extern struct lfs_config cfg;
extern uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE];
extern serial_handle_t g_serialHandle;
extern shell_handle_t s_shellHandle;

#define SPI_NAND_TEST_MODE 0

/*Error trap function*/
void ErrorTrap(void)
{
    PRINTF("\n\rError occurred. Please check the configurations.\n\r");
    while (1)
    {
        ;
    }
}

SHELL_COMMAND_DEFINE(format,
                     "\r\n\"format yes\": Formats the filesystem\r\n",
                     lfs_format_handler,
                     SHELL_IGNORE_PARAMETER_COUNT);
SHELL_COMMAND_DEFINE(mount, "\r\n\"mount\": Mounts the filesystem\r\n", lfs_mount_handler, 0);
SHELL_COMMAND_DEFINE(unmount, "\r\n\"unmount\": Unmounts the filesystem\r\n", lfs_unmount_handler, 0);
SHELL_COMMAND_DEFINE(umount, "", lfs_unmount_handler, 0); // unmount alias
SHELL_COMMAND_DEFINE(cd, "", lfs_cd_handler, SHELL_IGNORE_PARAMETER_COUNT);
SHELL_COMMAND_DEFINE(ls,
                     "\r\n\"ls <path>\": Lists directory content\r\n",
                     lfs_ls_handler,
                     SHELL_IGNORE_PARAMETER_COUNT);
SHELL_COMMAND_DEFINE(dir, "", lfs_ls_handler, SHELL_IGNORE_PARAMETER_COUNT); // ls alias
SHELL_COMMAND_DEFINE(rm, "\r\n\"rm <path>\": Removes file or directory\r\n", lfs_rm_handler, 1);
SHELL_COMMAND_DEFINE(mkdir, "\r\n\"mkdir <path>\": Creates a new directory\r\n", lfs_mkdir_handler, 1);
SHELL_COMMAND_DEFINE(write, "\r\n\"write <path> <text>\": Writes/appends text to a file\r\n", lfs_write_handler, 2);
SHELL_COMMAND_DEFINE(cat, "\r\n\"cat <path>\": Prints file content\r\n", lfs_cat_handler, 1);

int main(void)
{
    status_t status;

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_SetRootClockDiv(kCLOCK_Root_Flexspi1, 2);
    CLOCK_SetRootClockMux(kCLOCK_Root_Flexspi1, 0);

#if SPI_NAND_TEST_MODE

    spi_nand_flash_init(EXAMPLE_FLEXSPI);

    nand_ops_test();

    nand_app_test();

#else
    lfs_get_default_config(&cfg);

    status = lfs_storage_init(&cfg);
    if (status != kStatus_Success)
    {
        PRINTF("LFS storage init failed: %i\r\n", status);
        return status;
    }

    s_shellHandle = &s_shellHandleBuffer[0];
    SHELL_Init(s_shellHandle, g_serialHandle, "LFS>> ");

    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(format));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(mount));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(unmount));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(umount));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(cd));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(ls));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(dir));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(rm));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(mkdir));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(write));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(cat));
#endif
    while (1)
    {
#if !(defined(SHELL_NON_BLOCKING_MODE) && (SHELL_NON_BLOCKING_MODE > 0U))
        SHELL_Task(s_shellHandle);
#endif
    }

}

