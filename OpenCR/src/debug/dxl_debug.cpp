/*
 *  dxl_node_op3.cpp
 *
 *  debug node op3
 *
 *  Created on: 2017. 2. 2.
 *      Author: Baram
 */

#include "dxl_debug.h"

#include <EEPROM.h>

#include "../hardware/dxl_hw.h"
#include "../hardware/dxl_hw_op3.h"
#include "../protocol/dxl_node_op3.h"


#define DEBUG_SERIAL Serial
#define DEBUG_BAUD   115200
#define DEBUG_SWITCH BDPIN_DIP_SW_1
#define DEBUG_LED    BDPIN_LED_USER_3


extern dxl_mem_op3_t* p_dxl_mem;

// are we in debug mode (generally set by DIP switch)
uint8_t debug_state = 0;
// have we forced debug mode on?
uint8_t debug_override = 0;
// character to force debug mode
const char override_char = '`';  // grave (`)


static void dxl_debug_menu_show_list(void);
static bool dxl_debug_menu_loop(uint8_t ch);
static void dxl_debug_menu_show_cmdline(void);
static bool dxl_debug_menu_show_ctrltbl();

static void dxl_debug_write_ctrltbl(void);
static void dxl_debug_send_write_command(void);
static void dxl_debug_test_gpio(void);
static void dxl_debug_buzzer();


/*---------------------------------------------------------------------------
     TITLE   : dxl_debug_init
     WORK    :
---------------------------------------------------------------------------*/
void dxl_debug_init(void) {
    DEBUG_SERIAL.begin(DEBUG_BAUD);
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_debug_loop
     WORK    :
---------------------------------------------------------------------------*/
void dxl_debug_loop(void) {

    /* Activate debug mode if dip switch active */
    uint8_t dip = digitalReadFast(DEBUG_SWITCH);
    // show debug state with LED
    digitalWriteFast(DEBUG_LED, dip);
    // set state (dip switch pulled up), or override if needed
    debug_state = !dip | debug_override;

    if (DEBUG_SERIAL.available()) {
        uint8_t ch = DEBUG_SERIAL.read();


        switch (debug_state) {
            case 0:
                // force debug on if override is sent, regardless of dip switch.
                debug_override = (ch == override_char);
                break;

            case 1: dxl_debug_menu_loop(ch); break;

            default: break;
        }
    }
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_debug_menu_show_list
     WORK    :
---------------------------------------------------------------------------*/
void dxl_debug_menu_show_list(void) {
    DEBUG_SERIAL.println("---------------------------");
    DEBUG_SERIAL.println("m - show menu");
    DEBUG_SERIAL.println("d - show step");
    DEBUG_SERIAL.println("l - show control table");
    DEBUG_SERIAL.println("c - write to control table");
    DEBUG_SERIAL.println("s - send dynamixel write command");
    DEBUG_SERIAL.println("g - test gpio (buttons)");
    DEBUG_SERIAL.println("b - test buzzer");
    DEBUG_SERIAL.println("q - exit menu");
    DEBUG_SERIAL.println("---------------------------");
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_debug_menu_show_cmdline
     WORK    :
---------------------------------------------------------------------------*/
void dxl_debug_menu_show_cmdline(void) {
    DEBUG_SERIAL.print(">>");
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_debug_menu_show_list
     WORK    :
---------------------------------------------------------------------------*/
bool dxl_debug_menu_loop(uint8_t ch) {
    bool exit_menu = false;


    switch (ch) {
        case 'm': dxl_debug_menu_show_list(); break;


        case 'q':
            exit_menu = true;
            // disable override on menu exit
            debug_override = 0;
            DEBUG_SERIAL.println(" ");
            DEBUG_SERIAL.println("exit menu...");
            break;

        case 'd': break;

        case 'l':
            DEBUG_SERIAL.println(" ");
            dxl_debug_menu_show_ctrltbl();
            break;

        case 'c':
            DEBUG_SERIAL.println(" ");
            dxl_debug_write_ctrltbl();
            break;

        case 's':
            DEBUG_SERIAL.println(" ");
            dxl_debug_send_write_command();
            break;

        case 'g':
            DEBUG_SERIAL.println(" ");
            dxl_debug_test_gpio();
            break;

        case 'b':
            DEBUG_SERIAL.println(" ");
            dxl_debug_buzzer();
            break;

        default: exit_menu = true; break;
    }


    if (exit_menu == false) {
        dxl_debug_menu_show_cmdline();
    }

    return exit_menu;
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_debug_menu_show_ctrltbl
     WORK    :
---------------------------------------------------------------------------*/
bool dxl_debug_menu_show_ctrltbl() {
    uint32_t addr;


    addr = (uint32_t) &p_dxl_mem->Model_Number - (uint32_t) p_dxl_mem;
    DEBUG_SERIAL.print(addr);
    DEBUG_SERIAL.print("\t Model_Number    \t ");
    DEBUG_SERIAL.print(p_dxl_mem->Model_Number);
    DEBUG_SERIAL.print("\t 0x");
    DEBUG_SERIAL.println(p_dxl_mem->Model_Number, HEX);

    addr = (uint32_t) &p_dxl_mem->Firmware_Version - (uint32_t) p_dxl_mem;
    DEBUG_SERIAL.print(addr);
    DEBUG_SERIAL.print("\t Firmware_Version\t ");
    DEBUG_SERIAL.print(p_dxl_mem->Firmware_Version);
    DEBUG_SERIAL.print("\t 0x");
    DEBUG_SERIAL.println(p_dxl_mem->Firmware_Version, HEX);

    addr = (uint32_t) &p_dxl_mem->ID - (uint32_t) p_dxl_mem;
    DEBUG_SERIAL.print(addr);
    DEBUG_SERIAL.print("\t ID              \t ");
    DEBUG_SERIAL.print(p_dxl_mem->ID);
    DEBUG_SERIAL.print("\t 0x");
    DEBUG_SERIAL.println(p_dxl_mem->ID, HEX);

    addr = (uint32_t) &p_dxl_mem->Baud - (uint32_t) p_dxl_mem;
    DEBUG_SERIAL.print(addr);
    DEBUG_SERIAL.print("\t Baud            \t ");
    DEBUG_SERIAL.print(p_dxl_mem->Baud);
    DEBUG_SERIAL.print("\t 0x");
    DEBUG_SERIAL.println(p_dxl_mem->Baud, HEX);

    addr = (uint32_t) &p_dxl_mem->Return_Delay_Time - (uint32_t) p_dxl_mem;
    DEBUG_SERIAL.print(addr);
    DEBUG_SERIAL.print("\t Return_Delay_Time \t ");
    DEBUG_SERIAL.print(p_dxl_mem->Return_Delay_Time);
    DEBUG_SERIAL.print("\t 0x");
    DEBUG_SERIAL.println(p_dxl_mem->Return_Delay_Time, HEX);

    addr = (uint32_t) &p_dxl_mem->Status_Return_Level - (uint32_t) p_dxl_mem;
    DEBUG_SERIAL.print(addr);
    DEBUG_SERIAL.print("\t Status_Return_Level \t ");
    DEBUG_SERIAL.print(p_dxl_mem->Status_Return_Level);
    DEBUG_SERIAL.print("\t 0x");
    DEBUG_SERIAL.println(p_dxl_mem->Status_Return_Level, HEX);

    addr = (uint32_t) &p_dxl_mem->Roll_Offset - (uint32_t) p_dxl_mem;
    DEBUG_SERIAL.print(addr);
    DEBUG_SERIAL.print("\t Roll_Offset     \t ");
    DEBUG_SERIAL.print(p_dxl_mem->Roll_Offset);
    DEBUG_SERIAL.print("\t 0x");
    DEBUG_SERIAL.println(p_dxl_mem->Roll_Offset, HEX);

    addr = (uint32_t) &p_dxl_mem->Pitch_Offset - (uint32_t) p_dxl_mem;
    DEBUG_SERIAL.print(addr);
    DEBUG_SERIAL.print("\t Pitch_Offset    \t ");
    DEBUG_SERIAL.print(p_dxl_mem->Pitch_Offset);
    DEBUG_SERIAL.print("\t 0x");
    DEBUG_SERIAL.println(p_dxl_mem->Pitch_Offset, HEX);

    addr = (uint32_t) &p_dxl_mem->Yaw_Offset - (uint32_t) p_dxl_mem;
    DEBUG_SERIAL.print(addr);
    DEBUG_SERIAL.print("\t Yaw_Offset      \t ");
    DEBUG_SERIAL.print(p_dxl_mem->Yaw_Offset);
    DEBUG_SERIAL.print("\t 0x");
    DEBUG_SERIAL.println(p_dxl_mem->Yaw_Offset, HEX);

    addr = (uint32_t) &p_dxl_mem->Dynamixel_Power - (uint32_t) p_dxl_mem;
    DEBUG_SERIAL.print(addr);
    DEBUG_SERIAL.print("\t Dynamixel_Power \t ");
    DEBUG_SERIAL.print(p_dxl_mem->Dynamixel_Power);
    DEBUG_SERIAL.print("\t 0x");
    DEBUG_SERIAL.println(p_dxl_mem->Dynamixel_Power, HEX);

    addr = (uint32_t) &p_dxl_mem->LED - (uint32_t) p_dxl_mem;
    DEBUG_SERIAL.print(addr);
    DEBUG_SERIAL.print("\t LED             \t ");
    DEBUG_SERIAL.print(p_dxl_mem->LED);
    DEBUG_SERIAL.print("\t 0x");
    DEBUG_SERIAL.println(p_dxl_mem->LED, HEX);

    addr = (uint32_t) &p_dxl_mem->LED_RGB - (uint32_t) p_dxl_mem;
    DEBUG_SERIAL.print(addr);
    DEBUG_SERIAL.print("\t LED_RGB         \t ");
    DEBUG_SERIAL.print(p_dxl_mem->LED_RGB);
    DEBUG_SERIAL.print("\t 0x");
    DEBUG_SERIAL.println(p_dxl_mem->LED_RGB, HEX);

    addr = (uint32_t) &p_dxl_mem->Buzzer - (uint32_t) p_dxl_mem;
    DEBUG_SERIAL.print(addr);
    DEBUG_SERIAL.print("\t Buzzer          \t ");
    DEBUG_SERIAL.print(p_dxl_mem->Buzzer);
    DEBUG_SERIAL.print("\t 0x");
    DEBUG_SERIAL.println(p_dxl_mem->Buzzer, HEX);

    addr = (uint32_t) &p_dxl_mem->Button - (uint32_t) p_dxl_mem;
    DEBUG_SERIAL.print(addr);
    DEBUG_SERIAL.print("\t Button          \t ");
    DEBUG_SERIAL.print(p_dxl_mem->Button);
    DEBUG_SERIAL.print("\t 0x");
    DEBUG_SERIAL.println(p_dxl_mem->Button, HEX);

    addr = (uint32_t) &p_dxl_mem->Voltage - (uint32_t) p_dxl_mem;
    DEBUG_SERIAL.print(addr);
    DEBUG_SERIAL.print("\t Voltage         \t ");
    DEBUG_SERIAL.print(p_dxl_mem->Voltage);
    DEBUG_SERIAL.print("\t 0x");
    DEBUG_SERIAL.println(p_dxl_mem->Voltage, HEX);

    addr = (uint32_t) &p_dxl_mem->Gyro_X - (uint32_t) p_dxl_mem;
    DEBUG_SERIAL.print(addr);
    DEBUG_SERIAL.print("\t Gyro_X          \t ");
    DEBUG_SERIAL.print(p_dxl_mem->Gyro_X);
    DEBUG_SERIAL.print("\t 0x");
    DEBUG_SERIAL.println(p_dxl_mem->Gyro_X, HEX);

    addr = (uint32_t) &p_dxl_mem->Gyro_Y - (uint32_t) p_dxl_mem;
    DEBUG_SERIAL.print(addr);
    DEBUG_SERIAL.print("\t Gyro_Y          \t ");
    DEBUG_SERIAL.print(p_dxl_mem->Gyro_Y);
    DEBUG_SERIAL.print("\t 0x");
    DEBUG_SERIAL.println(p_dxl_mem->Gyro_Y, HEX);

    addr = (uint32_t) &p_dxl_mem->Gyro_Z - (uint32_t) p_dxl_mem;
    DEBUG_SERIAL.print(addr);
    DEBUG_SERIAL.print("\t Gyro_Z          \t ");
    DEBUG_SERIAL.print(p_dxl_mem->Gyro_Z);
    DEBUG_SERIAL.print("\t 0x");
    DEBUG_SERIAL.println(p_dxl_mem->Gyro_Z, HEX);

    addr = (uint32_t) &p_dxl_mem->Acc_X - (uint32_t) p_dxl_mem;
    DEBUG_SERIAL.print(addr);
    DEBUG_SERIAL.print("\t Acc_X           \t ");
    DEBUG_SERIAL.print(p_dxl_mem->Acc_X);
    DEBUG_SERIAL.print("\t 0x");
    DEBUG_SERIAL.println(p_dxl_mem->Acc_X, HEX);

    addr = (uint32_t) &p_dxl_mem->Acc_Y - (uint32_t) p_dxl_mem;
    DEBUG_SERIAL.print(addr);
    DEBUG_SERIAL.print("\t Acc_Y           \t ");
    DEBUG_SERIAL.print(p_dxl_mem->Acc_Y);
    DEBUG_SERIAL.print("\t 0x");
    DEBUG_SERIAL.println(p_dxl_mem->Acc_Y, HEX);

    addr = (uint32_t) &p_dxl_mem->Acc_Z - (uint32_t) p_dxl_mem;
    DEBUG_SERIAL.print(addr);
    DEBUG_SERIAL.print("\t Acc_Z           \t ");
    DEBUG_SERIAL.print(p_dxl_mem->Acc_Z);
    DEBUG_SERIAL.print("\t 0x");
    DEBUG_SERIAL.println(p_dxl_mem->Acc_Z, HEX);

    addr = (uint32_t) &p_dxl_mem->Roll - (uint32_t) p_dxl_mem;
    DEBUG_SERIAL.print(addr);
    DEBUG_SERIAL.print("\t Roll            \t ");
    DEBUG_SERIAL.print(p_dxl_mem->Roll);
    DEBUG_SERIAL.print("\t 0x");
    DEBUG_SERIAL.println(p_dxl_mem->Roll, HEX);

    addr = (uint32_t) &p_dxl_mem->Pitch - (uint32_t) p_dxl_mem;
    DEBUG_SERIAL.print(addr);
    DEBUG_SERIAL.print("\t Pitch           \t ");
    DEBUG_SERIAL.print(p_dxl_mem->Pitch);
    DEBUG_SERIAL.print("\t 0x");
    DEBUG_SERIAL.println(p_dxl_mem->Pitch, HEX);

    addr = (uint32_t) &p_dxl_mem->Yaw - (uint32_t) p_dxl_mem;
    DEBUG_SERIAL.print(addr);
    DEBUG_SERIAL.print("\t Yaw             \t ");
    DEBUG_SERIAL.print(p_dxl_mem->Yaw);
    DEBUG_SERIAL.print("\t 0x");
    DEBUG_SERIAL.println(p_dxl_mem->Yaw, HEX);

    addr = (uint32_t) &p_dxl_mem->IMU_Control - (uint32_t) p_dxl_mem;
    DEBUG_SERIAL.print(addr);
    DEBUG_SERIAL.print("\t IMU_Control     \t ");
    DEBUG_SERIAL.print(p_dxl_mem->IMU_Control);
    DEBUG_SERIAL.print("\t 0x");
    DEBUG_SERIAL.println(p_dxl_mem->IMU_Control, HEX);
}

/**
 * @brief Write a value to the control table at a given address
 */
void dxl_debug_write_ctrltbl(void) {
    DEBUG_SERIAL.print("[>] Address: ");
    while (!DEBUG_SERIAL.available())
        ;
    int addr = DEBUG_SERIAL.parseInt();

    // Match address to control table
    uint8_t known;
    switch (addr) {
        case 0:
        case 2:
        case 3:
        case 4:
        case 5:
        case 16:
        case 18:
        case 20:
        case 22:
        case 24:
        case 25:
        case 26:
        case 28:
        case 30:
        case 31:
        case 32:
        case 34:
        case 36:
        case 38:
        case 40:
        case 42:
        case 44:
        case 46:
        case 48:
        case 50: known = 1; break;
        default: known = 0; break;
    }

    // Confirm if the address isn't a known address
    if (!known) {
        DEBUG_SERIAL.print("[!] Address ");
        DEBUG_SERIAL.print(addr);
        DEBUG_SERIAL.println(" is not a known control table field. Proceed? (y/N)");
        while (!DEBUG_SERIAL.available())
            ;
        char c = DEBUG_SERIAL.read();
        if (c != 'y')
            return;
    }

    DEBUG_SERIAL.print("[>] Value: ");
    while (!DEBUG_SERIAL.available())
        ;
    int val = DEBUG_SERIAL.parseInt();

    // Print confirmation with name of address if possible
    DEBUG_SERIAL.print("[*] Writing value ");
    // Print in hex for easy reading and simple validation
    // DEBUG_SERIAL.print("0x");
    // DEBUG_SERIAL.print(val, HEX);
    DEBUG_SERIAL.print(val);
    DEBUG_SERIAL.print(" to ");
    switch (addr) {
        case 0: DEBUG_SERIAL.print("Model_Number"); break;
        case 2: DEBUG_SERIAL.print("Firmware_Version"); break;
        case 3: DEBUG_SERIAL.print("ID"); break;
        case 4: DEBUG_SERIAL.print("Baud"); break;
        case 5: DEBUG_SERIAL.print("Return_Delay_Time"); break;
        case 16: DEBUG_SERIAL.print("Status_Return_Level"); break;
        case 18: DEBUG_SERIAL.print("Roll_Offset"); break;
        case 20: DEBUG_SERIAL.print("Pitch_Offset"); break;
        case 22: DEBUG_SERIAL.print("Yaw_Offset"); break;
        case 24: DEBUG_SERIAL.print("Dynamixel_Power"); break;
        case 25: DEBUG_SERIAL.print("LED"); break;
        case 26: DEBUG_SERIAL.print("LED_RGB"); break;
        case 28: DEBUG_SERIAL.print("Buzzer"); break;
        case 30: DEBUG_SERIAL.print("Button"); break;
        case 31: DEBUG_SERIAL.print("Voltage"); break;
        case 32: DEBUG_SERIAL.print("Gyro_X"); break;
        case 34: DEBUG_SERIAL.print("Gyro_Y"); break;
        case 36: DEBUG_SERIAL.print("Gyro_Z"); break;
        case 38: DEBUG_SERIAL.print("Acc_X"); break;
        case 40: DEBUG_SERIAL.print("Acc_Y"); break;
        case 42: DEBUG_SERIAL.print("Acc_Z"); break;
        case 44: DEBUG_SERIAL.print("Roll"); break;
        case 46: DEBUG_SERIAL.print("Pitch"); break;
        case 48: DEBUG_SERIAL.print("Yaw"); break;
        case 50: DEBUG_SERIAL.print("IMU_Control"); break;
        default: DEBUG_SERIAL.print("Unknown"); break;
    }
    DEBUG_SERIAL.println(".");

    // get the address of the value in the control table
    dxl_debug_write_byte_wrapper(addr, val);
}

/**
 * @brief Send a dynamixel command to the specified ID
 */
void dxl_debug_send_write_command(void) {

    DEBUG_SERIAL.print("Input format `<id> <addr> <data> ~` with sentinel. ");
    DEBUG_SERIAL.print("[!] Input wait is BLOCKING because im LAZY");

    const uint8_t buf_size = 32;
    char buf[buf_size];
    buf[buf_size - 1] = '\0';  // just in case

    // read into buffer (blocking, very bad)
    int i = 0;
    while (i < buf_size - 1) {
        if (DEBUG_SERIAL.available()) {
            uint8_t ch = DEBUG_SERIAL.read();
            // DEBUG_SERIAL.printf("%c", ch);
            // handle sentinel value (or other possible EOF)
            if (ch == '~' || ch == '\n' || ch == '\r') {
                buf[i] = '\0';
                break;
            }
            // otherwise insert
            buf[i++] = ch;
        }
    }

    // fill vars
    char* p  = buf;
    int id   = strtol(p, &p, 10);
    int addr = strtol(p, &p, 10);
    int data = strtol(p, &p, 10);

    //                  0     1     2     3     4  5     6     7     8  9  10 11 12 13 14 15
    uint8_t packet[] = {0xFF, 0xFF, 0xFD, 0x00, 0, 0x09, 0x00, 0x03, 0, 0, 0, 0, 0, 0, 0, 0};
    enum blanks {
        ID     = 4,
        ADDR_L = 8,
        ADDR_H = 9,
        DATA1  = 10,
        DATA2  = 11,
        DATA3  = 12,
        DATA4  = 13,
        CRC_L  = 14,
        CRC_H  = 15
    };

    // fill in the blanks
    packet[ID]     = (uint8_t) ((id & 0xFF));
    packet[ADDR_L] = (uint8_t) ((addr & 0x00FF));
    packet[ADDR_H] = (uint8_t) ((addr & 0xFF00) >> 8);
    packet[DATA1]  = (uint8_t) ((data & 0x000000FF));
    packet[DATA2]  = (uint8_t) ((data & 0x0000FF00) >> 8);
    packet[DATA3]  = (uint8_t) ((data & 0x00FF0000) >> 16);
    packet[DATA4]  = (uint8_t) ((data & 0xFF000000) >> 24);

    DEBUG_SERIAL.printf("ID %02x -> %02x, Addr %04x -> %02x %02x, Data %08x -> %02x %02x %02x %02x \n",
                        id,
                        packet[ID],
                        addr,
                        packet[ADDR_L],
                        packet[ADDR_H],
                        data,
                        packet[DATA1],
                        packet[DATA2],
                        packet[DATA3],
                        packet[DATA4]);

    // calculate CRC
    uint16_t crc = 0;

    // pass each consective byte of the struct until we reach the CRC field
    for (int i = 0; i < (sizeof(packet) - 2); i++) {
        dxlUpdateCrc(&crc, packet[i]);
    }

    // done, insert crc into struct
    packet[CRC_L] = (crc & 0x00FF);
    packet[CRC_H] = (crc & 0xFF00) >> 8;


    // create container packet and fill
    dxl_t container;
    for (int i = 0; i < sizeof(packet); i++) {
        container.tx.data[i] = packet[i];
    }
    container.tx.packet_length = sizeof(packet);

    // send packet
    dxlTxPacket(&container);
}

/**
 * @brief poll the gpio pins to see what is being read
 */
void dxl_debug_test_gpio(void) {
    // entry messages
    DEBUG_SERIAL.println("---------------------------");
    DEBUG_SERIAL.println("> gpio state will print on update");
    DEBUG_SERIAL.println("> pins in bit order w/ LSB = BDPIN_GPIO_1 (silkscreen gpio 3)");
    DEBUG_SERIAL.println("> MSB is always 1 to prevent zeroes truncating");
    DEBUG_SERIAL.println("m - return to menu");
    DEBUG_SERIAL.println("b - toggle debouncing (default on)");
    DEBUG_SERIAL.println("---------------------------");
    DEBUG_SERIAL.println("      x.........987654321");

    /* config variables */
    // do we want debounced or raw pin values
    bool debounce = true;
    // The 10x10 header on the board, minus 3v3 and GND
    const uint8_t num_gpio_pins = 18;
    // gpio pin definitions from OpenCR's chip.h
    const uint32_t gpio_pin_num[num_gpio_pins] = {BDPIN_GPIO_1,
                                                  BDPIN_GPIO_2,
                                                  BDPIN_GPIO_3,
                                                  BDPIN_GPIO_4,
                                                  BDPIN_GPIO_5,
                                                  BDPIN_GPIO_6,
                                                  BDPIN_GPIO_7,
                                                  BDPIN_GPIO_8,
                                                  BDPIN_GPIO_9,
                                                  BDPIN_GPIO_10,
                                                  BDPIN_GPIO_11,
                                                  BDPIN_GPIO_12,
                                                  BDPIN_GPIO_13,
                                                  BDPIN_GPIO_14,
                                                  BDPIN_GPIO_15,
                                                  BDPIN_GPIO_16,
                                                  BDPIN_GPIO_17,
                                                  BDPIN_GPIO_18};

    // for keeping track while printing (overflow at 255)
    static uint8_t poll_count = 0;
    // bit field for each gpio pin value
    static uint32_t gpio_state = 0;
    // temp loop variable for state
    static uint32_t gpio_state_now = 0;
    // scope serial character variable outside do-while
    char ch;

    /* debounce variables */
    // keep track of whether a pin is being debounced (as bit)
    static uint32_t gpio_debounce_state = 0;
    // how long to wait for debouncing
    const uint8_t debounce_time_ms = 30;
    // ms since last pin poll time
    static uint32_t pin_time[num_gpio_pins] = {
        0,
    };


    /* gpio testing loop */
    do {
        // poll each gpio pin
        for (int pin = 0; pin < num_gpio_pins; pin++) {

            // read the current real time state (invert because of pullups)
            const uint8_t pin_state_now = !digitalRead(gpio_pin_num[pin]);
            // the last "true" (debounced) pin state
            const uint8_t pin_value = (gpio_state >> pin) & 1;

            // simple pin polling if debounce is off
            if (debounce == false) {
                // set the bit active if pin is active
                gpio_state_now |= pin_state_now << pin;
            }

            // otherwise run debounce routine
            else {
                // is the pin currently being debounced?
                const bool pin_debouncing = (gpio_debounce_state >> pin) & 1;

                // pin inactive, already debounced / nothing happened yet
                if (pin_debouncing == false) {
                    // if the pin state has changed since last debounce
                    if (pin_value != pin_state_now) {
                        // set the bit to log a debounce action
                        gpio_debounce_state |= 1 << pin;
                        // and record the timestamp for debouncing
                        pin_time[pin] = millis();
                    }
                }
                // active, we recently went high or low
                else {
                    // is the time delta greater than the debounce threshold
                    if ((millis() - pin_time[pin]) > debounce_time_ms) {
                        // update the  current gpio state
                        gpio_state_now &= ~(1 << pin);           // bit reset
                        gpio_state_now |= pin_state_now << pin;  // bit set

                        // clear the debounce state
                        gpio_debounce_state &= ~(1 << pin);
                    }
                }
            }
        }

        // bit mask the field just in case
        // check it's new
        if (gpio_state_now != gpio_state) {

            // debug the debug
            // DEBUG_SERIAL.print("\n[*] ");
            // DEBUG_SERIAL.print(gpio_state_now, BIN);
            // DEBUG_SERIAL.print(" ");
            // DEBUG_SERIAL.println(gpio_state, BIN);

            // update state variable
            gpio_state = gpio_state_now;

            // poll counter
            DEBUG_SERIAL.print("[");
            // leading zeros
            DEBUG_SERIAL.print(poll_count < 100 ? (poll_count < 10 ? "00" : "0") : "");
            // print round and increment
            DEBUG_SERIAL.print(poll_count++);
            DEBUG_SERIAL.print("] ");

            // print pin state with a leading 1 to ensure every bit prints (not truncated 0)
            DEBUG_SERIAL.println((gpio_state | (1 << num_gpio_pins)), BIN);

            // And print the button state too
            //     uint32_t addr = (uint32_t) &p_dxl_mem->Button - (uint32_t) p_dxl_mem;
            //     DEBUG_SERIAL.print(addr);
            //     DEBUG_SERIAL.print("\t Button          \t ");
            //     DEBUG_SERIAL.print(p_dxl_mem->Button);
            //     DEBUG_SERIAL.print("\t 0x");
            //     DEBUG_SERIAL.println(p_dxl_mem->Button, HEX);
        }

        // if a charcater was sent, read it, else make it null
        ch = DEBUG_SERIAL.available() ? DEBUG_SERIAL.read() : '\0';

        // toggle debouncing if 'b' sent
        if (ch == 'b') {
            debounce = debounce ? false : true;
            // print status
            DEBUG_SERIAL.print(">> debounce ");
            DEBUG_SERIAL.println(debounce ? "enabled" : "disabled");
        }
    }
    // exit character
    while (ch != 'm');
    // show menu
    dxl_debug_menu_show_list();
}
/**
 * @brief Sound the buzzer at a given frequency and duration
 */
void dxl_debug_buzzer() {
    DEBUG_SERIAL.print("[>] Frequency (Hz): ");
    while (!DEBUG_SERIAL.available())
        ;
    int freq = DEBUG_SERIAL.parseInt();

    DEBUG_SERIAL.print("[>] Duration (ms): ");
    while (!DEBUG_SERIAL.available())
        ;
    int dur = DEBUG_SERIAL.parseInt();

    // End if frequency is 0, delayed by duration
    if (freq == 0) {
        delay(dur);
        noTone(BDPIN_BUZZER);
        return;
    }

    // Confirm frequency
    DEBUG_SERIAL.print("[*] Playing tone at ");
    DEBUG_SERIAL.print(freq);
    DEBUG_SERIAL.print("Hz");

    // Test without duration if duration is 0
    if (dur == 0) {
        DEBUG_SERIAL.println(" indefinitely.");
        tone(BDPIN_BUZZER, freq);

        return;
    }

    // Confirm duration
    DEBUG_SERIAL.print(" for ");
    DEBUG_SERIAL.print(dur);
    DEBUG_SERIAL.println("ms");

    // Play the tone normally
    tone(BDPIN_BUZZER, freq, dur);
}
