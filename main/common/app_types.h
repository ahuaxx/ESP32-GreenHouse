#ifndef APP_TYPES_H
#define APP_TYPES_H

#include <stdint.h>

typedef struct
{
    float temperature;
    float humidity;
    float pressure_hpa;
    float gas_resistance;
} bme680_measurement_t;

typedef struct
{
    uint16_t clear;
    uint16_t red;
    uint16_t green;
    uint16_t blue;
} tmg39931_measurement_t;

typedef struct
{
    bme680_measurement_t bme680;
    tmg39931_measurement_t light;
} sensor_measurement_t;

#endif