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
#ifndef LITTLEFS_SHELL_H
#define LITTLEFS_SHELL_H

#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "fsl_component_serial_manager.h"
#include "fsl_shell.h"
#include "lfs.h"
#include "lfs_mflash.h"
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/* SHELL LFS command handlers */
shell_status_t lfs_format_handler(shell_handle_t shellHandle, int32_t argc, char **argv);
shell_status_t lfs_mount_handler(shell_handle_t shellHandle, int32_t argc, char **argv);
shell_status_t lfs_unmount_handler(shell_handle_t shellHandle, int32_t argc, char **argv);
shell_status_t lfs_cd_handler(shell_handle_t shellHandle, int32_t argc, char **argv);
shell_status_t lfs_ls_handler(shell_handle_t shellHandle, int32_t argc, char **argv);
shell_status_t lfs_rm_handler(shell_handle_t shellHandle, int32_t argc, char **argv);
shell_status_t lfs_mkdir_handler(shell_handle_t shellHandle, int32_t argc, char **argv);
shell_status_t lfs_write_handler(shell_handle_t shellHandle, int32_t argc, char **argv);
shell_status_t lfs_cat_handler(shell_handle_t shellHandle, int32_t argc, char **argv);


#endif
