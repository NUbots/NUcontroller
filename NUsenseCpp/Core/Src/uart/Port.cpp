#include "Port.hpp"

namespace uart {

    Port::Port(uint8_t uart_number = 1) :
        rs_link(uart_number),
        num_bytes_tx(0),
        comm_state(RX_IDLE)
    {

    }

    Port::~Port() {
        // May do something with this later.
        // Not really anything to free as of yet.
    }

    uint16_t Port::get_available_rx() {
    #ifdef USE_DMA_RX_BUFFER
        handle_rx();
    #endif
        return rx_buffer.size;
    }

    #ifndef SIMPLE_WRITE
    uint16_t Port::get_available_tx() {
    #ifdef USE_QUEUE_CLASS
        return PORT_BUFFER_SIZE - tx_buffer.size();
    #else
        return PORT_BUFFER_SIZE - tx_buffer.size;
    #endif
    }
    #endif

    uint16_t Port::peek() {
        uint8_t read_byte;

    #ifdef USE_DMA_RX_BUFFER
        handle_rx();
    #endif

        // If there is no byte to read, then return 0xFFFF as a value two bytes long so that is not to 
        // be confused with a received byte.
        if (!get_available_rx())
            return 0xFFFF;
        
        // Read from the front of the buffer.
        read_byte = rx_buffer.data[rx_buffer.front];

        return read_byte;
    }

    uint16_t Port::read() {
        uint8_t read_byte;

    #ifdef USE_DMA_RX_BUFFER
        //handle_rx();
    #endif

        // If there is no byte to read, then return 0xFFFF as a value two bytes long so that is not to 
        // be confused with a received byte.
        if (!get_available_rx())
            return 0xFFFF;

        // Read from the front of the buffer.
        read_byte = rx_buffer.pop();

        return read_byte;
    }

    void Port::flush_rx() {
        // Just reset the buffer.
        rx_buffer.front = rx_buffer.back;
        rx_buffer.size = 0;
    }

    uint8_t Port::begin_rx() {
    #ifdef USE_DMA_RX_BUFFER
        // Begin the receival of all bytes into a circular buffer.
        return (uint8_t)rs_link.receive(rx_buffer.data, PORT_BUFFER_SIZE);
    #else
        // Begin the receival of a byte only if there is no transmission.
        switch (comm_state) {
        case TX_DONE:
            comm_state = RX_IDLE;
            HAL_GPIO_WritePin(SPARE1_GPIO_Port, SPARE1_Pin, GPIO_PIN_SET);
            return (uint8_t)rs_link.receive(&received_byte, 1);
        case RX_IDLE:
            return 0xFE;
        case TX_BUSY:
        default:
            return 0xFF;
        }
    #endif
    }

    void Port::handle_rx() {
    #ifdef USE_DMA_RX_BUFFER
        uint16_t old_back, count;
        // Update the back of the buffer.
        old_back = rx_buffer.back;
        count = rs_link.get_receive_counter();
        rx_buffer.back = (PORT_BUFFER_SIZE - count) % PORT_BUFFER_SIZE;
        rx_buffer.size +=
                rx_buffer.back >= old_back ?
                rx_buffer.back - old_back :
                rx_buffer.back + (PORT_BUFFER_SIZE - old_back);
        // Handle if the buffer has overflowed. This should be very unlikely, and if it has happened, 
        // then something seriously bad has happened at the protocol-handling level! If this happens, 
        // then buffer may be unusable since the DMA may still be updating further down this function 
        // when the front is popped.
        if (rx_buffer.size > PORT_BUFFER_SIZE) {
            rx_buffer.front = rx_buffer.back;
            rx_buffer.size = PORT_BUFFER_SIZE;
        }
    #else
        // If the buffer is not full yet, then add the received byte to it.
        if (get_available_rx() < (PORT_BUFFER_SIZE)) {
            rx_buffer.push(received_byte);
        }
        
        // Reset the comm-state to trigger another receival.
        comm_state = TX_DONE;
        // Begin the receiving again for the next byte.
        // Even if the buffer is full, one would still want to receive the next
        // byte in case that the buffer frees up until then.
        begin_rx();
    #endif
    }

    void Port::check_rx() {
        // If there has been a receival, then handle it.
        if (rs_link.get_receive_flag())
            handle_rx();
    }

    #ifndef SIMPLE_WRITE
    const uint16_t Port::write(const uint8_t* data, const uint16_t length) {
        // For each byte, try to add it to the buffer.
        for (int i = 0; i < length; i++) {
            // If it is full, then begin a transmission if not already begun.
            if (!get_available_tx()) {
                while (!num_bytes_tx)
                    begin_tx();
                // Wait for a spot in the buffer to show up.
                while (!get_available_tx())
                    // This has to be done inside this loop lest it freezes here. The alternative to 
                    // this is to do what Robotis did in their UARTClass, that is to do the handling 
                    // inside the callback-function itself. However, I think that this is inefficient 
                    // and has caused bugs when I have tested it.
                    check_tx();
            }
            // Add the byte once there is room.
            tx_buffer.push(data[i]);
        }
        // Begin a transmission if not already begun.
        if (!num_bytes_tx) {
            // Keep trying if there is an error.
            while (begin_tx());
        }
        //return length;
        return 0xFFFF;
    }

    void Port::flush_tx() {
        // Wait until there are no bytes transmitting.
        //while (!num_bytes_tx)
        while (comm_state == TX_BUSY)
            check_tx();
    }

    uint8_t Port::begin_tx() {
        RS485::status status = RS485::RS485_OK;

        // Transmit the bytes after you have calculated how many are to be
        // transmitted.
    #ifdef USE_QUEUE_CLASS
        num_bytes_tx = tx_buffer.size();
        bytes_tx = tx_buffer;
        // If there is an error, etc., then set the number of bytes to zero.
        if (RS485::RS485_OK != rs_link.transmit_it((uint8_t*)bytes_tx.data(), num_bytes_tx)) {
            num_bytes_tx = 0;
            comm_state = TX_DONE;
        } else
            comm_state = TX_BUSY;
    #else
        // Only send from the front to the end of the actual array.
        // This is less efficient than using a C++ queue.
        num_bytes_tx = tx_buffer.back >= tx_buffer.front ?
                tx_buffer.size
                : PORT_BUFFER_SIZE - tx_buffer.front;
        // If there is an error, etc., then set the number of bytes to zero to say
        // that none have been sent.
        status = rs_link.transmit(&tx_buffer.data[tx_buffer.front], num_bytes_tx);
        if (RS485::RS485_OK != status) {
            num_bytes_tx = 0;
            comm_state = TX_DONE;
        } else
            comm_state = TX_BUSY;
    #endif

    #ifdef SEE_STATISTICS
        for (int i = 0; i < 10-1; i++)
            old_num_bytes_tx[i] = old_num_bytes_tx[i+1];
        old_num_bytes_tx[9] = num_bytes_tx;
    #endif

        return status;
    }

    void Port::handle_tx() {
    #ifdef USE_QUEUE_CLASS
        // Remove the bytes that were just transmitted from the front of the buffer.
        tx_buffer.erase(tx_buffer.begin(),tx_buffer.begin()+num_bytes_tx);
        // Send any remaining bytes.
        if (tx_buffer.size() != 0)
            begin_tx();
        else
            num_bytes_tx = 0;
    #else
        // Move the front further backwards.
        tx_buffer.front = (tx_buffer.front + num_bytes_tx) % PORT_BUFFER_SIZE;
        // Update the size.
        tx_buffer.size -= num_bytes_tx;
        // Update the number of bytes to be transmitted.
        num_bytes_tx = 0;
        // Send any remaining bytes.
        if (tx_buffer.size)
            begin_tx();
        // If not, then set to done.
        else
            comm_state = TX_DONE;
    #endif
    }
    #endif

    void Port::check_tx() {
        // If the transmission has been done, then handle it.
        if (rs_link.get_transmit_flag())
    #ifndef SIMPLE_WRITE
            handle_tx();
    #else
            comm_state = TX_DONE;
    #endif
    }

} // namespace uart
