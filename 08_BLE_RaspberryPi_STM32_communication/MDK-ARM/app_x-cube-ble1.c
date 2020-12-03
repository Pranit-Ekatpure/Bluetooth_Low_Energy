/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

#include "app_x-cube-ble1.h"
#include "bluenrg_gap.h"
#include "hci_const.h"
#include "bluenrg_hal_aci.h"
#include "hci.h"
#include "hci_le.h"
#include "bluenrg_gatt_aci.h"
#include "bluenrg_gap_aci.h"
#include "bluenrg_utils.h"
#include "services.h"
#include "stm32f4_nucleo_f401re.h"

/* Defines -------------------------------------------------------------------*/
#define BDADDR_SIZE 6

/* Globals -------------------------------------------------------------------*/
extern volatile uint8_t set_connectable;
extern volatile uint8_t notification_enabled ;
extern volatile int connected;
uint8_t btn_init_state;

/* Function Prototypes -------------------------------------------------------*/
static void userIOInit(void);
static void userChat(void);
void Make_Connection(void);

/* Private user code ---------------------------------------------------------*/
/**
  * Function:  MX_BlueNRG_MS_Init.
  * Retval:    void
  */
void MX_BlueNRG_MS_Init(void)
{
	const char *name = "ST_BLE";
	uint8_t SERVER_BDADDR[] ={0x01,0x02,0x03,0x04,0x05,0x06};
	uint8_t bdaddr[BDADDR_SIZE];
	uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
	
	userIOInit();
	hci_init(user_notify,NULL);
	hci_reset();
	HAL_Delay(100);
	BLUENRG_memcpy(bdaddr,SERVER_BDADDR,sizeof(SERVER_BDADDR));
	
	aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET,CONFIG_DATA_PUBADDR_LEN,bdaddr);
	
	aci_gatt_init();
  aci_gap_init_IDB05A1(GAP_PERIPHERAL_ROLE_IDB05A1, 0, 0x07, &service_handle, &dev_name_char_handle, &appearance_char_handle);
	
	aci_gatt_update_char_value(service_handle,dev_name_char_handle,0,strlen(name),(uint8_t *)name); 
																 
	/* Add Service */
	addService();
}
extern UART_HandleTypeDef huart2;
/**
  * Function:  MX_BlueNRG_MS_Process.
  * Retval:    void
  */
void MX_BlueNRG_MS_Process(void)
{
	userChat();
	hci_user_evt_proc();
}

/**
  * Function:  User I/O init.
  * Retval:    void
  */
static void userIOInit(void)
{
	BSP_PB_Init(BUTTON_KEY,BUTTON_MODE_GPIO);
	BSP_LED_Init(LED2);
  BSP_COM_Init(COM1);
}

/**
  * Function:  User Chat function.
  * Retval:    void
  */
static void userChat(void)
{
	if(set_connectable)
	{
		Make_Connection();
		set_connectable = FALSE;
		btn_init_state = BSP_PB_GetState(BUTTON_KEY);
	}
	
  if(BSP_PB_GetState(BUTTON_KEY) == !btn_init_state)
	{
		while(BSP_PB_GetState(BUTTON_KEY) == !btn_init_state);
		if(connected)// && notification_enabled)
		{
			uint8_t data[] =  "Hi, Raspberry Pi.";
			sendData(data,sizeof(data));
			HAL_UART_Transmit(&huart2, (uint8_t*)"\r\nTransmitted Data: ", sizeof("\r\nTransmitted Data: "), 10);
			HAL_UART_Transmit(&huart2, data, sizeof(data), 10);
		}
	}		
}

/**
  * Function:  Make connection.
  * Retval:    void
  */
void Make_Connection(void)
{
	tBleStatus ret;
	const char local_name[] = {AD_TYPE_COMPLETE_LOCAL_NAME, 'B','L','E','_','T','x','R','x'};
	hci_le_set_scan_resp_data(0,NULL);
	aci_gap_set_discoverable(ADV_IND, 0,0,PUBLIC_ADDR,NO_WHITE_LIST_USE,sizeof(local_name),local_name,0,NULL,0,0);
}
