/*
 *  dxl_node_op3.h
 *
 *  dynamixel node op3
 *
 *  Created on: 2016. 10. 21.
 *      Author: Baram
 */

#ifndef DXL_NODE_OP3_H
#define DXL_NODE_OP3_H


#include "../dxl_def.h"

/// @brief The Dynamixel ID of the OpenCR for packet communication
#define DXL_NODE_OP3_ID           200  // 0xC8
#define DXL_NODE_OP3_MODEL_NUMBER 0x7400
#define DXL_NODE_OP3_FW_VER       0x05
/// @brief The factory default Baud Rate
/// @details Value is indexed from {9600, 57600, 115200, 1000000, 2000000, 3000000, 4000000, 4500000}
/// @see dxl_hw_begin(baud) in ../hardware/dxl_hw.cpp
#define DXL_NODE_OP3_BAUD 5

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif


typedef struct {
    uint16_t Model_Number;        // 0
    uint8_t Firmware_Version;     // 2
    uint8_t ID;                   // 3
    uint8_t Baud;                 // 4
    uint8_t Return_Delay_Time;    // 5
    uint8_t Dummy1[10];           // 6
    uint8_t Status_Return_Level;  // 16
    uint8_t Dummy2[1];            // 17
    int16_t Roll_Offset;          // 18
    int16_t Pitch_Offset;         // 20
    int16_t Yaw_Offset;           // 22

    uint8_t Dynamixel_Power;  // 24
    uint8_t LED;              // 25
    uint16_t LED_RGB;         // 26
    uint16_t Buzzer;          // 28
    uint8_t Button;           // 30
    uint8_t Voltage;          // 31
    int16_t Gyro_X;           // 32
    int16_t Gyro_Y;           // 34
    int16_t Gyro_Z;           // 36
    int16_t Acc_X;            // 38
    int16_t Acc_Y;            // 40
    int16_t Acc_Z;            // 42
    int16_t Roll;             // 44
    int16_t Pitch;            // 46
    int16_t Yaw;              // 48
    uint8_t IMU_Control;      // 50

} __attribute__((packed)) dxl_mem_op3_t;


void dxl_node_op3_init(void);
void dxl_node_op3_loop(void);

void dxl_debug_write_byte_wrapper(uint16_t addr, uint8_t data);

#endif
