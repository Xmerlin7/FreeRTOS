
/*------------------------------------- Includes ---------------------------------------*/
#include "bootloader.h"

/*---------------------------- Static Functions Declerations ---------------------------*/

static void BL_print_message(uint8_t *format, ...);
static void BL_Get_Version(uint8_t *hostBuffer);
static void BL_Get_Help(uint8_t *hostBuffer);
static void BL_Get_Chip_Identification_Number(uint8_t *hostBuffer);
static void BL_Read_Protection_Level(uint8_t *hostBuffer);
static void BL_Jump_To_Address(uint8_t *hostBuffer);
static void BL_Erase_Flash(uint8_t *hostBuffer);
static void BL_Memory_Write(uint8_t *hostBuffer);
static void BL_Enable_RW_Protection(uint8_t *hostBuffer);
static void BL_Memory_Read(uint8_t *hostBuffer);
static void BL_Get_Sector_Protection_Status(uint8_t *hostBuffer);
static void BL_Read_OTP(uint8_t *hostBuffer);
static void BL_Change_Read_Protection_Level(uint8_t *hostBuffer);
static uint8_t BL_CRC_verification(uint8_t *pData, uint32_t data_len, uint32_t hostCRC);
static void BL_send_ACK(uint8_t reply_len);
static void BL_send_NACK(void);
static void BL_send_to_host(uint8_t *pData, uint8_t dataLen);
static void BL_Jump_To_User_App(void);
static uint8_t perform_FLASH_erase(uint8_t sector_number, uint8_t number_of_sectors);


/*---------------------------- Global Variables Definitions ----------------------------*/

static uint8_t BL_buffer[BL_HOST_BUFFER_RX_LENGTH];

/*---------------------------- Software Interface Definitions --------------------------*/


BL_status BL_fetch_hostCommand(void)
{
	BL_status state = BL_NACK;
	HAL_StatusTypeDef uart_state = HAL_ERROR;
	uint8_t commandLength = 0;
	uint8_t commandCode = 0;
	memset(BL_buffer,0,BL_HOST_BUFFER_RX_LENGTH);
	BL_print_message("\r\n Boot-loader started \r\n");

	uart_state = HAL_UART_Receive(BL_USART,BL_buffer, 1, HAL_MAX_DELAY);
	if(uart_state != HAL_OK)
		return state;

	commandLength = BL_buffer[0];
	uart_state = HAL_UART_Receive(BL_USART,&BL_buffer[1], commandLength, HAL_MAX_DELAY);
	if(uart_state != HAL_OK)
		return state;

	commandCode = BL_buffer[1];
	switch(commandCode)
	{
		case CBL_GET_VER_CMD:
			BL_print_message("CBL_GET_VER_VMD \r\n");
			BL_Get_Version(BL_buffer);
			state = BL_OK;
		break;
		case CBL_GET_HELP_CMD:
			BL_print_message("CBL_GET_HELP \r\n");
			BL_Get_Help(BL_buffer);
			state = BL_OK;
		break;
		case CBL_GET_CID_CMD:
			BL_print_message("ID IS 0x432 \r\n");
			BL_Get_Chip_Identification_Number(BL_buffer);
			state = BL_OK;
		break;
		case CBL_GET_RDP_STATUS_CMD:
			BL_print_message("BL_Read_Protection_Level \r\n");
			BL_Read_Protection_Level(BL_buffer);
			state = BL_OK;
		break;
		case CBL_GO_TO_ADDR_CMD:
			BL_print_message("BL_Jump_To_Address \r\n");
			BL_Jump_To_Address(BL_buffer);
			state = BL_OK;
		break;
		case CBL_FLASH_ERASE_CMD:
			BL_print_message("BL_Erase_Flash \r\n");
			BL_Erase_Flash(BL_buffer);
			state = BL_OK;
		break;
		case CBL_MEM_WRITE_CMD:
			BL_print_message("BL_Memory_Write \r\n");
			BL_Memory_Write(BL_buffer);
			state = BL_OK;
		break;
		case CBL_ED_W_PROTECT_CMD:
			BL_print_message("Enable or Disable write protect on different sectors of the user flash \r\n");
			BL_Enable_RW_Protection(BL_buffer);
			state = BL_OK;
		break;
		case CBL_MEM_READ_CMD:
			BL_print_message("Read data from different memories of the microcontroller \r\n");
			BL_Memory_Read(BL_buffer);
			state = BL_OK;
		break;
		case CBL_READ_SECTOR_STATUS_CMD:
			BL_print_message("Read all the sector protection status \r\n");
			BL_Get_Sector_Protection_Status(BL_buffer);
			state = BL_OK;
		break;
		case CBL_OTP_READ_CMD:
			BL_print_message("Read the OTP contents \r\n");
			BL_Read_OTP(BL_buffer);
			state = BL_OK;
		break;
		case CBL_CHANGE_ROP_Level_CMD:
			BL_Change_Read_Protection_Level(BL_buffer);
			state = BL_OK;
		break;
		default:
			BL_print_message("Invalid command code received from host !! \r\n");
		break;
	}

	return state;
}

/*---------------------------- Static Functions Definitions ----------------------------*/

static void BL_print_message(uint8_t *format, ...)
{
	uint8_t message[100] = {0};
	va_list args;
	va_start(args,format);
	vsprintf(message, format, args);
	HAL_UART_Transmit(BL_USART,message,sizeof(message),HAL_MAX_DELAY);
	va_end(args);
}

static uint8_t BL_CRC_verification(uint8_t *pData, uint32_t data_len, uint32_t hostCRC)
{
	uint8_t CRC_state = CRC_VERIFICATION_PASSED;
	uint32_t MCU_calculated_CRC = 0;
	uint32_t cnt = 0;
	uint32_t dataBuffer = 0;
	for(; cnt < data_len; ++cnt)
	{
		dataBuffer = (uint32_t)pData[cnt];
		MCU_calculated_CRC = HAL_CRC_Accumulate(BL_CRC, &dataBuffer, 1);
	}

	/* Reset the CRC Calculation Unit */
  __HAL_CRC_DR_RESET(BL_CRC);

	if(MCU_calculated_CRC == hostCRC)
		return CRC_state;

	CRC_state = CRC_VERIFICATION_FAILED;
	return CRC_state;
}

static void BL_send_ACK(uint8_t reply_len)
{
	uint8_t ACK[2] = {CBL_SEND_ACK, reply_len};
	HAL_UART_Transmit(BL_USART, ACK, 2, HAL_MAX_DELAY);
}

static void BL_send_NACK(void)
{
	uint8_t ACK = CBL_SEND_NACK;
	HAL_UART_Transmit(BL_USART, &ACK, 1, HAL_MAX_DELAY);
}

static void BL_send_to_host(uint8_t *pData, uint8_t dataLen)
{
	HAL_UART_Transmit(BL_USART, pData, dataLen, HAL_MAX_DELAY);
}

static void BL_Get_Version(uint8_t *hostBuffer)
{
#if DEBUG_FEATURE == DEBUG_ENABLE
	BL_print_message("BL_Get_Version\r\n");
#endif
	uint8_t BL_version[4] = {CBL_VENDOR_ID, CBL_SW_MAJOR_VERSION, CBL_SW_MINOR_VERSION, CBL_SW_PATCH_VERSION};
	uint16_t CMD_length = 0;
	uint32_t host_CRC = 0;

	/* Extract the CRC32 and packet length sent by the HOST */
	CMD_length = hostBuffer[0] + 1;
	host_CRC = *((uint32_t *)((hostBuffer + CMD_length) - CRC_TYPE_SIZE_BYTE));

	/* CRC Verification */
	if(BL_CRC_verification(hostBuffer,CMD_length - CRC_TYPE_SIZE_BYTE, host_CRC) == CRC_VERIFICATION_PASSED)
	{
#if DEBUG_FEATURE == DEBUG_ENABLE
		BL_print_message("ACK\r\n");
		BL_print_message("BL Ver: %d. %d. %d\r\n", BL_version[1], BL_version[2], BL_version[3]);
#else
		BL_send_ACK(4);
		BL_send_to_host(BL_version, 4);
#endif
	}
	else
	{
#if DEBUG_FEATURE == DEBUG_ENABLE
		BL_print_message("NACK\r\n");
#else
		BL_send_NACK();
#endif
	}
}

static void BL_Get_Help(uint8_t *hostBuffer)
{
#if DEBUG_FEATURE == DEBUG_ENABLE
	BL_print_message("BL_Get_Help\r\n");
#endif
	uint8_t Bootloader_Supported_CMDs[12] =
	{
    CBL_GET_VER_CMD,
    CBL_GET_HELP_CMD,
    CBL_GET_CID_CMD,
    CBL_GET_RDP_STATUS_CMD,
    CBL_GO_TO_ADDR_CMD,
    CBL_FLASH_ERASE_CMD,
    CBL_MEM_WRITE_CMD,
    CBL_ED_W_PROTECT_CMD,
    CBL_MEM_READ_CMD,
    CBL_READ_SECTOR_STATUS_CMD,
    CBL_OTP_READ_CMD,
    CBL_CHANGE_ROP_Level_CMD
	};
	uint16_t CMD_length = 0;
	uint32_t host_CRC = 0;

	/* Extract the CRC32 and packet length sent by the HOST */
	CMD_length = hostBuffer[0] + 1;
	host_CRC = *((uint32_t *)((hostBuffer + CMD_length) - CRC_TYPE_SIZE_BYTE));

	/* CRC Verification */
	if(BL_CRC_verification(hostBuffer,CMD_length - CRC_TYPE_SIZE_BYTE, host_CRC) == CRC_VERIFICATION_PASSED)
	{
#if DEBUG_FEATURE == DEBUG_ENABLE
		BL_print_message("ACK\r\n");
		BL_print_message("BL comm: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n",
		Bootloader_Supported_CMDs[0], Bootloader_Supported_CMDs[1],  Bootloader_Supported_CMDs[2],
		Bootloader_Supported_CMDs[3], Bootloader_Supported_CMDs[4],  Bootloader_Supported_CMDs[5],
		Bootloader_Supported_CMDs[6], Bootloader_Supported_CMDs[7],  Bootloader_Supported_CMDs[8],
		Bootloader_Supported_CMDs[9], Bootloader_Supported_CMDs[10], Bootloader_Supported_CMDs[11]);
#else
		BL_send_ACK(12);
		BL_send_to_host(Bootloader_Supported_CMDs, 12);
#endif
	}
	else
	{
#if DEBUG_FEATURE == DEBUG_ENABLE
		BL_print_message("NACK\r\n");
#else
		BL_send_NACK();
#endif
	}
}

static void BL_Get_Chip_Identification_Number(uint8_t *hostBuffer)
{
#if DEBUG_FEATURE == DEBUG_ENABLE
	BL_print_message("BL_Get_Chip_Identification_Number\r\n");
#endif
	uint16_t CMD_length = 0;
	uint32_t host_CRC = 0;
	uint16_t MCU_ID = 0;

	/* Extract the CRC32 and packet length sent by the HOST */
	CMD_length = hostBuffer[0] + 1;
	host_CRC = *((uint32_t *)((hostBuffer + CMD_length) - CRC_TYPE_SIZE_BYTE));

	/* CRC Verification */
	if(BL_CRC_verification(hostBuffer,CMD_length - CRC_TYPE_SIZE_BYTE, host_CRC) == CRC_VERIFICATION_PASSED)
	{
		/* Get the MCU chip identification number */
		MCU_ID = (uint16_t)((DBGMCU->IDCODE) & 0x00000FFF);
#if DEBUG_FEATURE == DEBUG_ENABLE
		BL_print_message("ACK\r\n");
		BL_print_message("BL MCU_ID: %d\r\n", MCU_ID);
#else
		/* Report chip identification number to HOST */
		BL_send_ACK(2);
		BL_send_to_host((uint8_t *)&MCU_ID, 2);
#endif
	}
	else
	{
#if DEBUG_FEATURE == DEBUG_ENABLE
		BL_print_message("NACK\r\n");
#else
		BL_send_NACK();
#endif
	}
}

static void BL_Jump_To_User_App(void)
{
	/*Value of the main stack pointer of main application*/
	uint32_t MSP_value = *((volatile uint32_t *)APP_START_ADDRESS);
	/*Address Reset Hander function of main application*/
	uint32_t AppAddress = *((volatile uint32_t *)(APP_START_ADDRESS + 4));

	/*Pointer to function for application resert handler*/
	MainApp ResetHandler_address = (MainApp) AppAddress;

	/*Set the value of applicaiton stack pointer*/
	__set_MSP(MSP_value);

	/*Deintialization of modules*/
	HAL_RCC_DeInit();

	/*Jump to application*/
	ResetHandler_address();
}

static uint8_t Host_Address_Verification(uint32_t Jump_Address)
{
	uint8_t Address_Verification = ADDRESS_IS_INVALID;
	if((Jump_Address >= SRAM1_BASE) && (Jump_Address <= STM32F401rct6_SRAM1_END)){
		Address_Verification = ADDRESS_IS_VALID;
	}
	else if((Jump_Address >= FLASH_BASE) && (Jump_Address <= STM32F401rct6_FLASH_END)){
		Address_Verification = ADDRESS_IS_VALID;
	}
	else{
		Address_Verification = ADDRESS_IS_INVALID;
	}
	return Address_Verification;
}

static void BL_Jump_To_Address(uint8_t *hostBuffer)
{
#if DEBUG_FEATURE == DEBUG_ENABLE
	BL_print_message("BL_Jump_To_Address\r\n");
#endif
	uint16_t CMD_length = 0;
	uint32_t host_CRC = 0;
	uint32_t jump_address = 0;
	uint8_t Address_Verification = ADDRESS_IS_INVALID;

	/* Extract the CRC32 and packet length sent by the HOST */
	CMD_length = hostBuffer[0] + 1;
	host_CRC = *((uint32_t *)((hostBuffer + CMD_length) - CRC_TYPE_SIZE_BYTE));

	/* CRC Verification */
	if(BL_CRC_verification(hostBuffer,CMD_length - CRC_TYPE_SIZE_BYTE, host_CRC) == CRC_VERIFICATION_PASSED)
	{
#if DEBUG_FEATURE == DEBUG_ENABLE
		BL_print_message("ACK\r\n");
#else
		BL_send_ACK(1);
#endif

		/* Extract the address form the HOST packet */
		jump_address = *((uint32_t *)&hostBuffer[2]);
		/* Verify the Extracted address to be valid address */
		Address_Verification = Host_Address_Verification(jump_address);

		if(Address_Verification == ADDRESS_IS_VALID)
		{
#if (DEBUG_FEATURE == DEBUG_ENABLE)
			BL_print_message("Address verification succeeded \r\n");
#else
			/* Report address verification succeeded */
			BL_send_to_host((uint8_t *)&Address_Verification, 1);
#endif

			/* Prepare the address to jump */
			JumpPtr Jump_Address_ptr = (JumpPtr)(jump_address + 1);

#if (DEBUG_FEATURE == DEBUG_ENABLE)
			BL_print_message("Jump to: 0x%X \r\n", Jump_Address_ptr);
#endif

			Jump_Address_ptr();
		}
		else
		{
#if (DEBUG_FEATURE == DEBUG_ENABLE)
			BL_print_message("Address verification succeeded \r\n");
#else
			/* Report address verification failed */
			BL_send_to_host((uint8_t *)&Address_Verification, 1);
#endif
		}
	}
	else
	{
#if DEBUG_FEATURE == DEBUG_ENABLE
		BL_print_message("NACK\r\n");
#else
		BL_send_NACK();
#endif
	}
}

static uint8_t perform_FLASH_erase(uint8_t sector_number, uint8_t number_of_sectors)
{
	uint8_t Sector_Validity_Status = INVALID_SECTOR_NUMBER;
	FLASH_EraseInitTypeDef pEraseInit = {
			.Banks = FLASH_BANK_1,
			.NbSectors = number_of_sectors,
			.Sector = sector_number,
			.TypeErase = FLASH_TYPEERASE_SECTORS,
			.VoltageRange = FLASH_VOLTAGE_RANGE_3
		};
	HAL_StatusTypeDef HAL_Status = HAL_ERROR;
	uint32_t SectorError = 0;
	if(
		 ( (sector_number >= 0) && (sector_number < STM32F401RCT6_FLASH_NUMBER_OF_SECTORS) )
	   &&(sector_number + number_of_sectors <= STM32F401RCT6_FLASH_NUMBER_OF_SECTORS)
		)
	{
		/* Unlock the FLASH control register access */
    HAL_Status = HAL_FLASH_Unlock();
		/* Perform a mass erase or erase the specified FLASH memory sectors */
		HAL_Status = HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
		if(HAL_SUCCESSFUL_ERASE == SectorError){
			Sector_Validity_Status = SUCCESSFUL_ERASE;
		}
		else{
			Sector_Validity_Status = UNSUCCESSFUL_ERASE;
		}
		/* Locks the FLASH control register access */
		HAL_Status = HAL_FLASH_Lock();
	}
	else
	{
		return Sector_Validity_Status;
	}
	return Sector_Validity_Status;
}

static void BL_Erase_Flash(uint8_t *hostBuffer)
{
#if DEBUG_FEATURE == DEBUG_ENABLE
	BL_print_message("BL_Erase_Flash\r\n");
#endif
	uint16_t CMD_length = 0;
	uint32_t host_CRC = 0;
	uint8_t erase_status = 0;
	uint8_t sector_number = hostBuffer[2];
	uint8_t number_of_sectors = hostBuffer[3];

	/* Extract the CRC32 and packet length sent by the HOST */
	CMD_length = hostBuffer[0] + 1;
	host_CRC = *((uint32_t *)((hostBuffer + CMD_length) - CRC_TYPE_SIZE_BYTE));

	/* CRC Verification */
	if((BL_CRC_verification(hostBuffer,CMD_length - CRC_TYPE_SIZE_BYTE, host_CRC) == CRC_VERIFICATION_PASSED))
	{
#if DEBUG_FEATURE == DEBUG_ENABLE
		BL_print_message("ACK\r\n");
#else
		BL_send_ACK(1);
#endif
		erase_status = perform_FLASH_erase(sector_number, number_of_sectors);
		if(erase_status == SUCCESSFUL_ERASE)
		{
#if DEBUG_FEATURE == DEBUG_ENABLE
			BL_print_message("Sector %d to sector %d was erased\r\n", sector_number, sector_number + number_of_sectors);
#else
			BL_send_to_host(&erase_status, 1);
#endif
		}
		else
		{
#if DEBUG_FEATURE == DEBUG_ENABLE
			BL_print_message("Sector %d to sector %d was not erased\r\n", sector_number, sector_number + number_of_sectors);
#else
			BL_send_to_host(&erase_status, 1);
#endif
		}
	}
	else
	{
#if DEBUG_FEATURE == DEBUG_ENABLE
		BL_print_message("NACK\r\n");
#else
		BL_send_NACK();
#endif
	}
}

static void BL_Read_Protection_Level(uint8_t *hostBuffer)
{

}
static void BL_Memory_Write(uint8_t *hostBuffer)
{

}
static void BL_Enable_RW_Protection(uint8_t *hostBuffer)
{

}
static void BL_Memory_Read(uint8_t *hostBuffer)
{

}
static void BL_Get_Sector_Protection_Status(uint8_t *hostBuffer)
{

}
static void BL_Read_OTP(uint8_t *hostBuffer)
{

}
static void BL_Change_Read_Protection_Level(uint8_t *hostBuffer)
{

}
