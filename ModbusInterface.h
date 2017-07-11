// C++ Interface definitions for Modbus library
//
// See also:
// http://www.modbus.org/docs/Modbus_Application_Protocol_V1_1b3.pdf
// http://www.modbus.org/docs/Modbus_over_serial_line_V1_02.pdf
// http://www.modbus.org/docs/PI_MBUS_300.pdf
//
#ifndef __ModbusPotato_Interface_h__
#define __ModbusPotato_Interface_h__
#include <stddef.h>
#include "ModbusTypes.h"
namespace ModbusPotato
{
#ifdef ARDUINO
#ifndef htons
    static inline uint16_t htons(uint16_t value) { return (value << 8) | (value >> 8); }
#endif
#else
#ifndef htons
    static inline uint16_t htons(uint16_t value) { return (value << 8) | (value >> 8); }
#endif
#ifndef ntohs
    static inline uint16_t ntohs(uint16_t value) { return (value << 8) | (value >> 8); }
#endif
#endif

    // forward declarations
    class IFramer;

    /// <summary>
    /// Represents an endpoint that can read or write characters.
    /// </summary>
    class IStream
    {
    public:
        virtual ~IStream() {}

        /// <summary>
        /// Read some characters without blocking.
        /// </summary>
        /// <returns>
        /// The number of characters read, or a -1 for a communications error.
        /// </returns>
        /// <remarks>
        /// If there is a parity or framing error in any of the characters,
        /// this function should dump any remaining characters in the input
        /// buffer and return -1.  If no data is in the buffer then this
        /// function must return 0.
        ///
        /// If the buffer pointer is NULL then any characters in the input
        /// buffer must be dumped without storing them.  If any characters are
        /// dumped then this function must return a non-zero value, preferably
        /// the number of characters dumped.
        ///
        /// If the buffer_size parameter is 0, then this function should
        /// immediately return 0.  If the value is (size_t)-1 and the buffer
        /// is NULL then this function must dump all remaining characters.
        /// </remarks>
        virtual int read(uint8_t* buffer, size_t buffer_size) = 0;

        /// <summary>
        /// Sends the given characters without blocking.
        /// </summary>
        /// <returns>
        /// The number of characters written, or -1 if communications exception.
        /// </returns>
        /// <remarks>
        /// This function should only return a negative value in fatal
        /// situations, in which the framer is expected to shut down.
        /// </remarks>
        virtual int write(uint8_t* buffer, size_t len) = 0;

        /// <summary>
        /// Enables or disables the RS-485 transmitter.
        /// </summary>
        /// <remarks>
        /// Ideally the receiver should be disabled when the transmitter is
        /// enabled, but this is not mandatory.
        /// </remarks>
        virtual void txEnable(bool state) = 0;

        /// <summary>
        /// Indicates if all of the characters have been written.
        /// </summary>
        virtual bool writeComplete() = 0;

        /// <summary>
        /// Provides the user some indication that a frame is being sent or received.
        /// </summary>
        /// <remarks>
        /// This is to be used as an indication only of the receive and
        /// transmit status, and may not accurately reflect the state of the
        /// data lines.
        /// </remarks>
        virtual void communicationStatus(bool rx, bool tx) = 0;
    };

    /// <summary>
    /// Provides access to the system tick clock.
    /// </summary>
    class ITimeProvider
    {
    public:
        virtual ~ITimeProvider() {}

        /// <summary>
        /// Returns the number of ticks since an arbitrary epoch point
        /// </summary>
        /// <remarks>
        /// The clock must increment monotomically and must roll over at the
        /// maximum size of system_tick_t to 0.  system_tick_t must be an
        /// unsigned integer of an arbitrary size.
        ///
        /// This is in this interface to aid in unit testing.
        /// </remarks>
        virtual system_tick_t ticks() const = 0;

        /// <summary>
        /// Returns the number of microseconds per tick.
        /// </summary>
        /// <remarks>
        /// This is in this interface to aid in unit testing.
        /// </remarks>
        virtual unsigned long microseconds_per_tick() const = 0;
    };

    /// <summary>
    /// Handles frame requests from the framer.
    /// </summary>
    class IFrameHandler
    {
    public:
        virtual ~IFrameHandler() {}

        /// <summary>
        /// Called when a new frame has been received by the remote.
        /// </summary>
        virtual void frame_ready(IFramer* framer) = 0;
    };

    /// <summary>
    /// This interface implements the framing protocol for Modbus.
    /// </summary>
    /// <remarks>
    /// To use this object, the setup() method must be called to setup the
    /// time-outs, and on slaves the address must be set using the
    /// set_station_address() method.  The frame received callback must also
    /// be set using the set_frame_ready_callback() method if the application
    /// layer requires it.
    /// </remarks>
    class IFramer
    {
    public:
        IFramer(IStream* stream, ITimeProvider* timer, uint8_t* buffer, size_t buffer_max)
            :   m_stream(stream)
            ,   m_timer(timer)
            ,   m_buffer(buffer)
            ,   m_buffer_max(buffer_max)
            ,   m_buffer_len()
            ,   m_handler()
            ,   m_station_address()
            ,   m_frame_address()
        {}

        virtual ~IFramer() {}

        /// <summary>
        /// Sets the handler interface for various events.
        /// </summary>
        void set_handler(IFrameHandler* handler)
        {
                m_handler = handler;
        }

        /// <summary>
        /// Returns the station address.
        /// </summary>
        /// <remarks>
        /// This is the local slave address, not the address of the received or
        /// transmitted slave.  If this is 0 then all addresses will be matched.
        /// </remarks>
        uint8_t station_address() const
        {
                return m_station_address;
        }

        /// <summary>
        /// Sets the station address.
        /// </summary>
        /// <remarks>
        /// This is the local slave address, not the address of the received or
        /// transmitted slave.  If this is 0 then all addresses will be matched.
        ///
        /// This must be set to 0 on the master, and set to the slave address
        /// on slaves.
        /// </remarks>
        void set_station_address(uint8_t address)
        {
                m_station_address = address;
        }

        /// <summary>
        /// Handles any timeouts and transfers more data as needed.
        /// </summary>
        /// <returns>
        /// The next timeout, in system ticks, or 0 if none.
        /// </returns>
        /// <remarks>
        /// This method can be called repeatedly in the loop() statement.  It
        /// performs all the actual reads and writes to the output device.
        ///
        /// In an interrupt driven system, it must be called after a new
        /// character is available, the transmitter is ready to send more data,
        /// the transmission has completed or after the previously returned
        /// timeout has elapsed.
        ///
        /// This function must be called again after any function call that may
        /// change the state.  Any prior timeout, if still pending, must be
        /// cancelled and replaced with the new one returned by this function.
        /// </remarks>
        virtual unsigned long poll() = 0;

        /// <summary>
        /// Places the state machine into the transmitting state to reserve the
        /// data buffer.
        /// </summary>
        /// <returns>
        /// true if the buffer() is available, false if the state machine is busy.
        /// </returns>
        /// <remarks>
        /// If data reception is already in progress, this method will fail and
        /// return false.  The return result must be checked to ensure that the
        /// application does not over-write incoming data.
        ///
        /// After a new transmission is started, it must be completed either by
        /// calling the send() method to send the data or the finished() method
        /// to abort the transmission.
        /// </remarks>
        virtual bool begin_send() = 0;

        /// <summary>
        /// Begin transmission of the buffer to the given address.
        /// </summary>
        /// <remarks>
        /// Before calling send(), the data buffer must be reserved using the
        /// begin_send() method.  If any data is received while the application
        /// has the buffer locked, the information in the buffer may be
        /// discarded.

        /// The poll() method must also be invoked with the rules listed in the
        /// remarks after calling this method.
        /// </remarks>
        virtual void send() = 0;

        /// <summary>
        /// Aborts any pending response and returns the state machine to the
        /// idle state.
        /// </summary>
        /// <remarks>
        /// The poll() method must also be invoked with the rules listed in the
        /// remarks after calling this method.
        /// </remarks>
        virtual void finished() = 0;

        /// <summary>
        /// Indicates that a frame has been received for our station address or broadcast.
        /// </summary>
        /// <remarks>
        /// If this returns true, the buffer must be released using the
        /// finished() method, or starting a new transmission using
        /// begin_send() and following the respective process.
        /// </remarks>
        virtual bool frame_ready() const = 0;

        /// <summary>
        /// Returns the station address for the PDU, or 0 if broadcast or point-to-point.
        /// </summary>
        /// <remarks>
        /// This is the address of the received or transmitted frame, not the
        /// local address of the slave.  See station_address() for the local
        /// slave address.
        /// </remarks>
        uint8_t frame_address() const
        {
                return m_frame_address;
        }

        /// <summary>
        /// Sets the station address for the PDU, or 0 if broadcast or point-to-point.
        /// </summary>
        /// <remarks>
        /// This is the address of the received or transmitted frame, not the
        /// local address of the slave.  See set_station_address() for the
        /// local slave address.
        /// </remarks>
        void set_frame_address(uint8_t address)
        {
                m_frame_address = address;
        }

        /// <summary>
        /// Gets the PDU buffer pointer.
        /// </summary>
        /// <remarks>
        /// Regardless of the protocol used, the first byte will always be the
        /// function code, and the bytes that follow are the data bytes.
        ///
        /// The station address and checksum bytes are not included in this
        /// buffer.
        /// </remarks>
        uint8_t* buffer()
        {
                return m_buffer;
        }

        /// <summary>
        /// Returns the number of data bytes in the PDU buffer.
        /// </summary>
        /// <remarks>
        /// This value includes the function code byte, and data bytes, but
        /// excludes the station address and checksum bytes.
        /// </remarks>
        size_t buffer_len() const
        {
                return m_buffer_len;
        }

        /// <summary>
        /// Sets the length of the buffer.
        /// </summary>
        /// <remarks>
        /// This value includes the function code byte, and data bytes, but
        /// excludes the station address and checksum bytes.
        /// </remarks>
        void set_buffer_len(size_t len)
        {
                m_buffer_len = len;
        }

        /// <summary>
        /// Returns the maximum allowable length of the buffer.
        /// </summary>
        size_t buffer_max() const
        {
                return m_buffer_max;
        }

    protected:
        IStream* m_stream;
        ITimeProvider* m_timer;
        IFrameHandler* m_handler;
        uint8_t m_station_address, m_frame_address;
        uint8_t* m_buffer;
        size_t m_buffer_len, m_buffer_max;
    };

    /// <summary>
    /// The interface to be implemented by the user application for handling slave requests.
    /// </summary>
    /// <remarks>
    /// This is used by the ISlave interface to execute the user code.
    ///
    /// The 'address' parameter found in each of these methods is the raw
    /// address number, not the modbus register number.  For example, a value
    /// of 0 on read_holding_registers means the first holding register, which
    /// modbus register 40001.
    ///
    /// Each function should return modbus_exception_code::ok if the request
    /// executed successfully, or an appropriate error code if not.  This error
    /// code will be returned as the one-byte response to the master.
    ///
    /// The address and count should be validated before processing the
    /// request.  If either is found to be invalid, then the handler should
    /// return modbus_exception_code::illegal_data_address.
    ///
    /// If the address and count are valid, but an error occurs when handling
    /// the command, then the handler should return
    /// modbus_exception_code::server_device_failure.
    ///
    /// </remarks>
    class ISlaveHandler
    {
    public:
        virtual ~ISlaveHandler() {}

        /// <summary>
        /// Handles Modbus function 0x01: Read Coils.
        /// </summary>
        /**
         * @param address  register start address
         * @param n        number of registers
         * @param values   values to store
         * @return Modbus exception code, if any
         */
        virtual modbus_exception_code::modbus_exception_code read_coils(uint16_t, uint16_t, uint8_t*) {
            return modbus_exception_code::illegal_function;
        }

        /// <summary>
        /// Handles Modbus function 0x02: Read Discrete Inputs.
        /// </summary>
        /**
         * @param address  register start address
         * @param n        number of registers
         * @param values   values to store
         * @return Modbus exception code, if any
         */
        virtual modbus_exception_code::modbus_exception_code read_discrete_inputs(uint16_t, uint16_t, uint8_t*) {
            return modbus_exception_code::illegal_function;
        }

        /// <summary>
        /// Handles Modbus function 0x03: Read Holding Registers.
        /// </summary>
        /**
         * @param address  register start address
         * @param n        number of registers
         * @param values   values to store
         * @return Modbus exception code, if any
         */
        virtual modbus_exception_code::modbus_exception_code read_holding_registers(uint16_t, uint16_t, uint16_t*) {
            return modbus_exception_code::illegal_function;
        }

        /// <summary>
        /// Handles Modbus function 0x04: Read Input Registers.
        /// </summary>
        /**
         * @param address  register start address
         * @param n        number of registers
         * @param values   values to store
         * @return Modbus exception code, if any
         */
        virtual modbus_exception_code::modbus_exception_code read_input_registers(uint16_t, uint16_t, uint16_t*) {
            return modbus_exception_code::illegal_function;
        }

        /// <summary>
        /// Handles Modbus function 0x05: Write Single Coil.
        /// </summary>
        /**
         * @param address  register start address
         * @param value    value to write
         * @return Modbus exception code, if any
         */
        virtual modbus_exception_code::modbus_exception_code write_single_coil(uint16_t address, bool value) {
            uint8_t tmp = value ? 1 : 0; return write_multiple_coils(address, 1, &tmp);
        }

        /// <summary>
        /// Handles Modbus function 0x06: Write Single Register.
        /// </summary>
        /**
         * @param address  register start address
         * @param value    value to write
         * @return Modbus exception code, if any
         */
        virtual modbus_exception_code::modbus_exception_code write_single_register(uint16_t address, uint16_t value) {
            return write_multiple_registers(address, 1, &value);
        }

        /// <summary>
        /// Handles Modbus function 0x0F: Write Multiple Coils.
        /// </summary>
        /**
         * @param address  register start address
         * @param n        number of registers
         * @param values   values to write
         * @return Modbus exception code, if any
         */
        virtual modbus_exception_code::modbus_exception_code write_multiple_coils(uint16_t, uint16_t, const uint8_t*) {
            return modbus_exception_code::illegal_function;
        }

        /// <summary>
        /// Handles Modbus function 0x10: Write Multiple registers.
        /// </summary>
        /**
         * @param address  register start address
         * @param n        number of registers
         * @param values   values to write
         * @return Modbus exception code, if any
         */
        virtual modbus_exception_code::modbus_exception_code write_multiple_registers(uint16_t, uint16_t, const uint16_t*) {
            return modbus_exception_code::illegal_function;
        }
    };

    /// <summary>
    /// The interface to be implemented by the user application for handling master requests.
    /// </summary>
    class IMasterHandler
    {
    public:
        virtual ~IMasterHandler() {}

        /// <summary>
        /// Handles Modbus function 0x01: Read Coils.
        /// </summary>
        /**
         * @param framer  frame handler
         * @return true, when successful, false otherwise
         */
        virtual bool read_coils_rsp(IFramer*) {
            return true;
        }

        /// <summary>
        /// Handles Modbus function 0x02: Read Discrete Inputs.
        /// </summary>
        /**
         * @param framer  frame handler
         * @return true, when successful, false otherwise
         */
        virtual bool read_discrete_inputs_rsp(IFramer*) {
            return true;
        }

        /// <summary>
        /// Handles Modbus function 0x03: Read Holding Registers.
        /// </summary>
        /**
         * @param address  register start address
         * @param n        number of registers
         * @param values   values to store
         * @return true, when successful, false otherwise
         */
        virtual bool read_holding_registers_rsp(uint16_t, size_t, const uint16_t*) {
            return true;
        }

        /// <summary>
        /// Handles Modbus function 0x04: Read Input Register.
        /// </summary>
        /**
         * @param address  register start address
         * @param n        number of registers
         * @param values   values to store
         * @return true, when successful, false otherwise
         */
        virtual bool read_input_registers_rsp(uint16_t, size_t, const uint16_t*) {
            return true;
        }

        /// <summary>
        /// Handles Modbus function 0x05: Write Single Coil.
        /// </summary>
        /**
         * @param framer  frame handler
         * @return true, when successful, false otherwise
         */
        virtual bool write_single_coil_rsp(IFramer*) {
            return true;
        }

        /// <summary>
        /// Handles Modbus function 0x06: Write Single Register.
        /// </summary>
        /**
         * @param address  register address
         * @return true, when successful, false otherwise
         */
        virtual bool write_single_register_rsp(uint16_t) {
            return true;
        }

        /// <summary>
        /// Handles Modbus function 0x0F: Write Multiple Coils.
        /// </summary>
        /**
         * @param framer  frame handler
         * @return true, when successful, false otherwise
         */
        virtual bool write_multiple_coils_rsp(IFramer*) {
            return true;
        }

        /// <summary>
        /// Handles Modbus function 0x10: Write Multiple registers.
        /// </summary>
        /**
         * @param address  register start address
         * @param n        number of registers
         * @return true, when successful, false otherwise
         */
        virtual bool write_multiple_registers_rsp(uint16_t, size_t) {
            return true;
        }

        /// <summary>
        /// Handles Modbus function 0x17: Read Write Multiple Registers.
        /// </summary>
        /**
         * @param read_address   read start address
         * @param read_n         number of read registers
         * @param read_values    read values to store
         * @param write_address  write start address
         * @param write_n        number of written registers
         * @return true, when successful, false otherwise
         */
        virtual bool read_write_multiple_registers_rsp(uint16_t read_address, size_t read_n, const uint16_t* read_values, uint16_t write_address, size_t write_n) {
            return this->write_multiple_registers_rsp(write_address, write_n)
                && this->read_holding_registers_rsp(read_address, read_n, read_values);
        }

        /// <summary>
        /// Slave didn't respond on time.
        /// </summary>
        virtual bool response_time_out(void) {
            return true;
        }

        /// <summary>
        /// Slave responded with an exception.
        /// </summary>
        /**
         * @param code  exception code
         * @return
         */
        virtual bool exception_response(enum modbus_exception_code::modbus_exception_code) {
            return true;
        }
    };
}
#endif
