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
#define DEBUG_BAUD   57600
#define DEBUG_SWITCH BDPIN_DIP_SW_1
#define DEBUG_LED    BDPIN_LED_USER_3


extern dxl_mem_op3_t* p_dxl_mem;

static uint8_t debug_state = 0;


static void dxl_debug_menu_show_list(void);
static bool dxl_debug_menu_loop(uint8_t ch);
static void dxl_debug_menu_show_cmdline(void);
static bool dxl_debug_menu_shwo_ctrltbl();

static void dxl_debug_send_write_command(void);


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
    uint8_t ch;

    /* Activate debug mode if dip switch active */
    uint8_t dip = digitalReadFast(DEBUG_SWITCH);
    // show debug state with LED
    digitalWriteFast(DEBUG_LED, dip);
    // set state (dip switch pulled up)
    debug_state = !dip;

    if (DEBUG_SERIAL.available()) {
        ch = DEBUG_SERIAL.read();


        switch (debug_state) {
            case 0:
                if (ch == 'm') {
                    debug_state = 1;
                    dxl_debug_menu_show_list();
                    dxl_debug_menu_show_cmdline();
                }
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
    DEBUG_SERIAL.println("s - send dynamixel write command");
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
            DEBUG_SERIAL.println(" ");
            DEBUG_SERIAL.println("exit menu...");
            break;

        case 'd': break;

        case 'l':
            DEBUG_SERIAL.println(" ");
            dxl_debug_menu_shwo_ctrltbl();
            break;

        case 's':
            DEBUG_SERIAL.println(" ");
            dxl_debug_send_write_command();
            break;


        default: exit_menu = true; break;
    }


    if (exit_menu == false) {
        dxl_debug_menu_show_cmdline();
    }

    return exit_menu;
}


/*---------------------------------------------------------------------------
     TITLE   : dxl_debug_menu_shwo_ctrltbl
     WORK    :
---------------------------------------------------------------------------*/
bool dxl_debug_menu_shwo_ctrltbl() {
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
 * @brief Send a dynamixel command to the specified ID
 */
void dxl_debug_send_write_command(void) {

    const uint8_t buf_size = 32;
    char buf[buf_size];
    buf[buf_size - 1] = '\0';  // just in case

    DEBUG_SERIAL.print("Input format `<id> <addr> <data> ~` with sentinel. ");
    DEBUG_SERIAL.print("[!] Input wait is BLOCKING because im LAZY");

    // read into buffer (blocking, very bad)
    int i = 0;
    while (i < buf_size - 1) {
        if (DEBUG_SERIAL.available()) {
            uint8_t ch = DEBUG_SERIAL.read();
            /**/ DEBUG_SERIAL.printf("%c ", ch);
            // handle sentinel value (or other possible EOF)
            if (ch == '~' || ch == '\n' || ch == '\r') {
                buf[i] = '\0';
                break;
            }

            // otherwise insert
            buf[i++] = ch;
        }
    }

    /**/ DEBUG_SERIAL.printf("[*] finished read: %s\n", buf);

    // fill vars
    char* p  = buf;
    int id   = strtol(p, &p, 10);
    int addr = strtol(p, &p, 10);
    int data = strtol(p, &p, 10);

    /**/ DEBUG_SERIAL.println("[*] filled vars");

    // // create instruction struct
    // dxl_debug_write_packet_t packet;

    // // fill in the blanks
    // packet.id     = (uint8_t) (id & 0xFF);
    // packet.addr_l = (uint8_t) (addr & 0x00FF);
    // packet.addr_h = (uint8_t) (addr & 0xFF00) >> 8;
    // packet.data1  = (uint8_t) (data & 0x000000FF);
    // packet.data2  = (uint8_t) (data & 0x0000FF00) >> 8;
    // packet.data3  = (uint8_t) (data & 0x00FF0000) >> 16;
    // packet.data4  = (uint8_t) (data & 0xFF000000) >> 24;

    // /**/ DEBUG_SERIAL.println("[*] filled packet");

    // // calculate CRC
    // uint16_t crc = 0;
    // /**/ DEBUG_SERIAL.println("Completed packet");
    // // pass each consective byte of the struct until we reach the CRC field
    // for (int i = 0; i < (sizeof(packet) - 2); ++i) {
    //     /**/ DEBUG_SERIAL.printf("%x ", *((uint8_t*) (&packet + i)));

    //     dxlUpdateCrc(&crc, *((uint8_t*) (&packet + i)));
    // }

    // DEBUG_SERIAL.setTimeout(10000);  // 10 second timeout
    // DEBUG_SERIAL.println("Enter values as decimal followed by any whitespace");
    // DEBUG_SERIAL.print("ID> ");
    // int id = DEBUG_SERIAL.parseInt();
    // DEBUG_SERIAL.print("Addr> ");
    // int addr = DEBUG_SERIAL.parseInt();
    // DEBUG_SERIAL.print("Data> ");
    // int data = DEBUG_SERIAL.parseInt();
    // DEBUG_SERIAL.setTimeout(1000);  // reset to default

    // //                  0     1     2     3     4  5     6     7     8  9  10 11 12 13 14 15
    // uint8_t packet[] = {0xFF, 0xFF, 0xFD, 0x00, 0, 0x09, 0x00, 0x03, 0, 0, 0, 0, 0, 0, 0, 0};
    // enum blanks {
    //     ID     = 4,
    //     ADDR_L = 8,
    //     ADDR_H = 9,
    //     DATA1  = 10,
    //     DATA2  = 11,
    //     DATA3  = 12,
    //     DATA4  = 13,
    //     CRC_L  = 14,
    //     CRC_H  = 15
    // };

    //*.................................../*!*/
    uint8_t packet[] = {0xFF, 0xFF, 0xFD, 0xFD, 0x00, 0, 0x09, 0x00, 0x03, 0, 0, 0, 0, 0, 0, 0, 0};
    enum blanks {
        ID     = 4 + 1,
        ADDR_L = 8 + 1,
        ADDR_H = 9 + 1,
        DATA1  = 10 + 1,
        DATA2  = 11 + 1,
        DATA3  = 12 + 1,
        DATA4  = 13 + 1,
        CRC_L  = 14 + 1,
        CRC_H  = 15 + 1
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
    for (int i = 0; i < (sizeof(packet) - 2); ++i) {
        /**/ DEBUG_SERIAL.printf("%02x ", packet[i]);
        dxlUpdateCrc(&crc, packet[i]);
    }

    // done, insert crc into struct
    packet[CRC_L] = (crc & 0x00FF);
    packet[CRC_H] = (crc & 0xFF00) >> 8;

    /**/ DEBUG_SERIAL.printf("%02x ", packet[CRC_L]);
    /**/ DEBUG_SERIAL.printf("%02x ", packet[CRC_H]);
    /**/ DEBUG_SERIAL.println(" ");

    // FF FF FD 00 01 09 00 03 74 00 00 02 00 00 CA 89
    // ff ff fd 00 01 09 00 03 74 00 00 02 00 00 ca ca

    // send packet
    dxl_hw_write(packet, sizeof(packet));
}
