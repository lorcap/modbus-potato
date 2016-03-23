#ifndef __ModbusMasterHandlerBase_h__
#define __ModbusMasterHandlerBase_h__
#include "ModbusInterface.h"
namespace ModbusPotato
{
    /// <summary>
    /// This class provides a base class which can be inherited to provide default handlers for master implementations.
    /// </summary>
    class CModbusMasterHandlerBase : public IMasterHandler
    {
    public:
        virtual bool read_coils_rsp(IFramer* framer) {
            (void) framer;
            return true;
        }
        virtual bool read_discrete_inputs_rsp(IFramer* framer) {
            (void) framer;
            return true;
        }
        virtual bool read_holding_registers_rsp(uint16_t address, size_t n, const uint16_t* values) {
            (void) address;
            (void) n;
            (void) values;
            return true;
        }
        virtual bool read_input_registers_rsp(uint16_t address, size_t n, const uint16_t* values) {
            (void) address;
            (void) n;
            (void) values;
            return true;
        }
        virtual bool write_single_coil_rsp(IFramer* framer) {
            (void) framer;
            return true;
        }
        virtual bool write_single_register_rsp(uint16_t address, uint16_t result) {
            (void) address;
            (void) result;
            return true;
        }
        virtual bool write_multiple_coils_rsp(IFramer* framer) {
            (void) framer;
            return true;
        }
        virtual bool write_multiple_registers_rsp(uint16_t address, size_t n, uint16_t* result) {
            (void) address;
            (void) n;
            (void) result;
            return true;
        }
        virtual bool response_time_out(void) {
            return true;
        }
        virtual bool exception_response(enum modbus_exception_code::modbus_exception_code code) {
            (void) code;
            return true;
        }
    };
}
#endif
