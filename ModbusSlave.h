#include "ModbusInterface.h"
namespace ModbusPotato
{
    /// <summary>
    /// This class implements a basic Modbus slave interface.
    /// </summary>
    class CModbusSlave : public IFrameHandler
    {
    public:
        CModbusSlave(ISlaveHandler* handler);
        void frame_ready(IFramer* framer) override;
    private:
        uint8_t read_bit_input_rsp(IFramer* framer, bool discrete);
        uint8_t read_registers_rsp(IFramer* framer, bool holding);
        uint8_t write_single_coil_rsp(IFramer* framer);
        uint8_t write_single_register_rsp(IFramer* framer);
        uint8_t write_multiple_coils_rsp(IFramer* framer);
        uint8_t write_multiple_registers_rsp(IFramer* framer);
        ISlaveHandler* m_handler;
    };
}
