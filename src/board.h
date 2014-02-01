#pragma once

// for roundf()
#define __USE_C99_MATH

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include "stm32f4xx_conf.h"
#include "core_cm4.h"

#ifndef __CC_ARM
// only need this garbage on gcc
#define USE_LAME_PRINTF
#include "printf.h"
#endif

#include "drv_system.h"         // timers, delays, etc
#include "drv_gpio.h"

#ifndef M_PI
#define M_PI       3.14159265358979323846f
#endif /* M_PI */

#define RADX10 (M_PI / 1800.0f)                  // 0.001745329252f
#define RAD    (M_PI / 180.0f)

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define abs(x) ((x) > 0 ? (x) : -(x))

// Chip Unique ID on F4xx
#define U_ID_0 (*(uint32_t*)0x1FFF7A10)
#define U_ID_1 (*(uint32_t*)0x1FFF7A14)
#define U_ID_2 (*(uint32_t*)0x1FFF7A18)

typedef enum {
    SENSOR_GYRO = 1 << 0, // always present
    SENSOR_ACC = 1 << 1,
    SENSOR_BARO = 1 << 2,
    SENSOR_MAG = 1 << 3,
    SENSOR_SONAR = 1 << 4,
    SENSOR_GPS = 1 << 5,
    SENSOR_GPSMAG = 1 << 6,
} AvailableSensors;

// Type of accelerometer used/detected
typedef enum AccelSensors {
    ACC_DEFAULT = 0,
    ACC_ADXL345 = 1,
    ACC_MPU6050 = 2,
    ACC_MMA8452 = 3,
    ACC_BMA280 = 4,
    ACC_NONE = 5
} AccelSensors;

typedef enum {
    FEATURE_PPM = 1 << 0,
    FEATURE_VBAT = 1 << 1,
    FEATURE_INFLIGHT_ACC_CAL = 1 << 2,
    FEATURE_SERIALRX = 1 << 3,
    FEATURE_MOTOR_STOP = 1 << 4,
    FEATURE_SERVO_TILT = 1 << 5,
    FEATURE_GYRO_SMOOTHING = 1 << 6,
    FEATURE_LED_RING = 1 << 7,
    FEATURE_GPS = 1 << 8,
    FEATURE_FAILSAFE = 1 << 9,
    FEATURE_SONAR = 1 << 10,
    FEATURE_TELEMETRY = 1 << 11,
    FEATURE_POWERMETER = 1 << 12,
    FEATURE_VARIO = 1 << 13,
    FEATURE_3D = 1 << 14,
    FEATURE_SOFTSERIAL = 1 << 15,
} AvailableFeatures;

typedef enum {
    SERIALRX_SPEKTRUM1024 = 0,
    SERIALRX_SPEKTRUM2048 = 1,
    SERIALRX_SBUS = 2,
} SerialRXType;

typedef enum {
    GPS_NMEA = 0,
    GPS_UBLOX,
    GPS_MTK_NMEA,
    GPS_MTK_BINARY,
    GPS_MAG_BINARY,
} GPSHardware;

typedef enum {
    TELEMETRY_UART = 0,
    TELEMETRY_SOFTSERIAL,
} TelemetrySerial;

typedef enum {
    X = 0,
    Y,
    Z
} sensor_axis_e;

typedef enum {
    ALIGN_DEFAULT = 0,                                      // driver-provided alignment
    CW0_DEG = 1,
    CW90_DEG = 2,
    CW180_DEG = 3,
    CW270_DEG = 4,
    CW0_DEG_FLIP = 5,
    CW90_DEG_FLIP = 6,
    CW180_DEG_FLIP = 7,
    CW270_DEG_FLIP = 8
} sensor_align_e;

enum {
    GYRO_UPDATED = 1 << 0,
    ACC_UPDATED = 1 << 1,
    MAG_UPDATED = 1 << 2,
    TEMP_UPDATED = 1 << 3
};

typedef struct sensor_data_t
{
    int16_t gyro[3];
    int16_t acc[3];
    int16_t mag[3];
    float temperature;
    int updated;
} sensor_data_t;

typedef void (* sensorInitFuncPtr)(sensor_align_e align);   // sensor init prototype
typedef void (* sensorReadFuncPtr)(int16_t *data);          // sensor read and align prototype
typedef void (* baroOpFuncPtr)(void);                       // baro start operation
typedef void (* baroCalculateFuncPtr)(int32_t *pressure, int32_t *temperature);             // baro calculation (filled params are pressure and temperature)
typedef void (* serialReceiveCallbackPtr)(uint16_t data);   // used by serial drivers to return frames to app
typedef uint16_t (* rcReadRawDataPtr)(uint8_t chan);        // used by receiver driver to return channel data
typedef void (* pidControllerFuncPtr)(void);                // pid controller function prototype

typedef struct sensor_t
{
    sensorInitFuncPtr init;                                 // initialize function
    sensorReadFuncPtr read;                                 // read 3 axis data function
    sensorReadFuncPtr temperature;                          // read temperature if available
    float scale;                                            // scalefactor (currently used for gyro only, todo for accel)
} sensor_t;

typedef struct baro_t
{
    uint16_t ut_delay;
    uint16_t up_delay;
    baroOpFuncPtr start_ut;
    baroOpFuncPtr get_ut;
    baroOpFuncPtr start_up;
    baroOpFuncPtr get_up;
    baroCalculateFuncPtr calculate;
} baro_t;

// Hardware definitions and GPIO
#ifdef FY90Q
 // FY90Q
#define LED0_GPIO   GPIOC
#define LED0_PIN    GPIO_Pin_12
#define LED1_GPIO   GPIOA
#define LED1_PIN    GPIO_Pin_15

#define GYRO
#define ACC

#define SENSORS_SET (SENSOR_ACC)

#else

#ifdef OLIMEXINO
// OLIMEXINO

// LED2 is using one of the pwm pins (PWM2), so we must not use PWM2.  @See pwmInit()
#define LED0_GPIO   GPIOA
#define LED0_PIN    GPIO_Pin_1 // D3, PA1/USART2_RTS/ADC1/TIM2_CH3 - "LED2" on silkscreen, Yellow
#define LED1_GPIO   GPIOA
#define LED1_PIN    GPIO_Pin_5 // D13, PA5/SPI1_SCK/ADC5 - "LED1" on silkscreen, Green

#define GYRO
#define ACC

#define SENSORS_SET (SENSOR_ACC)

#else
// Afroflight32
#define LED0_GPIO   GPIOD
#define LED0_PIN    GPIO_Pin_12
#define LED1_GPIO   GPIOD
#define LED1_PIN    GPIO_Pin_13
#define LED2_GPIO   GPIOD
#define LED2_PIN    GPIO_Pin_14
#define LED3_GPIO   GPIOD
#define LED3_PIN    GPIO_Pin_15
#define BEEP_GPIO   GPIOD
#define BEEP_PIN    GPIO_Pin_11
#define BARO_GPIO   GPIOC
#define BARO_PIN    GPIO_Pin_13

#define GYRO
#define ACC
#define MAG
#define BARO
#define LEDRING
#define SONAR
#define BUZZER

#define SENSORS_SET (SENSOR_ACC | SENSOR_BARO | SENSOR_MAG)

#endif
#endif

// Helpful macros
#define LED0_TOGGLE              digitalToggle(LED0_GPIO, LED0_PIN);
#define LED0_ON                 digitalHi(LED0_GPIO, LED0_PIN);
#define LED0_OFF                  digitalLo(LED0_GPIO, LED0_PIN);

#define LED1_TOGGLE              digitalToggle(LED1_GPIO, LED1_PIN);
#define LED1_ON                 digitalHi(LED1_GPIO, LED1_PIN);
#define LED1_OFF                  digitalLo(LED1_GPIO, LED1_PIN);

#define LED2_TOGGLE              digitalToggle(LED2_GPIO, LED2_PIN);
#define LED2_ON                 digitalHi(LED2_GPIO, LED2_PIN);
#define LED2_OFF                  digitalLo(LED2_GPIO, LED2_PIN);

#define LED3_TOGGLE              digitalToggle(LED3_GPIO, LED3_PIN);
#define LED3_ON                 digitalHi(LED3_GPIO, LED3_PIN);
#define LED3_OFF                  digitalLo(LED3_GPIO, LED3_PIN);

#ifdef BEEP_GPIO
#define BEEP_TOGGLE              digitalToggle(BEEP_GPIO, BEEP_PIN);
#define BEEP_OFF                 digitalLo(BEEP_GPIO, BEEP_PIN);
#define BEEP_ON                  digitalHi(BEEP_GPIO, BEEP_PIN);
#else
#define BEEP_TOGGLE              ;
#define BEEP_OFF                 ;
#define BEEP_ON                  ;
#endif

#undef SOFT_I2C                 // enable to test software i2c

#include "utils.h"

#ifdef FY90Q
 // FY90Q
#include "drv_adc.h"
#include "drv_i2c.h"
#include "drv_pwm.h"
#include "drv_uart.h"
#else

#ifdef OLIMEXINO
// OLIMEXINO
#include "drv_adc.h"
#include "drv_i2c.h"
#include "drv_spi.h"
#include "drv_adxl345.h"
#include "drv_mpu3050.h"
#include "drv_mpu6050.h"
#include "drv_l3g4200d.h"
#include "drv_pwm.h"
#include "drv_timer.h"
#include "drv_serial.h"
#include "drv_uart.h"
#include "drv_softserial.h"
#else

 // AfroFlight32
#include "drv_adc.h"
#include "drv_adxl345.h"
#include "drv_bma280.h"
#include "drv_bmp085.h"
#include "drv_ms5611.h"
#include "drv_hmc5883l.h"
#include "drv_i2c.h"
#include "drv_spi.h"
#include "drv_ledring.h"
#include "drv_mma845x.h"
#include "drv_mpu3050.h"
#include "drv_mpu6050.h"
#include "drv_l3g4200d.h"
#include "drv_pwm.h"
//#include "drv_timer.h"
#include "drv_serial.h"
#include "drv_uart.h"
#include "drv_softserial.h"
#include "drv_hcsr04.h"

#endif
#endif
