#ifndef __SERVICES_H
#define __SERVICES_H

/* Includes ------------------------------------------------------------------*/
#include "bluenrg_aci_const.h"
#include "bluenrg_gap_aci.h"

/* Function Prototypes -------------------------------------------------------*/
tBleStatus addService(void);
void user_notify(void *pData);
void sendData(uint8_t * data_buffer, uint8_t no_bytes);

#endif