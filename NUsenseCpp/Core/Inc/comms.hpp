/*
 * comms.hpp
 *
 *  Created on: 3 Dec. 2023
 *      Author: Johanne Montano
 */

#include "usbd_cdc_if.h"
#include "comms/ServoTarget.pb.h"
#include "comms/pb_encode.h"
#include "comms/pb_decode.h"

#ifndef INC_COMMS_HPP_
#define INC_COMMS_HPP_

/// @brief Container for servo command values
struct ServoMessage {
	uint8_t id;
	float position;
	float gain;
	float torque;
};
/// @brief Array to contain ServoMessage commands
ServoMessage servo_commands[20];
/// @brief Receive flag declared in usb_cdc_if.h - raised if the USB receive callback is triggered
extern uint8_t rx_flag;
/// @brief Receive buffer - contents of UserRxBufferHs are appended into it using memset inside the callback
/// @NOTE if something weird happens its probably because of the memset (blocking function) inside the callback
/// Tests denote that it is fine at the moment. It has a size of RX_BUF_SIZE which is 2048 by default
extern uint8_t rx_buf[];
/// @brief Index accessor for rx_buf
extern uint16_t rx_buf_idx;
/// @brief Length tracker for rx_buf
extern uint16_t rx_buf_len;
/// @brief Length of the received packets from the NUC side.
/// @NOTE Max bulk size of USB2.0 is 512 so uint16 should be enough.
extern uint16_t rx_len;
/// @brief Protobuf packets memcpy from rx_buf
char pb_packets[RX_BUF_SIZE];
/// @brief Flag to raise if a header is found and data is copied to pb_packets from rx_buf
uint8_t packet_fin = 0;
/// @brief The receive subroutine which processes data received from the NUC.
/// @return True if data has been decoded properly, false if it was not decoded properly or data has not been received.
bool receive();
/// @brief @TODO The transmit subroutine that sends servo states to the NUC using the same protocol
//bool transmit();

#endif /* INC_COMMS_HPP_ */
