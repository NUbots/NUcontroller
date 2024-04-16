#include <array>    // needed for the simple tx-buffer
#include <cstring>  // needed for the memcpy
#include <deque>
#include <vector>

#include "RS485.h"   // needed for the RS485 interface
#include "main.h"    // only used for GPIO labels for debugging
#include "stdint.h"  // needed for explicit type-defines

#ifndef SRC_PORT_H_
    #define SRC_PORT_H_

namespace uart {

    #define USE_DMA_RX_BUFFER
    #define SIMPLE_WRITE

    constexpr uint16_t PORT_BUFFER_SIZE = 2048;
    constexpr uint16_t NO_BYTE_READ     = 0xFFFF;

    class Port {
    private:
        /// @brief  the RS485 link
        /// @note   This should be unique for all ports, i.e. only one port for
        ///         each RS485 link.
        RS485 rs_link{};

        /// @brief  a ring buffer for the received data
        struct RingBuffer {
            RingBuffer() {}
            /// @brief  the data of the buffer:
            uint8_t data[PORT_BUFFER_SIZE]{};
            /// @brief  the front of the 'queue' where bytes are read or popped,
            /// @note   This is inclusive of the first byte.
            /// @note   "I have been waiting for so long; I am nearly at the front of the queue."
            volatile uint16_t front = 0;
            /// @brief  the back of the 'queue' where bytes are added or pushed,
            /// @note   This is exclusive of the last byte.
            /// @note   "That rude man just cut in line; he should go at the back of the queue."
            volatile uint16_t back = 0;
            /// @brief  the number of bytes in the ring-buffer,
            volatile uint16_t size = 0;

            /// @brief   Adds one byte to the back of the queue.
            /// @note    This is a helper function; it is not meant to encapsulate anything.
            /// @param   byte the byte to be added,
            /// @return  nothing,

            void push(uint8_t byte) {
                // Move the back backwards (higher) in the array unless there is no more room left.
                if (size < PORT_BUFFER_SIZE) {
                    data[back] = byte;
                    back       = (back + 1) % PORT_BUFFER_SIZE;
                    size++;
                }
            }

            /// @brief   Removes one byte from the front of the queue.
            /// @note    This is a helper function; it is not meant to encapsulate anything.
            /// @return  the byte to be removed,

            uint8_t pop() {
                uint8_t byte = 0xFF;
                // Update the front to move back (higher) in the array unless there
                // is nothing left in the buffer.
                if (size != 0) {
                    byte  = data[front];
                    front = (front + 1) % PORT_BUFFER_SIZE;
                    size--;
                }
                return byte;
            }
        };
        /// @brief  The buffer for RX.
        RingBuffer rx_buffer{};

        /// @brief  The buffer for TX.
        /// @note   Even though the bytes are being directly sent with the simple write, a buffer is
        ///         still needed in the middle for the DMA. One can't just give a reference to a
        ///         temporary variable in the port's scope. I found that this made a bug.
    #ifdef SIMPLE_WRITE
        std::array<uint8_t, UINT16_MAX> tx_buffer{};
    #else
        RingBuffer tx_buffer{};
    #endif

        /// @brief  the number of bytes just transmitted,
        volatile uint16_t num_bytes_tx = 0;

        /// @brief  the received byte:
        uint8_t received_byte = 0;

        /// @brief  the state of the half-duplex channel,
        enum { RX_IDLE = 0, TX_BUSY, TX_DONE } comm_state{};

        /// @brief  the current count of the DMA buffer, i.e. the older contents of the NDTR.
        uint16_t count = 0;

    #ifdef SEE_STATISTICS
        uint16_t old_num_bytes_tx[10];
    #endif

        /// @brief   Begins transmitting all remaining bytes in the tx-buffer,
        /// @note    This should only be called within the class-functions, not outside of it.
        /// @return  the status,
        uint8_t begin_tx();

        /// @brief   Handles the receive-complete interrupt,
        void handle_rx();

        /// @brief   Handles the transmit-complete interrupt,
        void handle_tx();

    public:
        /// @brief   Constructs the port by mapping the number to the corresponding UART interface.
        /// @param   uart_number the number of the corresponding UART interface,
        Port(uint8_t uart_number);

        /// @brief   Destructs the port.
        /// @note    Nothing needs to be freed for now.
        virtual ~Port();

        /// @brief   Gets the number of remaining bytes in the rx-buffer, i.e. the bytes received so far.
        /// @return  the number of remaining bytes in the rx-buffer,
        uint16_t get_available_rx();

        /// @brief   Peeks the next byte from the rx-buffer, i.e. the foremost byte received so far.
        /// @return  the next byte,
        /// @retval  #0xFFFF if there is no byte to read,
        uint16_t peek();

        /// @brief   Pops the next byte from the rx-buffer, i.e. the foremost byte received so far.
        /// @return  the next byte,
        /// @retval  #0xFFFF if there is no byte to read,
        uint16_t read();

        /// @brief   Flushes all the bytes out of the rx-buffer.
        void flush_rx();

        /// @brief   Begins the receival of the first byte.
        /// @note    This should be called only once and before attempting to read the port.
        /// @return  the status,
        /// @retval  #0xFE if the port is already in receiving, i.e. the RX_IDLE state,
        /// @retval  #0xFF if the port is busy transmitting, i.e. the TX_BUSY state,
        uint8_t begin_rx();

        /// @brief   Checks and handles the receive-complete interrupt,
        /// @note    This should be called repeatedly within the context of the reading, i.e. the loop.
        void check_rx();

    #ifndef SIMPLE_WRITE
        /// @brief   Gets the number of unused bytes in the tx-buffer, i.e. the bytes yet to be added.
        /// @return  the number of unused bytes in the tx-buffer,
        uint16_t get_available_tx();

        /// @brief   Pushes the next byte to the tx-buffer, i.e. the next byte to be sent.
        /// @param   data the bytes to be pushed to the buffer,
        /// @param   length the number of bytes,
        /// @return  the number of bytes pushed,
        const uint16_t write(const uint8_t* data, const uint16_t length);

        /// @brief   Flushes all the bytes out of the tx-buffer, i.e. to send all remaining bytes.
        void flush_tx();
    #else
        /// @brief   Copies all bytes to a buffer and then transmits those copied bytes in the buffer through DMA.
        /// @note    This function bypasses the circular buffer completely and just transmits all bytes
        ///          together in a basic buffer instead. This was to temporarily fix a bug with the
        ///          RS485 where the circular buffer was splitting a Dynamixel packet in two.
        /// @param   data the bytes to be pushed to the buffer,
        /// @param   length the number of bytes,
        /// @return  the number of bytes pushed,
        const uint16_t write(const uint8_t* data, const uint16_t length) {
            // Transmit everything at once.
            std::memcpy(tx_buffer.data(), data, length);
            while (rs_link.transmit(tx_buffer.data(), length))
                ;
            comm_state = TX_BUSY;
            // return length;
            return 0xFFFF;
        }
    #endif

        /// @brief   Pushes the object, e.g. a packet, as bytes to the  tx-buffer, i.e. the next byte to
        ///          be sent.
        /// @note    If the simple-write is defined, then this function bypasses the tx-buffer
        ///          completely and just transmits all bytes together. This is to temporarily fix a bug
        ///          with the RS485 and with splitting a Dynamixel packet in two.
        /// @param   data the object to be casted as bytes and then pushed to the buffer
        /// @return  the number of bytes pushed,
        template <typename T>
        const uint16_t write(const T& data) {
            return write(reinterpret_cast<const uint8_t*>(&data), sizeof(T));
        }

        /// @brief   Checks and handles the transmit-complete interrupt,
        /// @note    This should be called repeatedly within the context of the writing, i.e. the loop.
        void check_tx();
    };

}  // namespace uart

#endif  // SRC_PORT_H_
