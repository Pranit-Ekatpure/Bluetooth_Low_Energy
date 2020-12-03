/* Includes ------------------------------------------------------------------*/
#include "services.h"
#include "bluenrg_gatt_aci.h"
#include "bluenrg_hal_aci.h"
#include "bluenrg_gap.h"
#include "stm32f4_nucleo_f401re.h"

/* Globals -------------------------------------------------------------------*/
volatile int connected = FALSE;
volatile uint8_t set_connectable = 1;
volatile uint16_t connection_handle =0;
volatile uint8_t start_read_tx_char_handle = FALSE;
volatile uint8_t start_read_rx_char_handle = FALSE;
volatile uint8_t end_read_tx_char_handle = FALSE;
volatile uint8_t end_read_rx_char_handle = FALSE;
volatile uint8_t notification_enabled = FALSE;


const uint8_t service_uuid[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0xe0,0xf2,0x73,0xd9};
const uint8_t char_uuid_tx[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0xe1,0xf2,0x73,0xd9};
const uint8_t char_uuid_rx[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0xe2,0xf2,0x73,0xd9};
 
uint16_t servHandle,charTxHandle,charRxHandle;
uint8_t rcv_data[30];
extern UART_HandleTypeDef huart2;

/* Private user code ---------------------------------------------------------*/
/**
  * Function:  Add simple service.
  * Retval:    tBleStatus
  */
tBleStatus addService(void)
{
	aci_gatt_add_serv(UUID_TYPE_128,service_uuid,PRIMARY_SERVICE,7,&servHandle);
	
	aci_gatt_add_char(servHandle,
										UUID_TYPE_128,
										char_uuid_tx,
										20,
	                  CHAR_PROP_NOTIFY,
										ATTR_PERMISSION_NONE,
										0,
										16,
										1,
										&charTxHandle);

	aci_gatt_add_char(servHandle,
										UUID_TYPE_128,
										char_uuid_rx,
										20,
										CHAR_PROP_WRITE |CHAR_PROP_WRITE_WITHOUT_RESP,
										ATTR_PERMISSION_NONE,
										GATT_NOTIFY_ATTRIBUTE_WRITE,
										16,
										1,
										&charRxHandle);
}

/**
  * Function:  Receive Data.
  * Retval:    void
  */
void receiveData(uint8_t * data_buffer, uint8_t no_bytes)
{
  BSP_LED_Toggle(LED2);
	HAL_UART_Transmit(&huart2, (uint8_t*)"\r\nReceived Data: ", sizeof("\r\nReceived Data: "), 10);

	HAL_UART_Transmit(&huart2, data_buffer, no_bytes, 10);
}

/**
  * Function:  Send Data.
  * Retval:    void
  */
void sendData(uint8_t * data_buffer, uint8_t no_bytes)
{
	aci_gatt_update_char_value(servHandle,charTxHandle,0,no_bytes,data_buffer);
}

/**
  * Function:  Attribute modified callback.
  * Retval:    void
  */
void Attribute_Modified_CB(uint16_t handle, uint8_t data_length, uint8_t * att_data)
{
	if(handle == charRxHandle+1)
	{
		receiveData(att_data,data_length);
	}
	else if(handle == charTxHandle +1)
	{
		if(att_data[0] == 0x01)
		{
			notification_enabled = TRUE;
			HAL_UART_Transmit(&huart2, (uint8_t*)"Notification Enabled :\r\n", sizeof("Notification Enabled :\r\n"), 10);
		}
	}
}
 
/**
 * Function:  GAP_ConnectionComplete_CB.
 * Retval:    void
 */
void GAP_ConnectionComplete_CB(uint8_t addr[6],uint16_t handle)
{
	connected = TRUE;
	connection_handle = handle;

	HAL_UART_Transmit(&huart2, (uint8_t*)"\r\nConnected to Device: ", sizeof("\r\nConnected to Device: "), 10);

	char bdAddrStr[30];
	snprintf(bdAddrStr, sizeof(bdAddrStr), "%02x:%02x:%02x:%02x:%02x:%02x",
																					addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
	HAL_UART_Transmit(&huart2, (uint8_t*)bdAddrStr, 17, 10);
}
 
 /**
  * Function:  GAP_DisconnectionComplet_CB.
  * Retval:    void
  */
void GAP_DisconnectionComplet_CB(void)
{
	connected = FALSE;
	HAL_UART_Transmit(&huart2, (uint8_t*)"\r\nDisconnected..", sizeof("\r\nDisconnected.."), 10);
	set_connectable = TRUE;
	
	start_read_tx_char_handle = FALSE;
	start_read_rx_char_handle = FALSE;
	end_read_tx_char_handle =FALSE;
	end_read_rx_char_handle = FALSE;
}
 
/**
* Function:  user_notify.
* Retval:    void
*/
void user_notify(void *pData)
{
	hci_uart_pckt *hci_pckt = pData;
	hci_event_pckt *event_pckt = (hci_event_pckt *)hci_pckt->data;
	
	if(hci_pckt->type !=HCI_EVENT_PKT)
		return;
	switch(event_pckt->evt)
	{
		case EVT_DISCONN_COMPLETE_BIT:
		{
			GAP_DisconnectionComplet_CB();
		}
		break;
		case EVT_LE_META_EVENT:
		{
			 evt_le_meta_event *evt = (void *)event_pckt->data;
			switch(evt->subevent)
			{
				case EVT_LE_CONN_COMPLETE:
				{
					evt_le_connection_complete *cc = (void *)evt->data;
					GAP_ConnectionComplete_CB(cc->peer_bdaddr, cc->handle);
				}
				break;
			}
		}
		break;
		case EVT_VENDOR:
		{
			evt_blue_aci *blue_evt = (void *)event_pckt->data;
			switch(blue_evt->ecode)
			{
				case EVT_BLUE_GATT_ATTRIBUTE_MODIFIED:
				{
					evt_gatt_attr_modified_IDB05A1 *evt = (evt_gatt_attr_modified_IDB05A1*) blue_evt->data;
					Attribute_Modified_CB(evt->attr_handle,evt->data_length,evt->att_data);
				}
			}
			break;
		}
	  break;
	}
}
/**********************************************************************************/