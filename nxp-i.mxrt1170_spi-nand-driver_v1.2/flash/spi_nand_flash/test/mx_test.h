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

#ifndef APP_TEST_H
#define APP_TEST_H

#include "spi_nand_flash.h"

#define FLASH_TEST_REGION_OFFSET 0
#define SMALL_DATA_LENGTH        7
#define IO_MODE_LENGTH           6

#define CompareData( mem, mem_exp, flash_addr, compare_len, offset ) \
     PRINTF("\t\t **Start Dump data=> address %x\r\n", flash_addr); \
     PRINTF("\t\t            Length=> %d\r\n", compare_len); \
    for(int i=0 ; i < compare_len ; i=i+1 ){  \
      if( *(mem+offset+i) != *(mem_exp+offset+i) ){ \
          ErrorTrap(); \
      } \
	  if((i < 4) || (mem[offset+i] != mem_exp[offset+i])){\
		  PRINTF("\t\t  READ number=> %d\r\n", (i)); \
		  PRINTF("\t\t         data=> %02x\r\n", *(mem+offset+i)); \
		  PRINTF("\t\t     expected=> %02x\r\n", *(mem_exp+offset+i)); \
	   } \
    }

#define CompareValue( mem, value, flash_addr, compare_len, offset ) \
     PRINTF("\t\t **Start Dump data=> address %x\r\n", flash_addr); \
     PRINTF("\t\t            Length=> %d\r\n", compare_len); \
    for( i=0 ; i < compare_len ; i=i+1 ){  \
      if( mem[offset+i] != value ){ \
          ErrorTrap(); \
      } \
	  if((i < 4) || (mem[offset+i] != value)){ \
		  PRINTF("\t\t  READ number=> %d\r\n", (i)); \
		  PRINTF("\t\t         data=> %02X\r\n", *(mem+offset+i)); \
		  PRINTF("\t\t     expected=> %02X\r\n", value); \
	  } \
    }

#endif /* TEST_APP_TEST_H_ */
