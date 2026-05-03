#ifndef SENSOR_TASK_H
#define SENSOR_TASK_H

#include "driver/i2c_master.h"

void sensor_task_start(i2c_master_bus_handle_t bus_handle);

#endif