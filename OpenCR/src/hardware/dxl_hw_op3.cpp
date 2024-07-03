/*
 *  dxl_hw_op3.cpp
 *
 *  dynamixel hardware op3
 *
 *  Created on: 2016. 10. 21.
 *      Author: Baram
 */

#include "dxl_hw_op3.h"

#include <IMU.h>


#define LED_PWM_PIN_MAX 3
#define LED_PWM_PWM_MAX 31

#define BUTTON_PIN_MAX 4

#define IMU_CALI_MAX_COUNT 512


static uint8_t led_pwm_pins[LED_PWM_PIN_MAX] = {PIN_LED_R, PIN_LED_G, PIN_LED_B};
static uint8_t led_pwm_value[LED_PWM_PIN_MAX];

static float imu_offset[3];
static int16_t imu_cali_count[3];
static float imu_cali_sum[3];

static uint8_t button_value[BUTTON_PIN_MAX];
static uint32_t button_pin_num[BUTTON_PIN_MAX] = {PIN_BUTTON_S1, PIN_BUTTON_S2, PIN_BUTTON_S3, PIN_BUTTON_S4};


#define BATTERY_POWER_OFF      0
#define BATTERY_POWER_STARTUP  1
#define BATTERY_POWER_NORMAL   2
#define BATTERY_POWER_CHECK    3
#define BATTERY_POWER_WARNNING 4


static uint8_t battery_voltage   = 0;
static float battery_valtage_raw = 0;
static uint8_t battery_state     = BATTERY_POWER_STARTUP;


cIMU IMU;
HardwareTimer Timer(TIMER_CH1);

void dxl_hw_op3_button_update();
void dxl_hw_op3_voltage_update();

/**
 * @brief Use globals to control RGB led with PWM based on 500us timer interrupt
 * @note 5 bit resolution, 500us timer
 */
void handler_led(void) {
    uint32_t i;
    static uint8_t led_counter = 0;


    for (i = 0; i < LED_PWM_PIN_MAX; i++) {
        if (led_counter < led_pwm_value[i] && led_pwm_value[i] > 0) {
            dxl_hw_op3_led_set(led_pwm_pins[i], 0);  // LED ON
        }
        else {
            dxl_hw_op3_led_set(led_pwm_pins[i], 1);  // LED OFF
        }
    }

    led_counter++;
    if (led_counter > LED_PWM_PWM_MAX)
        led_counter = 0;
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_hw_op3_init
     WORK    :
---------------------------------------------------------------------------*/
void dxl_hw_op3_init(void) {
    uint16_t i;


    IMU.begin();


    for (i = 0; i < 3; i++) {
        imu_offset[i]     = 0.0;
        imu_cali_count[i] = 0;
    }

    for (i = 0; i < BUTTON_PIN_MAX; i++) {
        button_value[i] = 0;
    }

    pinMode(PIN_LED_R, OUTPUT);
    pinMode(PIN_LED_G, OUTPUT);
    pinMode(PIN_LED_B, OUTPUT);

    pinMode(PIN_LED_1, OUTPUT);
    pinMode(PIN_LED_2, OUTPUT);
    pinMode(PIN_LED_3, OUTPUT);

    pinMode(PIN_BUTTON_S1, INPUT_PULLUP);
    pinMode(PIN_BUTTON_S2, INPUT_PULLUP);
    pinMode(PIN_BUTTON_S3, INPUT_PULLUP);
    pinMode(PIN_BUTTON_S4, INPUT_PULLUP);


    dxl_hw_op3_led_set(PIN_LED_1, 1);  // R
    dxl_hw_op3_led_set(PIN_LED_2, 1);  // G
    dxl_hw_op3_led_set(PIN_LED_3, 1);  // B


    dxl_hw_op3_led_pwm(PIN_LED_R, 0);
    dxl_hw_op3_led_pwm(PIN_LED_G, 0);
    dxl_hw_op3_led_pwm(PIN_LED_B, 0);

    Timer.pause();
    Timer.setPeriod(500);  // 500us
    Timer.attachInterrupt(handler_led);
    Timer.refresh();
    Timer.resume();
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_hw_op3_update
     WORK    :
---------------------------------------------------------------------------*/
void dxl_hw_op3_update(void) {
    uint8_t i;


    if (IMU.update()) {
        for (i = 0; i < 3; i++) {
            if (imu_cali_count[i] > 0) {
                imu_cali_sum[i] += IMU.rpy[i];
                imu_cali_count[i]--;

                if (imu_cali_count[i] == 0) {
                    imu_offset[i]     = imu_cali_sum[i] / IMU_CALI_MAX_COUNT;
                    imu_cali_count[i] = -1;
                }
            }
        }
    }

    dxl_hw_op3_button_update();
    dxl_hw_op3_voltage_update();
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_hw_op3_button_update
     WORK    : button_value
---------------------------------------------------------------------------*/
void dxl_hw_op3_button_update() {

    // keep button state as "down" for longer before resetting
    const uint8_t debounce_time_ms = 30;
    const uint8_t release_time_ms  = 200;

    enum btn_state { BTN_INACTIVE, BTN_ACTIVE };

    static enum btn_state pin_state[BUTTON_PIN_MAX] = {
        BTN_INACTIVE,
    };
    static uint32_t pin_time[BUTTON_PIN_MAX] = {
        0,
    };

    // loop over each button
    for (uint32_t i = 0; i < BUTTON_PIN_MAX; i++) {

        // get the raw pin value (pull up resistor so invert)
        uint8_t pin_input = !digitalRead(button_pin_num[i]);


        switch (pin_state[i]) {
            // if the button was last unpressed, or pressed and debounced
            case BTN_INACTIVE:
                // newly pressed, or was released since debounce
                if (button_value[i] != pin_input) {
                    // we have an action to log
                    pin_state[i] = BTN_ACTIVE;
                    // and record the timestamp for debouncing
                    pin_time[i] = millis();
                }
                break;

            // we have a press or release to poll for
            case BTN_ACTIVE:
                uint32_t pin_time_delta = millis() - pin_time[i];
                // if the button value is currently off, have a 30ms debounce timer before we log it as pressed.
                // However, if it currently pressed, have a longer timeout before we consider it released.
                if (((button_value[i] == 0) && (pin_time_delta > debounce_time_ms))
                    || ((button_value[i] == 1) && (pin_time_delta > release_time_ms))) {
                    // if it has, update the value
                    button_value[i] = pin_input;
                    // nothing more to do
                    pin_state[i] = BTN_INACTIVE;
                }
                break;
        }
    }
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_hw_op3_button_read
     WORK    :
---------------------------------------------------------------------------*/
uint8_t dxl_hw_op3_button_read(uint8_t pin_num) {
    for (uint8_t i = 0; i < BUTTON_PIN_MAX; i++) {
        if (button_pin_num[i] == pin_num) {
            return button_value[i];
        }
    }

    return 0;
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_hw_op3_led_set
     WORK    :
---------------------------------------------------------------------------*/
void dxl_hw_op3_led_set(uint8_t pin_num, uint8_t value) {
    digitalWrite(pin_num, value);
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_hw_op3_led_pwm
     WORK    :
---------------------------------------------------------------------------*/
void dxl_hw_op3_led_pwm(uint8_t pin_num, uint8_t value) {
    switch (pin_num) {
        case PIN_LED_R: led_pwm_value[0] = constrain(value, 0, LED_PWM_PWM_MAX); break;

        case PIN_LED_G: led_pwm_value[1] = constrain(value, 0, LED_PWM_PWM_MAX); break;

        case PIN_LED_B: led_pwm_value[2] = constrain(value, 0, LED_PWM_PWM_MAX); break;
    }
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_hw_op3_voltage_update
     WORK    :
---------------------------------------------------------------------------*/
void dxl_hw_op3_voltage_update(void) {
    static bool startup    = false;
    static int adc_index   = 0;
    static int prev_state  = 0;
    static int alram_state = 0;
    static int check_index = 0;

    int i;
    int adc_value;
    int adc_sum;
    float vol_value;

    static uint32_t process_time[8] = {
        0,
    };
    static uint16_t adc_value_tbl[10] = {
        0,
    };

    float voltage_ref = 11.1;


    if (millis() - process_time[0] > 100) {
        process_time[0] = millis();
        adc_value       = analogRead(BDPIN_BAT_PWR_ADC);

        adc_value_tbl[adc_index] = adc_value;

        adc_index++;
        adc_index %= 10;

        adc_sum = 0;
        for (i = 0; i < 10; i++) {
            adc_sum += adc_value_tbl[i];
        }
        adc_value           = adc_sum / 10;
        vol_value           = map(adc_value, 0, 1023, 0, 331 * 57 / 10);
        battery_valtage_raw = vol_value / 100;

        battery_valtage_raw += 0.5;

        // /// Serial.println(vol_value);

        vol_value       = battery_valtage_raw * 10;
        vol_value       = constrain(vol_value, 0, 255);
        battery_voltage = vol_value;
    }


    if (millis() - process_time[1] > 1000) {
        process_time[1] = millis();


        switch (battery_state) {
            case BATTERY_POWER_OFF:
                alram_state = 0;
                if (battery_valtage_raw > voltage_ref * 0.20) {
                    prev_state    = battery_state;
                    battery_state = BATTERY_POWER_STARTUP;
                }
                break;

            case BATTERY_POWER_STARTUP:
                if (battery_valtage_raw > voltage_ref) {
                    prev_state    = battery_state;
                    battery_state = BATTERY_POWER_NORMAL;
                }
                else {
                    prev_state    = battery_state;
                    battery_state = BATTERY_POWER_CHECK;
                }
                break;

            case BATTERY_POWER_NORMAL:
                alram_state = 0;
                if (battery_valtage_raw < voltage_ref) {
                    prev_state    = battery_state;
                    battery_state = BATTERY_POWER_CHECK;
                    check_index   = 0;
                }
                break;

            case BATTERY_POWER_CHECK:
                if (check_index < 5) {
                    check_index++;
                }
                else {
                    if (battery_valtage_raw < voltage_ref) {
                        prev_state    = battery_state;
                        battery_state = BATTERY_POWER_WARNNING;
                    }
                }
                if (battery_valtage_raw >= voltage_ref) {
                    prev_state    = battery_state;
                    battery_state = BATTERY_POWER_NORMAL;
                }
                break;

            case BATTERY_POWER_WARNNING:
                // alram_state ^= 1;
                // if(alram_state)
                //{
                //   tone(BDPIN_BUZZER, 1000, 500);
                // }

                if (battery_valtage_raw > voltage_ref) {
                    prev_state    = battery_state;
                    battery_state = BATTERY_POWER_NORMAL;
                }
                if (battery_valtage_raw < voltage_ref * 0.20) {
                    prev_state    = battery_state;
                    battery_state = BATTERY_POWER_OFF;
                }
                break;

            default: break;
        }
    }

    if (battery_state == BATTERY_POWER_WARNNING) {
        if (millis() - process_time[2] >= 200) {
            process_time[2] = millis();

            tone(BDPIN_BUZZER, 1000, 100);
        }
    }
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_hw_op3_voltage_read
     WORK    :
---------------------------------------------------------------------------*/
uint8_t dxl_hw_op3_voltage_read(void) {
    return (uint8_t) battery_voltage;
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_hw_op3_acc_get_x
     WORK    :
---------------------------------------------------------------------------*/
int16_t dxl_hw_op3_acc_get_x(void) {
    return (int16_t) IMU.SEN.accRAW[0];
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_hw_op3_acc_get_y
     WORK    :
---------------------------------------------------------------------------*/
int16_t dxl_hw_op3_acc_get_y(void) {
    return (int16_t) IMU.SEN.accRAW[1];
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_hw_op3_acc_get_z
     WORK    :
---------------------------------------------------------------------------*/
int16_t dxl_hw_op3_acc_get_z(void) {
    return (int16_t) IMU.SEN.accRAW[2];
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_hw_op3_acc_get_x
     WORK    :
---------------------------------------------------------------------------*/
int16_t dxl_hw_op3_gyro_get_x(void) {
    return (int16_t) IMU.SEN.gyroADC[0];
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_hw_op3_acc_get_y
     WORK    :
---------------------------------------------------------------------------*/
int16_t dxl_hw_op3_gyro_get_y(void) {
    return (int16_t) IMU.SEN.gyroADC[1];
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_hw_op3_acc_get_z
     WORK    :
---------------------------------------------------------------------------*/
int16_t dxl_hw_op3_gyro_get_z(void) {
    return (int16_t) IMU.SEN.gyroADC[2];
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_hw_op3_get_rpy
     WORK    :
---------------------------------------------------------------------------*/
int16_t dxl_hw_op3_get_rpy(uint8_t rpy) {
    int16_t ret;


    switch (rpy) {
        case 0: ret = (int16_t) ((IMU.rpy[0] - imu_offset[0]) * 10.); break;

        case 1: ret = (int16_t) ((IMU.rpy[1] - imu_offset[1]) * 10.); break;

        case 2: ret = (int16_t) ((IMU.rpy[2] - imu_offset[2]) * 10.); break;

        default: ret = (int16_t) ((IMU.rpy[0] - imu_offset[0]) * 10.); break;
    }

    return ret;
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_hw_op3_start_cali
     WORK    :
---------------------------------------------------------------------------*/
void dxl_hw_op3_start_cali(uint8_t index) {
    imu_cali_count[index] = IMU_CALI_MAX_COUNT;
    imu_cali_sum[index]   = 0;
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_hw_op3_clear_cali
     WORK    :
---------------------------------------------------------------------------*/
void dxl_hw_op3_clear_cali(uint8_t index) {
    imu_cali_count[index] = 0;
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_hw_op3_get_cali
     WORK    :
---------------------------------------------------------------------------*/
int16_t dxl_hw_op3_get_cali(uint8_t index) {
    return imu_cali_count[index];
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_hw_op3_set_offset
     WORK    :
---------------------------------------------------------------------------*/
void dxl_hw_op3_set_offset(uint8_t index, float offset_data) {
    imu_offset[index] = offset_data;
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_hw_op3_get_offset
     WORK    :
---------------------------------------------------------------------------*/
float dxl_hw_op3_get_offset(uint8_t index) {
    return imu_offset[index];
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_hw_op3_start_gyro_cali
     WORK    :
---------------------------------------------------------------------------*/
void dxl_hw_op3_start_gyro_cali(void) {
    IMU.SEN.gyro_cali_start();
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_hw_op3_get_gyro_cali_done
     WORK    :
---------------------------------------------------------------------------*/
bool dxl_hw_op3_get_gyro_cali_done(void) {
    return IMU.SEN.gyro_cali_get_done();
}
