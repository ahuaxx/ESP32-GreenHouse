#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    float temperature_c;
    float humidity_pct;
    float pressure_hpa;
    float gas_kohms;
    uint16_t light_lux;

    bool bme680_ok;
    bool light_ok;
} sensor_data_t;

#endif