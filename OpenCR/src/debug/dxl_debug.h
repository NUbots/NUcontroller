/*
 *  dxl_debug.h
 *
 *  debug node op3
 *
 *  Created on: 2017. 2. 2.
 *      Author: Baram
 */

#ifndef DXL_DEBUG_H
#define DXL_DEBUG_H


#include "../dxl_def.h"
#include "../protocol/dxl.h"


#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

extern uint8_t debug_state;

void dxl_debug_init(void);
void dxl_debug_loop(void);

extern void dxlUpdateCrc(uint16_t* p_crc_cur, uint8_t data_in);

typedef struct {
    uint8_t header1     = 0xFF;
    uint8_t header2     = 0xFF;
    uint8_t header3     = 0xFD;
    uint8_t reserved    = 0x00;
    uint8_t id          = 0;
    uint8_t len_l       = 0x09;
    uint8_t len_h       = 0x00;
    uint8_t instruction = 0x03;
    uint8_t addr_l      = 0;
    uint8_t addr_h      = 0;
    uint8_t data1       = 0;
    uint8_t data2       = 0;
    uint8_t data3       = 0;
    uint8_t data4       = 0;
    uint8_t crc_l       = 0;
    uint8_t crc_h       = 0;
} __attribute__((packed)) dxl_debug_write_packet_t;

#endif
