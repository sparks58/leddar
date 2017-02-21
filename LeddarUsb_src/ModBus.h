#ifndef _MOBDUS_H_
#define _MOBDUS_H_

#include "OS.h"

#define MODBUS_MAX_PAYLOAD 252
#define MODBUS_SERVER_ID   0x11

LtBool
ModbusConnected( void );

LtResult
ModbusSend( LtByte aFunction, LtByte *aBuffer, LtByte aLength );

LtResult
ModbusReceive( LtByte *aBuffer );

LtResult
ModbusConnect( char *aPortName, LtByte aAddress );

void
ModbusDisconnect( void );

LtResult
ModbusReadInputRegisters( LtU16 aFirst, LtU16 aCount, LtU16 *aValues );

#endif
