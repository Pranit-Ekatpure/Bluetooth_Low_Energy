/* Includes ------------------------------------------------------------------*/
#include "bluenrg_gap.h"
#include "bluenrg_gatt_aci.h"
#include "bluenrg_aci_const.h"
#include "services.h"

#include <stdlib.h>

/* Globals _------------------------------------------------------------------*/
uint32_t connected = FALSE;
uint8_t set_connectable =1;
uint16_t connection_handle =0;
uint8_t  notification_enabled =FALSE;

const uint8_t service_uuid[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0xe0,0xf2,0x73,0xd9};
const uint8_t char_uuid[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0xe1,0xf2,0x73,0xd9};

uint16_t myServHandle, myCharHandle;

/* Private user code ---------------------------------------------------------*/
/**
  * Function:  Add simple service.
  * Retval:    tBleStatus
  */
tBleStatus addService(void)
{
	tBleStatus ret;

	aci_gatt_add_serv(UUID_TYPE_128,service_uuid,PRIMARY_SERVICE,7,&myServHandle);
	
	ret = aci_gatt_add_char(myServHandle,
													UUID_TYPE_128,char_uuid,
													2,
													CHAR_PROP_READ,
													ATTR_PERMISSION_NONE,
													GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
													16,
													0,
													&myCharHandle);
										 
	return ret;
}

/**
  * Function:  updateData.
  * Retval:    void
  */
void updateData(int16_t new_data)
{
	aci_gatt_update_char_value(myServHandle,myCharHandle,0,2,(uint8_t *)&new_data);
}
 
/**
 * Function:  GAP_ConnectionComplete_CB.
 * Retval:    void
 */
void GAP_ConnectionComplete_CB(uint8_t addr[6],uint16_t handle)
{
	connected = TRUE;
	connection_handle  = handle;
}
 
 /**
  * Function:  GAP_DisconnectionComplet_CB.
  * Retval:    void
  */
void GAP_DisconnectionComplet_CB(void)
{
	 //....
}
 
/**
* Function:  Read_Request_CB.
* Retval:    void
*/
void Read_Request_CB(uint16_t handle)
{
	 int data;
	 data = 450+ ((uint64_t)rand()*100)/1000;
	 updateData(data);
	 
	 if(connection_handle !=0)
	 {
		 aci_gatt_allow_read(connection_handle);
	 }
}
 
/**
* Function:  user_notify.
* Retval:    void
*/
void user_notify(void *pData)
{
	hci_uart_pckt *hci_pckt = pData;
	 
	hci_event_pckt *event_pckt = (hci_event_pckt *)hci_pckt->data;
	 
	if(hci_pckt->type != HCI_EVENT_PKT)
		 return;
	 
	switch(event_pckt->evt)
	{
		case EVT_DISCONN_COMPLETE:
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
					GAP_ConnectionComplete_CB(cc->peer_bdaddr,cc->handle);
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
				case EVT_BLUE_GATT_READ_PERMIT_REQ:
				{
					evt_gatt_read_permit_req *pr =(void *) blue_evt->data;
					Read_Request_CB(pr->attr_handle);
				}
				break;
			}
		}
		break;
	}
}
/**********************************************************************************/