#include "ModbusMaster.h"

namespace ModbusPotato
{
    CModbusMaster::CModbusMaster(IMasterHandler* handler, IFramer* framer, ITimeProvider* timer, unsigned int response_time_out, unsigned int turnaround_delay)
        :   m_handler(handler)
        ,   m_framer(framer)
        ,   m_time_provider(timer)
        ,   m_state(state::idle)
        ,   m_timer()
        ,   m_slave_address()
        ,   m_read_starting_address()
        ,   m_write_starting_address()
        ,   m_write_n()
    {
         m_response_time_out = response_time_out * 1000
                             / m_time_provider->microseconds_per_tick();
         m_turnaround_delay  = turnaround_delay * 1000
                             / m_time_provider->microseconds_per_tick();
    }

    void CModbusMaster::poll (void)
    {
        switch (m_state)
        {
            case state::idle:
            default:
                break;
            case state::waiting_for_reply:
                if (m_time_provider->ticks() - m_timer <= m_response_time_out)
                    break;
                m_state = m_handler->response_time_out()
                        ? state::idle
                        : state::processing_error;
                break;
            case state::processing_reply:
                break;
            case state::waiting_turnaround_reply:
                if (m_time_provider->ticks() - m_timer >= m_turnaround_delay)
                    m_state = state::idle;
                break;
            case state::processing_error:
                break;
        }
    }

    bool CModbusMaster::read_coils_req(void)
    {
        return false;
    }

    bool CModbusMaster::read_discrete_inputs_req(void)
    {
        return false;
    }

    bool CModbusMaster::write_single_coil_req(void)
    {
        return false;
    }

    bool CModbusMaster::write_multiple_coils_req(void)
    {
        return false;
    }

    void CModbusMaster::frame_ready(IFramer* framer)
    {
        bool ret = false;

        // sanity checks
        if (framer->buffer_len() == 0)
            goto finish;
        if (m_state != state::waiting_for_reply)
            goto finish;

        // unexpected slave
        if (framer->frame_address() != m_slave_address)
        {
            m_timer = m_time_provider->ticks();
            goto finish;
        }

        // handle the function code
        m_state = state::processing_reply;
        switch (framer->buffer()[0])
        {
        case function_code::read_coil_status:
            ret = m_handler->read_coils_rsp(framer);
            break;
        case function_code::read_discrete_input_status:
            m_handler->read_discrete_inputs_rsp(framer);
            break;
        case function_code::read_holding_registers:
            ret = read_registers_rsp(framer, function_code::read_holding_registers);
            break;
        case function_code::read_input_registers:
            ret = read_registers_rsp(framer, function_code::read_input_registers);
            break;
        case function_code::write_single_coil:
            ret = m_handler->write_single_coil_rsp(framer);
            break;
        case function_code::write_single_register:
            ret = write_registers_rsp(framer, true);
            break;
        case function_code::write_multiple_coils:
            ret = m_handler->write_multiple_coils_rsp(framer);
            break;
        case function_code::write_multiple_registers:
            ret = write_registers_rsp(framer, false);
            break;
        case function_code::read_write_multiple_registers:
            ret = read_registers_rsp(framer, function_code::read_write_multiple_registers);
            break;
        default:
            if (framer->buffer()[0] & 0x80)
            {
                m_handler->exception_response((enum modbus_exception_code::modbus_exception_code) framer->buffer()[1]);
                ret = false;
            }
            break;
        }

        m_state = (ret == true) ? state::idle : state::processing_error;

    finish:
        framer->finished();
    }

    bool CModbusMaster::read_registers_req(const enum function_code::function_code func, const uint8_t slave, const uint16_t address, const uint16_t n)
    {
        const size_t len = 1                    // function
                         + 2                    // address
                         + 2;                   // number of data

        if (!sanity_check(n, len))
            return false;

        // make request frame
        m_framer->set_frame_address(slave);
        uint8_t* buffer = m_framer->buffer();
        *buffer++ = (uint8_t) func;
        *buffer++ = (uint8_t) (address >> 8);
        *buffer++ = (uint8_t) (address >> 0);
        *buffer++ = (uint8_t) (n >> 8);
        *buffer++ = (uint8_t) (n >> 0);

        m_read_starting_address = address;
        send_and_wait(slave, len);
        return true;
    }

    bool CModbusMaster::read_registers_rsp(IFramer* framer, const enum function_code::function_code func)
    {
        uint8_t* buffer = framer->buffer();
        (void)          *buffer++;      // function code
        uint8_t count = *buffer++;      // byte count

        if ((count < 2*1) || (count > 2*0x7d))
            return false;
        if (count % 2 != 0)
            return false;

        count /= 2;
        uint16_t *const data = (uint16_t*) buffer;

        if (ntohs(0x00ffu) != 0x00ffu)
        {
            for (int i = 0; i < count; ++i)
                data[i] = ntohs(data[i]);
        }

        switch (func)
        {
        case function_code::read_holding_registers:
            return m_handler->read_holding_registers_rsp(m_read_starting_address, count, data);
            break;
        case function_code::read_input_registers:
            return m_handler->read_input_registers_rsp(m_read_starting_address, count, data);
            break;
        case function_code::read_write_multiple_registers:
            return m_handler->read_write_multiple_registers_rsp(m_read_starting_address, count, data, m_write_starting_address, m_write_n);
            break;
        default:
            return false;
        }
    }

    bool CModbusMaster::write_registers_req(const enum function_code::function_code func, const uint8_t slave, const uint16_t address, const uint16_t* begin, const uint16_t* end)
    {
        const size_t n = end - begin;
        const size_t len = 1                    // function
                         + 2                    // address
                         + (n != 1 ? 2+1 : 0)   // number of data
                         + 2*n;                 // data

        if (!sanity_check(n, len))
            return false;

        // make request frame
        m_framer->set_frame_address(slave);
        uint8_t* buffer = m_framer->buffer();
        *buffer++ = (uint8_t) func;
        *buffer++ = (uint8_t) (address >> 8);
        *buffer++ = (uint8_t) address;
        if (func == function_code::write_multiple_registers)
        {
            *buffer++ = (uint8_t) (n >> 8);
            *buffer++ = (uint8_t) n;
            *buffer++ = (uint8_t) 2*n;
        }
        for (const uint16_t* i = begin; i != end; ++i)
        {
            const uint16_t d = htons(*i);

            *buffer++ = (uint8_t) (d >> 8);
            *buffer++ = (uint8_t) d;
        }

        m_write_starting_address = address;
        send_and_wait(slave, len);
        return true;
    }

    bool CModbusMaster::write_registers_rsp(IFramer* framer, bool single)
    {
        uint8_t* buffer = framer->buffer();
        (void)                         *buffer++;       // function code
        uint16_t address = (uint16_t) (*buffer++ << 8); // starting address
                 address|= (uint16_t) (*buffer++ << 0); //

        if (single)
        {
            return m_handler->write_single_register_rsp(address);
        }
        else {
            size_t n = (size_t) (*buffer++ << 8);       // quantity of registers
                   n|= (size_t) (*buffer++ << 0);       //

            if ((n < 1) || (n > 0x7b))
                return false;

            return m_handler->write_multiple_registers_rsp(address, n);
        }
    }

    bool CModbusMaster::read_write_registers_req(const enum function_code::function_code func, const uint8_t slave, const uint16_t read_address, const uint16_t read_n, const uint16_t write_address, const uint16_t* write_begin, const uint16_t* write_end)
    {
        (void) func;

        const size_t write_n = write_end - write_begin;
        const size_t len = 1            // function code
                         + 2            // read starting address
                         + read_n       // quantity to read
                         + 2            // write starting address
                         + 2            // quantity to write
                         + 1            // write byte count
                         + 2*write_n;   // write registers value

        if (!sanity_check(read_n, len))
            return false;

        // make request frame
        m_framer->set_frame_address(slave);
        uint8_t* buffer = m_framer->buffer();
        *buffer++ = (uint8_t) func;
        *buffer++ = (uint8_t) (read_address >> 8);
        *buffer++ = (uint8_t) (read_address >> 0);
        *buffer++ = (uint8_t) (read_n >> 8);
        *buffer++ = (uint8_t) (read_n >> 0);
        *buffer++ = (uint8_t) (write_address >> 8);
        *buffer++ = (uint8_t) (write_address >> 0);
        *buffer++ = (uint8_t) (write_n >> 8);
        *buffer++ = (uint8_t) (write_n >> 0);
        *buffer++ = (uint8_t) 2*write_n;
        for (const uint16_t* i = write_begin; i != write_end; ++i)
        {
            const uint16_t d = htons(*i);

            *buffer++ = (uint8_t) (d >> 8);
            *buffer++ = (uint8_t) d;
        }

        m_read_starting_address  = read_address;
        m_write_starting_address = write_address;
        m_write_n                = write_n;
        send_and_wait(slave, len);
        return true;
    }

    bool CModbusMaster::sanity_check(const size_t n, const size_t len)
    {
        if (this->m_state != state::idle)
            return false;
        if ((0x0001 < n) && (n > 0x007b))
            return false;
        if (m_framer->buffer_max() < len)
            return false;
        if (!m_framer->begin_send())
            return false;

        return true;
    }

    void CModbusMaster::send_and_wait(uint8_t slave, size_t len)
    {
        // send buffer
        m_framer->set_buffer_len(len);
        m_framer->send();

        // update state
        m_timer = m_time_provider->ticks();
        m_slave_address = slave;
        m_state = (slave == 0)
                ? state::waiting_turnaround_reply
                : state::waiting_for_reply;
    }
}
