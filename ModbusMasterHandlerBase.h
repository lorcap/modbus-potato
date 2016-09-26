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
        bool read_coils_rsp(IFramer* framer) override {
            (void) framer;
            return true;
        }
        bool read_discrete_inputs_rsp(IFramer* framer) override {
            (void) framer;
            return true;
        }
        bool read_holding_registers_rsp(uint16_t address, size_t n, const uint16_t* values) override {
            (void) address;
            (void) n;
            (void) values;
            return true;
        }
        bool read_input_registers_rsp(uint16_t address, size_t n, const uint16_t* values) override {
            (void) address;
            (void) n;
            (void) values;
            return true;
        }
        bool write_single_coil_rsp(IFramer* framer) override {
            (void) framer;
            return true;
        }
        bool write_single_register_rsp(uint16_t address) override {
            (void) address;
            return true;
        }
        bool write_multiple_coils_rsp(IFramer* framer) override {
            (void) framer;
            return true;
        }
        bool write_multiple_registers_rsp(uint16_t address, size_t n) override {
            (void) address;
            (void) n;
            return true;
        }
        bool response_time_out(void) override {
            return true;
        }
        bool exception_response(enum modbus_exception_code::modbus_exception_code code) override {
            (void) code;
            return true;
        }
    };
}
#endif
