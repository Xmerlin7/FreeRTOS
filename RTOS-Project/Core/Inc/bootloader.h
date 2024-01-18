

#ifndef BOOTLOADER_H
#define BOOTLOADER_H

/*------------------------------------- Includes --------------------------------------*/
#include "usart.h"
#include "crc.h"
#include "gpio.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

/*--------------------------------- Macro Declerations --------------------------------*/

#define BL_USART 									 &huart2
#define BL_HOST_BUFFER_RX_LENGTH		 200
#define BL_CRC										 &hcrc

#define DEBUG_ENABLE								 0x01
#define DEBUG_DISABLE								 0x00
#define DEBUG_FEATURE							DEBUG_DISABLE


#define CBL_GET_VER_CMD              0x10
#define CBL_GET_HELP_CMD             0x11
#define CBL_GET_CID_CMD              0x12
/* Get Read Protection Status */
#define CBL_GET_RDP_STATUS_CMD       0x13
#define CBL_GO_TO_ADDR_CMD           0x14
#define CBL_FLASH_ERASE_CMD          0x15
#define CBL_MEM_WRITE_CMD            0x16
/* Enable/Disable Write Protection */
#define CBL_ED_W_PROTECT_CMD         0x17
#define CBL_MEM_READ_CMD             0x18
/* Get Sector Read/Write Protection Status */
#define CBL_READ_SECTOR_STATUS_CMD   0x19
#define CBL_OTP_READ_CMD             0x20
/* Change Read Out Protection Level */
#define CBL_CHANGE_ROP_Level_CMD     0x21

#define CBL_VENDOR_ID                100
#define CBL_SW_MAJOR_VERSION         	1
#define CBL_SW_MINOR_VERSION         	0
#define CBL_SW_PATCH_VERSION         	0

/* CRC_VERIFICATION */
#define CRC_TYPE_SIZE_BYTE           	4

#define CRC_VERIFICATION_FAILED      0x00
#define CRC_VERIFICATION_PASSED      0x01

#define CBL_SEND_NACK                0xAB
#define CBL_SEND_ACK                 0xCD

#define APP_START_ADDRESS						 0x08010000U

#define FLASH_SECTOR2_BASE_ADDRESS   0x08008000U

#define ADDRESS_IS_INVALID           0x00
#define ADDRESS_IS_VALID             0x01

#define STM32F401rct6_SRAM_SIZE      (64  * 1024)
#define STM32F401rct6_FLASH_SIZE     (256 * 1024)
#define STM32F401rct6_SRAM1_END      (SRAM1_BASE + STM32F401rct6_SRAM_SIZE)
#define STM32F401rct6_FLASH_END      (FLASH_BASE + STM32F401rct6_FLASH_SIZE)

#define STM32F401RCT6_FLASH_NUMBER_OF_SECTORS      6
#define INVALID_SECTOR_NUMBER       			   0x00
#define VALID_SECTOR_NUMBER          			   0x01
#define UNSUCCESSFUL_ERASE						   0x02
#define SUCCESSFUL_ERASE						   0x03
#define HAL_SUCCESSFUL_ERASE         			   0xFFFFFFFFU


/*---------------------------- Macro Functions Declerations ---------------------------*/

/*------------------------------- Data Type Declerations ------------------------------*/

typedef enum
{
	BL_NACK = 0,
	BL_OK
}BL_status;

typedef void (*MainApp)(void);
typedef void (*JumpPtr)(void);

/*-------------------------- Software Interface Declerations --------------------------*/
BL_status BL_fetch_hostCommand(void);




#endif /*BOOTLOADER_H*/
