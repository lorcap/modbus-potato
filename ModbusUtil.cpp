#include "ModbusUtil.h"
namespace ModbusPotato
{

size_t
pdu_len (uint8_t station_address,
         uint8_t* buffer,
         size_t buffer_len)
{
    return (station_address != 0)
         ? pdu_len_req(buffer, buffer_len)
         : pdu_len_rsp(buffer, buffer_len);
}

/* ----------------------------------------------------------------------- */

size_t
pdu_len_req (uint8_t* buffer,
             size_t buffer_len)
{
    if (buffer_len == 0)
        return -1;

    const auto func = static_cast<function_code::function_code>(buffer[0]);

    size_t pos = -1;
    switch (func)
    {
    case function_code::read_coil_status             : pos = -1; break;
    case function_code::read_discrete_input_status   : pos = -1; break;
    case function_code::read_holding_registers       : pos = -1; break;
    case function_code::read_input_registers         : pos = -1; break;
    case function_code::write_single_coil            : pos = -1; break;
    case function_code::write_single_register        : pos = -1; break;
    case function_code::write_multiple_coils         : pos =  5; break;
    case function_code::write_multiple_registers     : pos =  5; break;
    case function_code::read_write_multiple_registers: pos =  9; break;
    }

    const size_t byte_count = (buffer_len > pos) ? buffer[pos] : 0;

    return pdu_len_req(func, byte_count);
}

size_t
pdu_len_rsp (uint8_t* buffer,
             size_t buffer_len)
{
    if (buffer_len == 0)
        return -1;

    const auto func = static_cast<function_code::function_code>(buffer[0]);

    size_t pos = -1;
    switch (func)
    {
    case function_code::read_coil_status             : pos =  1; break;
    case function_code::read_discrete_input_status   : pos =  1; break;
    case function_code::read_holding_registers       : pos =  1; break;
    case function_code::read_input_registers         : pos =  1; break;
    case function_code::write_single_coil            : pos = -1; break;
    case function_code::write_single_register        : pos = -1; break;
    case function_code::write_multiple_coils         : pos = -1; break;
    case function_code::write_multiple_registers     : pos = -1; break;
    case function_code::read_write_multiple_registers: pos =  1; break;
    }

    const size_t byte_count = (buffer_len > pos) ? buffer[pos] : 0;

    return pdu_len_rsp(func, byte_count);
}

/* ----------------------------------------------------------------------- */

size_t
pdu_len_req (function_code::function_code func,
             size_t bc) // byte count
{
    switch (func)
    {
    case function_code::read_coil_status             :         return pdu_len_req_read_coil_status             (  ); break;
    case function_code::read_discrete_input_status   :         return pdu_len_req_read_discrete_input_status   (  ); break;
    case function_code::read_holding_registers       :         return pdu_len_req_read_holding_registers       (  ); break;
    case function_code::read_input_registers         :         return pdu_len_req_read_input_registers         (  ); break;
    case function_code::write_single_coil            :         return pdu_len_req_write_single_coil            (  ); break;
    case function_code::write_single_register        :         return pdu_len_req_write_single_register        (  ); break;
    case function_code::write_multiple_coils         : if (bc) return pdu_len_req_write_multiple_coils         (bc); break;
    case function_code::write_multiple_registers     : if (bc) return pdu_len_req_write_multiple_registers     (bc); break;
    case function_code::read_write_multiple_registers: if (bc) return pdu_len_req_read_write_multiple_registers(bc); break;
    }

    return -1;
}

size_t
pdu_len_rsp (function_code::function_code func,
             size_t bc) // byte count
{
    switch (func)
    {
    case function_code::read_coil_status             : if (bc) return pdu_len_rsp_read_coil_status             (bc); break;
    case function_code::read_discrete_input_status   : if (bc) return pdu_len_rsp_read_discrete_input_status   (bc); break;
    case function_code::read_holding_registers       : if (bc) return pdu_len_rsp_read_holding_registers       (bc); break;
    case function_code::read_input_registers         : if (bc) return pdu_len_rsp_read_input_registers         (bc); break;
    case function_code::write_single_coil            :         return pdu_len_rsp_write_single_coil            (  ); break;
    case function_code::write_single_register        :         return pdu_len_rsp_write_single_register        (  ); break;
    case function_code::write_multiple_coils         :         return pdu_len_rsp_write_multiple_coils         (  ); break;
    case function_code::write_multiple_registers     :         return pdu_len_rsp_write_multiple_registers     (  ); break;
    case function_code::read_write_multiple_registers: if (bc) return pdu_len_rsp_read_write_multiple_registers(bc); break;
    }

    return -1;
}

}
