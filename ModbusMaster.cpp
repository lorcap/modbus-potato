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
        ,   m_starting_register()
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
                if (m_time_provider->ticks() - m_timer >= m_response_time_out)
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
            ret = read_registers_rsp(framer, true);
            break;
        case function_code::read_input_registers:
            ret = read_registers_rsp(framer, false);
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

        send_and_wait(slave, address, len);
        return true;
    }

    bool CModbusMaster::read_registers_rsp(IFramer* framer, bool holding)
    {
        uint8_t* buffer = framer->buffer();
        (void)          *buffer++;      // function code
        uint8_t count = *buffer++;      // byte count

        if ((count < 2*1) || (count > 2*0x7d))
            return false;
        if (count % 2 != 0)
            return false;

        if (holding)
            return m_handler->read_holding_registers_rsp(m_starting_register, count/2, (uint16_t *) buffer);
        else
            return m_handler->read_input_registers_rsp(m_starting_register, count/2, (uint16_t *) buffer);
    }

    template <typename ITER>
    bool CModbusMaster::write_registers_req(const enum function_code::function_code func, const uint8_t slave, const uint16_t address, const size_t n, const ITER begin, const ITER end)
    {
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
        if (n != 1)
        {
            *buffer++ = (uint8_t) (n >> 8);
            *buffer++ = (uint8_t) n;
            *buffer++ = (uint8_t) 2*n;
        }
        for (ITER i = begin; i != end; ++i)
        {
            *buffer++ = (uint8_t) (*i >> 8);
            *buffer++ = (uint8_t) *i;
        }

        send_and_wait(slave, address, len);
        return true;
    }
    template
    bool CModbusMaster::write_registers_req<const unsigned short*>(const enum function_code::function_code func, const uint8_t slave, const uint16_t address, const size_t n, const unsigned short* begin, const unsigned short* end);

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

    void CModbusMaster::send_and_wait(uint8_t slave, uint16_t address, size_t len)
    {
        // send buffer
        m_framer->set_buffer_len(len);
        m_framer->send();

        // update state
        m_timer = m_time_provider->ticks();
        m_slave_address = slave;
        m_starting_register = address;
        m_state = (slave == 0)
                ? state::waiting_turnaround_reply
                : state::waiting_for_reply;
    }
}
