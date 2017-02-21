#include <stdlib.h>
#include <string.h>

#include "Leddar.h"
#include "ModBus.h"

static int   gConfigurationLevel = LEDDAR_NO_CONFIGURATION;
static LtU16 gBuildNumber = 0;
static int   gDeviceType;

LtResult
LeddarConnect( char *aPortName, LtByte aAddress )
{
    LtResult lResult;

    lResult = ModbusConnect( aPortName, aAddress );

    if ( lResult == LT_SUCCESS )
    {
        lResult = ModbusSend( MODBUS_SERVER_ID, NULL, 0 );

        if ( lResult == LT_SUCCESS )
        {
            LtByte lId[MODBUS_MAX_PAYLOAD];

            lResult = ModbusReceive( lId );

            if ( lResult >= 0 )
            {
                gBuildNumber = lId[136] + (lId[137]<<8);
                gDeviceType = lId[150];

                // Identify which of the Leddar sensor model we are talking
                // to, to know features are available.
                if ( gDeviceType == LEDDAR_MODULE )
                {
                    gConfigurationLevel = LEDDAR_FULL_CONFIGURATION;
                    return LT_SUCCESS;
                }
                else if ( gDeviceType == LEDDAR_IS16 )
                {
                    gConfigurationLevel = LEDDAR_SIMPLE_CONFIGURATION;
                    return LT_SUCCESS;
                }
                else if ( gDeviceType == LEDDAR_EVAL_KIT )
                {
                    gConfigurationLevel = LEDDAR_NO_CONFIGURATION;
                    return LT_SUCCESS;
                }

                // Unrecognized device!
                lResult = LT_ERROR;
            }
        }

        ModbusDisconnect();
    }

    return lResult;
}

void
LeddarDisconnect( void )
{
    ModbusDisconnect();
    gConfigurationLevel = LEDDAR_NO_CONFIGURATION;
}

LtResult
LeddarGetResults( LtAcquisition *aAcquisition )
{
    LtResult lResult;


    lResult = ModbusSend( 0x41, NULL, 0 );

    if ( lResult == LT_SUCCESS )
    {
        LtByte lBuffer[MODBUS_MAX_PAYLOAD];

        lResult = ModbusReceive( lBuffer );

        if ( lResult > 0 )
        {
            int i;
            LtDetection *lDetections = aAcquisition->mDetections;

            aAcquisition->mDetectionCount = lBuffer[0] < LEDDAR_MAX_DETECTIONS ? lBuffer[0] : LEDDAR_MAX_DETECTIONS;

            for( i=0; i<aAcquisition->mDetectionCount; ++i )
            {
                lDetections[i].mDistance = ( lBuffer[1+i*5] + lBuffer[2+i*5]*256 );
                // lDetections[i].mAmplitude = ( lBuffer[3+i*5] + lBuffer[4+i*5]*256 )/64.f;
                lDetections[i].mSegment = lBuffer[5+i*5]>>4;
                // lDetections[i].mFlags = lBuffer[5+i*5] & 0xf;
            }

            

            return LT_SUCCESS;
        }
    }

    return lResult;
}

int
LeddarConfigurationLevel( void )
{
    return gConfigurationLevel;
}

LtResult
LeddarGetTemperature( float *aValue )
{
    LtU16 lValue;

    LtResult lResult = ModbusReadInputRegisters( LEDDAR_INPUT_TEMPERATURE, 1, &lValue );

    if ( lResult == LT_SUCCESS )
    {
        *aValue = lValue/LEDDAR_TEMPERATURE_SCALE;
    }

    return lResult;
}
