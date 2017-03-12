#pragma once

#include <stdbool.h>

#include "elevator_io_types.h"

void fsm_onInitBetweenFloors(void);
void fsm_lightUpdating(int btn_floor, Button btn_type);
void fsm_lightUpdating_onFloorArrival( int floor );
void fsm_onRequestButtonPress(int btn_floor, Button btn_type,  bool reqIsFromServer);
void fsm_onFloorArrival(int newFloor);
void fsm_onDoorTimeout(void);
void setFSM(int elevID);