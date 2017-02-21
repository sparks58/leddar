#ifndef _OS_H_
#define _OS_H_

#include "UserDefs.h"

typedef unsigned char  LtBool;
typedef unsigned char  LtByte;
typedef int            LtResult;
typedef unsigned short LtU16;
typedef short          Lt16;
typedef unsigned int   LtU32;
typedef int            LtHandle;

#define LT_INVALID_HANDLE     (-1)
#define LT_MAX_PORT_NAME_LEN  24

LtResult
OpenSerialPort( char *aPortName, LtHandle *aHandle );

void
CloseSerialPort( LtHandle aHandle );

LtResult
WriteToSerialPort( LtHandle aHandle, LtByte *aData, int aLength );

LtResult
ReadFromSerialPort( LtHandle aHandle, LtByte *aData, int aMaxLength );

#endif
