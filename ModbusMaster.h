#include <initializer_list>
#include <iterator>
#include "ModbusInterface.h"
namespace ModbusPotato
{
    /// <summary>
    /// This class implements a basic Modbus master interface.
    /// </summary>
    class CModbusMaster : public IFrameHandler
    {
    public:
        CModbusMaster(IMasterHandler* handler, IFramer* framer, ITimeProvider* timer, unsigned int response_time_out = 200, unsigned int turnaround_delay = 1000);

        bool read_coils_req(void);
        bool read_discrete_inputs_req(void);
        inline bool read_holding_registers_req(const uint8_t slave, const uint16_t address, const uint16_t n)
        {
            return read_registers_req(function_code::read_holding_registers, slave, address, n);
        }
        inline bool read_input_registers_req(const uint8_t slave, const uint16_t address, const uint16_t n)
        {
            return read_registers_req(function_code::read_input_registers, slave, address, n);
        }

        bool write_single_coil_req(void);
        inline bool write_single_register_req(const uint8_t slave, const uint16_t address, const uint16_t value)
        {
            const std::initializer_list<uint16_t> data = {value};
            return write_registers_req(function_code::write_single_register, slave, address, data.begin(), data.end());
        }
        bool write_multiple_coils_req(void);
        inline bool write_multiple_registers_req(const uint8_t slave, const uint16_t address, const std::initializer_list<uint16_t> data)
        {
            return write_multiple_registers_req(slave, address, data.begin(), data.end());
        }
        inline bool write_multiple_registers_req(const uint8_t slave, const uint16_t address, const size_t n, const uint16_t data[])
        {
            return write_multiple_registers_req(slave, address, &data[0], &data[n]);
        }
        inline bool write_multiple_registers_req(const uint8_t slave, const uint16_t address, const uint16_t* begin, const uint16_t* end)
        {
            return write_registers_req(function_code::write_multiple_registers, slave, address, begin, end);
        }
        inline bool read_write_multiple_registers_req(const uint8_t slave, const uint16_t address, const std::initializer_list<uint16_t> data)
        {
            return read_write_multiple_registers_req(slave, address, data.size(), address, data.begin(), data.end());
        }
        inline bool read_write_multiple_registers_req(const uint8_t slave, const uint16_t read_address, const size_t read_n, const uint16_t write_address, const std::initializer_list<uint16_t> write_data)
        {
            return read_write_multiple_registers_req(slave, read_address, read_n, write_address, write_data.begin(), write_data.end());
        }
        inline bool read_write_multiple_registers_req(const uint8_t slave, const uint16_t address, const size_t n, const uint16_t data[])
        {
            return read_write_multiple_registers_req(slave, address, n, address, &data[0], &data[n]);
        }
        inline bool read_write_multiple_registers_req(const uint8_t slave, const uint16_t read_address, const size_t read_n, const uint16_t write_address, const size_t write_n, const uint16_t write_data[])
        {
            return read_write_multiple_registers_req(slave, read_address, read_n, write_address, &write_data[0], &write_data[write_n]);
        }
        inline bool read_write_multiple_registers_req(const uint8_t slave, const uint16_t address, const uint16_t* begin, const uint16_t* end)
        {
            return read_write_multiple_registers_req(slave, address, end - begin, address, begin, end);
        }
        inline bool read_write_multiple_registers_req(const uint8_t slave, const uint16_t read_address, const uint16_t read_n, const uint16_t write_address, const uint16_t* write_begin, const uint16_t* write_end)
        {
            return read_write_registers_req(function_code::read_write_multiple_registers, slave, read_address, read_n, write_address, write_begin, write_end);
        }

        void poll(void);

        void frame_ready(IFramer* framer) override;

    private:
        enum class state {
                idle,
                waiting_for_reply,
                processing_reply,
                waiting_turnaround_reply,
                processing_error,
        };

        IMasterHandler* m_handler;
        IFramer* m_framer;
        ITimeProvider* m_time_provider;
        system_tick_t m_response_time_out;
        system_tick_t m_turnaround_delay;

        enum state m_state;
        system_tick_t m_timer;
        uint16_t m_slave_address;
        uint16_t m_read_starting_address;
        uint16_t m_write_starting_address;
        uint16_t m_write_n;

        bool read_registers_req(const enum function_code::function_code func, const uint8_t slave, const uint16_t address, const uint16_t n);
        bool read_registers_rsp(IFramer* framer, const enum function_code::function_code func);

        bool write_registers_req(const enum function_code::function_code func, const uint8_t slave, const uint16_t address, const uint16_t* begin, const uint16_t* end);
        bool write_registers_rsp(IFramer* framer, bool single);

        bool read_write_registers_req(const enum function_code::function_code func, const uint8_t slave, const uint16_t read_address, const uint16_t read_n, const uint16_t write_address, const uint16_t* write_begin, const uint16_t* write_end);

        bool sanity_check(const size_t n, const size_t len);
        void send_and_wait(uint8_t slave, size_t len);
};
}
