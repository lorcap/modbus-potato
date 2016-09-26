#ifndef __ModbusMasterHandlerHolding_h__
#define __ModbusMasterHandlerHolding_h__
#include "ModbusInterface.h"
namespace ModbusPotato
{
    /// <summary>
    /// This class is an example of a master handler for reading and writing holding registers to an array.
    /// </summary>
    class CModbusMasterHandlerHolding : public IMasterHandler
    {
    public:
        CModbusMasterHandlerHolding(uint16_t* array, size_t len);
        bool read_holding_registers_rsp(uint16_t address, size_t n, const uint16_t* values) override;
        bool write_multiple_registers_rsp(uint16_t address, size_t count) override;
    private:
        uint16_t* m_array;
        size_t m_len;

        bool sanity_check (const uint16_t address, const size_t n) const;
    };
}
#endif
