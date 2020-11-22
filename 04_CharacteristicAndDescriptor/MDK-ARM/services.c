/* Includes ------------------------------------------------------------------*/
#include "bluenrg_gap.h"
#include "bluenrg_gatt_aci.h"
#include "services.h"

/* Globals _------------------------------------------------------------------*/
const uint8_t service_uuid[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,
																  0x11,0x9e,0xb1,0xe0,0xf2,0x73,0xd9};
const uint8_t char_uuid[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,
															 0x9e,0xb1,0xe1,0xf2,0x73,0xd9};
const uint8_t uuid16[2] = {0x12,0x34};

uint16_t myServHandle, myCharHandle, myDescHandle;
charactFormat charFormat;

/* Private user code ---------------------------------------------------------*/
/**
  * Function:  Add simple service.
  * Retval:    tBleStatus
  */
tBleStatus addSimpleService(void)
{
	tBleStatus ret;
	
	/* add service */
	aci_gatt_add_serv(UUID_TYPE_128,service_uuid,PRIMARY_SERVICE,7,&myServHandle);
	/* add characteristic */
	ret = aci_gatt_add_char(myServHandle,UUID_TYPE_128,char_uuid,1,CHAR_PROP_NOTIFY,
													ATTR_PERMISSION_NONE,0,16,0,&myCharHandle);
	
	charFormat.unit = FORMAT_SINT16;
	charFormat.exp = -1;
	charFormat.unit =  UNIT_TEMP_CELSIUS;
	charFormat.name_space =0;
	charFormat.desc =0;
	/* add descriptor */
	aci_gatt_add_char_desc( myServHandle,
													myCharHandle,
													UUID_TYPE_16,
													(uint8_t *)&uuid16,
													7,
													7,
													(void *)&charFormat,
													ATTR_PERMISSION_NONE,
													ATTR_ACCESS_READ_ONLY,
													0,
													16,
													FALSE,
													&myDescHandle
													);
   
	return ret;
}
/**********************************************************************************/