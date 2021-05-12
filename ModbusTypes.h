// This file contains simple type definitions such as enumerations.
//
#ifndef __ModbusPotato_Types_h__
#define __ModbusPotato_Types_h__
#include <stdint.h>
#ifdef ARDUINO
#include <Arduino.h>
#elif _MSC_VER
#include <Windows.h>
#endif
#define MODBUS_DATA_BUFFER_SIZE (255)
namespace ModbusPotato
{
#ifdef ARDUINO
    // system tick type
    typedef unsigned long system_tick_t;
#elif _MSC_VER
    // system tick type
    typedef DWORD system_tick_t;
#else
    typedef unsigned int system_tick_t;
#endif

    namespace function_code
    {
        /// <summary>
        /// Function codes
        /// </summary>
        enum function_code
        {
            read_coil_status = 0x01,
            read_discrete_input_status = 0x02,
            read_holding_registers = 0x03,
            read_input_registers = 0x04,
            write_single_coil = 0x05,
            write_single_register = 0x06,
            write_multiple_coils = 0x0f,
            write_multiple_registers = 0x10,
            read_write_multiple_registers = 0x17,
        };
    }

    namespace modbus_exception_code
    {
        /// <summary>
        /// Contains the one-byte exception codes as defined in the modbus protocol spec.
        /// </summary>
        /// <remarks>
        /// See the section 7 titled 'MODBUS Exception Responses' in the PDF at
        /// http://www.modbus.org/docs/Modbus_Application_Protocol_V1_1b3.pdf for
        /// more information.
        /// </remarks>
        enum modbus_exception_code
        {
            ok = 0,
            illegal_function = 0x01,
            illegal_data_address = 0x02,
            illegal_data_value = 0x03,
            server_device_failure = 0x04,
            acknowledge = 0x05,
            server_device_busy = 0x06,
            memory_parity_error = 0x08,
            gateway_path_unavailable = 0x0A,
            gateway_target_failed_to_respond = 0x0B
        };
    }
}
#endif
