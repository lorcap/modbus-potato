#ifndef __ModbusPotato_ModbusRTU_h__
#define __ModbusPotato_ModbusRTU_h__
#include "ModbusInterface.h"
namespace ModbusPotato
{
    /// <summary>
    /// This class handles the RTU based protocol for Modbus.
    /// </summary>
    /// <remarks>
    /// See the IFramer interface for a complete description of the public
    /// methods.
    ///
    /// The setup() method must be called with the correct baud rate before
    /// using this class in order to calculate the proper inter-character and
    /// inter-frame delays.
    /// </remarks>
    class CModbusRTU : public IFramer
    {
    public:
        /// <summary>
        /// Constructor for the RTU framer.
        /// </summary>
        CModbusRTU(IStream* stream, ITimeProvider* timer, uint8_t* buffer, size_t buffer_max);

        /// <summary>
        /// Initialize any special values.
        /// </summary>
        /// <remarks>
        /// Notice that this method does NOT setup the serial link (i.e.
        /// Serial.begin(...)).  The baud rate is only needed to calculate
        /// the inter-character delays.
        /// </remarks>
        void setup(unsigned long baud);

        unsigned long poll();
        bool begin_send();
        void send();
        void finished();
        bool idle() const { return m_state == state_idle; }
        bool frame_ready() const { return m_state == state_frame_ready; }
    private:
        enum
        {
            CRC_LEN = 2,
            default_baud_rate = 19200,
            default_3t5_period = 1750, // T3.5 character timeout for high baud rates, in microseconds
            default_1t5_period = 750, // T1.5 character timeout for high baud rates, in microseconds
            minimum_tick_count = 2,
            quantization_rounding_count = 2,
            min_pdu_length = 3, // minimum PDU length, excluding the station address. function code and two crc bytes
        };
        uint16_t m_checksum;
        uint8_t m_buffer_tx_pos;
        enum state_type
        {
            state_exception,
            state_dump,
            state_idle,
            state_frame_ready,
            state_queue,
            state_collision,
            state_receive,
            state_tx_addr,
            state_tx_pdu,
            state_tx_crc,
            state_tx_drain,
            state_tx_wait,
        };
        state_type m_state;
        system_tick_t m_last_ticks;
        system_tick_t m_T3p5, m_T1p5;
    };
}
#endif
