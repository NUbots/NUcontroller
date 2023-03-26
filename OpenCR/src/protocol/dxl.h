/*
 * dxl.h
 *
 *  Created on: 2017. 4. 11.
 *      Author: Baram
 */

#ifndef DXL_H_
#define DXL_H_


#include "../dxl_def.h"


#ifdef __cplusplus
extern "C" {
#endif


#define DXL_PACKET_VER_1_0 0
#define DXL_PACKET_VER_2_0 1


#define DXL_STATE_WAIT_INST   0
#define DXL_STATE_WAIT_STATUS 1


#define DXL_TYPE_INST   0
#define DXL_TYPE_STATUS 1


#define DXL_INST_STATUS 0x55

#define DXL_GLOBAL_ID 0xFE


#define DXL_MAX_BUFFER 2048

/**
 * @brief Packet structure in byte order
 * @see https://emanual.robotis.com/docs/en/dxl/protocol2/#packet-parameters
 *
 * Instruction packets only go up to PKT_INST_IDX and then straight to CRC.
 * Status packets have the PKT_ERROR_IDX param and then three additional bytes
 *  starting from PKT_STATUS_PARAM_IDX
 */
#define PKT_HDR_1_IDX 0  // header bytes
#define PKT_HDR_2_IDX 1
#define PKT_HDR_3_IDX 2
#define PKT_RSV_IDX   3  // reserved
#define PKT_ID_IDX    4  // packet ID
#define PKT_LEN_L_IDX 5  // packet length (total incl CRC)
#define PKT_LEN_H_IDX 6
#define PKT_INST_IDX  7  // instruction ID (defined below)
#define PKT_ERROR_IDX 8  // error code - ONLY in status packet, not instruction

#define PKT_STATUS_PARAM_IDX 9

/**
 * @brief instruction ID defines the type of command in a packet
 * @see https://emanual.robotis.com/docs/en/dxl/protocol2/#instruction
 */
#define INST_PING       0x01
#define INST_READ       0x02
#define INST_WRITE      0x03
#define INST_REG_WRITE  0x04
#define INST_ACTION     0x05
#define INST_RESET      0x06
#define INST_REBOOT     0x08
#define INST_STATUS     0x55
#define INST_SYNC_READ  0x82
#define INST_SYNC_WRITE 0x83
#define INST_BULK_READ  0x92
#define INST_BULK_WRITE 0x93

/**
 * @brief Error field is included in Status packets to indicate the processing
 *  result of a corresponding Instruction packet.
 * @details The defines outline the error number, given by bits 0~6 of the error
 *  field. Additionally, bit 7 is the Alert field indicating a hardware error.
 *  The error field becomes 0x40 | ERROR_CODE in this case.
 *  Hardware error status value can we read from the Control Table with a read
 *  instruction.
 */
#define DXL_ERR_NONE        0x00
#define DXL_ERR_RESULT_FAIL 0x01
#define DXL_ERR_INST_ERROR  0x02
#define DXL_ERR_CRC_ERROR   0x03
#define DXL_ERR_DATA_RANGE  0x04
#define DXL_ERR_DATA_LENGTH 0x05
#define DXL_ERR_DATA_LIMIT  0x06
#define DXL_ERR_ACCESS      0x07


#define DXL_PROCESS_INST        0
#define DXL_PROCESS_BROAD_PING  1
#define DXL_PROCESS_BROAD_READ  2
#define DXL_PROCESS_BROAD_WRITE 3


typedef enum {
    DXL_RET_OK,
    DXL_RET_RX_INST,
    DXL_RET_RX_STATUS,
    DXL_RET_EMPTY,
    DXL_RET_PROCESS_BROAD_PING,
    DXL_RET_PROCESS_BROAD_READ,
    DXL_RET_PROCESS_BROAD_WRITE,
    DXL_RET_ERROR_CRC,
    DXL_RET_ERROR_LENGTH,
    DXL_RET_ERROR_NO_ID,
    DXL_RET_ERROR,
    DXL_RET_NO_STATUS_PKT,
    DXL_RET_NOT_REQUIRED
} dxl_error_t;


typedef struct {
    uint8_t header[3];
    uint8_t reserved;
    uint8_t id;
    uint8_t cmd;
    uint8_t error;
    uint8_t type;
    uint16_t index;
    uint16_t packet_length;
    uint16_t param_length;
    uint16_t crc;
    uint16_t crc_received;
    uint8_t* p_param;
    uint8_t data[DXL_MAX_BUFFER];
} dxl_packet_t;


typedef struct {
    dxl_error_t (*ping)(void* p_arg);
    dxl_error_t (*read)(void* p_arg);
    dxl_error_t (*write)(void* p_arg);
    dxl_error_t (*reg_write)(void* p_arg);
    dxl_error_t (*action)(void* p_arg);
    dxl_error_t (*factory_reset)(void* p_arg);
    dxl_error_t (*reboot)(void* p_arg);
    dxl_error_t (*status)(void* p_arg);
    dxl_error_t (*sync_read)(void* p_arg);
    dxl_error_t (*sync_write)(void* p_arg);
    dxl_error_t (*bulk_read)(void* p_arg);
    dxl_error_t (*bulk_write)(void* p_arg);
} dxl_inst_func_t;


typedef struct {
    uint8_t packet_ver;
    int8_t dxlport_ch;
    uint32_t dxlport_baud;
    uint8_t rx_state;
    // IDs we care about while processing packets, because some intrustions
    // require that we wait our turn (in sequential ID order) before returning.
    uint8_t id;         // ID of the OpenCR
    uint8_t current_id; // ID of packet we're waiting on
    uint8_t pre_id;     // ID of the last packet we processed

    uint32_t prev_time;
    uint8_t header_cnt;

    dxl_inst_func_t inst_func;
    dxl_packet_t rx;
    dxl_packet_t tx;
} dxl_t;


bool dxlInit(dxl_t* p_packet, uint8_t protocol_ver);
bool dxlOpenPort(dxl_t* p_packet, uint8_t ch, uint32_t baud);
void dxlAddInstFunc(dxl_t* p_packet, uint8_t inst, dxl_error_t (*func)(dxl_t* p_dxl));
bool dxlSetId(dxl_t* p_packet, uint8_t id);
uint8_t dxlGetId(dxl_t* p_packet);

uint32_t dxlRxAvailable(dxl_t* p_packet);
uint8_t dxlRxRead(dxl_t* p_packet);


dxl_error_t dxlProcessInst(dxl_t* p_packet);
dxl_error_t dxlRxPacket(dxl_t* p_packet);
dxl_error_t dxlRxPacketDataIn(dxl_t* p_packet, uint8_t data_in);

dxl_error_t dxlTxPacketInst(dxl_t* p_packet);
dxl_error_t dxlTxPacketStatus(dxl_t* p_packet, uint8_t id, uint8_t error, uint8_t* p_data, uint16_t length);
dxl_error_t dxlTxPacket(dxl_t* p_packet);
dxl_error_t dxlMakePacketStatus(dxl_t* p_packet, uint8_t id, uint8_t error, uint8_t* p_data, uint16_t length);

/* Moved from being `static` in .c file to allow access from dxl_debug */
void dxlUpdateCrc(uint16_t* p_crc_cur, uint8_t data_in);

#ifdef __cplusplus
}
#endif


#endif /* DXL_H_ */
