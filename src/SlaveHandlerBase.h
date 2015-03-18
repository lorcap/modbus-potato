#include "Interface.h"
namespace ModbusPotato
{
    /// <summary>
    /// This class provides a base class which can be inherited to provide default handlers for each the modbus function types.
    /// </summary>
    class CSlaveHandlerBase : public ISlaveHandler
    {
    public:
        virtual modbus_exception_code::modbus_exception_code read_holding_registers(uint16_t address, uint16_t count, uint16_t* result) { return modbus_exception_code::illegal_function; }
        virtual modbus_exception_code::modbus_exception_code preset_single_register(uint16_t address, uint16_t value) { return modbus_exception_code::illegal_function; }
        virtual modbus_exception_code::modbus_exception_code write_multiple_registers(uint16_t address, uint16_t count, const uint16_t* values) { return modbus_exception_code::illegal_function; }
    };
}