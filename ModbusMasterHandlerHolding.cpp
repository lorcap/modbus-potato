#include "ModbusMasterHandlerHolding.h"
namespace ModbusPotato
{
    CModbusMasterHandlerHolding::CModbusMasterHandlerHolding(uint16_t* array, size_t len)
        :  m_array(array)
        ,  m_len(len)
    {
    }

    bool CModbusMasterHandlerHolding::read_holding_registers_rsp(uint16_t address, size_t n, const uint16_t* values)
    {
        // make sure the address and count are valid
        if (n > m_len || address >= m_len || (size_t)(address + n) > m_len)
            return false;

        // copy in the values
        for (; n; ++address, --n)
            m_array[address] = ntohs(*values++);

        return true;
    }

    bool CModbusMasterHandlerHolding::write_multiple_registers_rsp(uint16_t address, size_t count)
    {
        // check to make sure the address and count are valid
        if (count > m_len || address >= m_len || (size_t)(address + count) > m_len)
            return false;

        return true;
    }
}
