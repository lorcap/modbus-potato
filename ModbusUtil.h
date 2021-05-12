#ifndef __ModbusPotato_ModbusUtil_h__
#define __ModbusPotato_ModbusUtil_h__
#include <cstddef>
#include "ModbusTypes.h"
namespace ModbusPotato
{

constexpr int PDU_LEN_FUNCTION   = 1;
constexpr int PDU_LEN_ADDRESS    = 2;
constexpr int PDU_LEN_BYTE_COUNT = 1;
constexpr int PDU_LEN_QUANTITY   = 2;
constexpr int PDU_LEN_CRC        = 2;

/* --- general PDU length ------------------------------------------------ */

extern size_t pdu_len (uint8_t station_address, uint8_t* buffer, size_t buffer_len);

/* --- request PDU length ------------------------------------------------ */

extern size_t pdu_len_req (uint8_t* buffer, size_t buffer_len);
extern size_t pdu_len_req (function_code::function_code func, size_t byte_count = 0);

inline size_t pdu_len_req_read_coil_status ()
{
    return PDU_LEN_FUNCTION
         + PDU_LEN_ADDRESS
         + PDU_LEN_QUANTITY // quantity/value
         + PDU_LEN_CRC;
}

inline size_t pdu_len_req_read_discrete_input_status ()
{ return pdu_len_req_read_coil_status(); }

inline size_t pdu_len_req_read_holding_registers ()
{ return pdu_len_req_read_coil_status(); }

inline size_t pdu_len_req_read_input_registers ()
{ return pdu_len_req_read_coil_status(); }

inline size_t pdu_len_req_write_single_coil ()
{ return pdu_len_req_read_coil_status(); }

inline size_t pdu_len_req_write_single_register ()
{ return pdu_len_req_read_coil_status(); }

inline size_t pdu_len_req_write_multiple_coils (int byte_count)
{
    return PDU_LEN_FUNCTION
         + PDU_LEN_ADDRESS
         + PDU_LEN_QUANTITY
         + PDU_LEN_BYTE_COUNT
         + byte_count
         + PDU_LEN_CRC;
}

inline size_t pdu_len_req_write_multiple_registers (int byte_count)
{ return pdu_len_req_write_multiple_coils(byte_count); }

inline size_t pdu_len_req_read_write_multiple_registers (int byte_count)
{
    return PDU_LEN_FUNCTION
         + PDU_LEN_ADDRESS
         + PDU_LEN_QUANTITY
         + PDU_LEN_ADDRESS
         + PDU_LEN_QUANTITY
         + PDU_LEN_BYTE_COUNT
         + byte_count
         + PDU_LEN_CRC;
}

/* --- response PDU length ----------------------------------------------- */

extern size_t pdu_len_rsp (uint8_t* buffer, size_t buffer_len);
extern size_t pdu_len_rsp (function_code::function_code func, size_t byte_count = 0);

inline size_t pdu_len_rsp_read_coil_status (int byte_count)
{
    return PDU_LEN_FUNCTION
         + PDU_LEN_BYTE_COUNT
         + byte_count
         + PDU_LEN_CRC;
}

inline size_t pdu_len_rsp_read_discrete_input_status (int byte_count)
{ return pdu_len_rsp_read_coil_status(byte_count); }

inline size_t pdu_len_rsp_read_holding_registers (int byte_count)
{ return pdu_len_rsp_read_coil_status(byte_count); }

inline size_t pdu_len_rsp_read_input_registers (int byte_count)
{ return pdu_len_rsp_read_coil_status(byte_count); }

inline size_t pdu_len_rsp_write_single_coil ()
{
    return PDU_LEN_FUNCTION
         + PDU_LEN_ADDRESS
         + PDU_LEN_QUANTITY
         + PDU_LEN_CRC;
}

inline size_t pdu_len_rsp_write_single_register ()
{ return pdu_len_rsp_write_single_coil(); }

inline size_t pdu_len_rsp_write_multiple_coils ()
{ return pdu_len_rsp_write_single_coil(); }

inline size_t pdu_len_rsp_write_multiple_registers ()
{ return pdu_len_rsp_write_single_coil(); }

inline size_t pdu_len_rsp_read_write_multiple_registers (int byte_count)
{ return pdu_len_rsp_read_coil_status(byte_count); }

}

#endif
