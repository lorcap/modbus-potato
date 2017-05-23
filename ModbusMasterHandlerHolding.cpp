#include "ModbusMasterHandlerHolding.h"
namespace ModbusPotato
{
    bool CModbusMasterHandlerHolding::sanity_check (const uint16_t address, const size_t n) const
    {
            return ((n <= m_len) && (address < m_len) && ((size_t) (address + n) <= m_len));
    }

    CModbusMasterHandlerHolding::CModbusMasterHandlerHolding(uint16_t* array, size_t len)
        :  m_array(array)
        ,  m_len(len)
    {
    }

    bool CModbusMasterHandlerHolding::read_holding_registers_rsp(uint16_t address, size_t n, const uint16_t* values)
    {
        if (!sanity_check(address, n))
            return false;

        // copy in the values
        while (n--)
            m_array[address++] = *values++;

        return true;
    }

    bool CModbusMasterHandlerHolding::write_multiple_registers_rsp(uint16_t address, size_t count)
    {
        if (!sanity_check(address, count))
            return false;

        return true;
    }
}
