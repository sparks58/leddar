#ifndef _LEDDAR_H_
#define _LEDDAR_H_

#include "OS.h"

#define LEDDAR_MAX_DETECTIONS    48
#define LEDDAR_TEMPERATURE_SCALE 256.f

#define LEDDAR_NO_CONFIGURATION     0
#define LEDDAR_SIMPLE_CONFIGURATION 1
#define LEDDAR_FULL_CONFIGURATION   2

#define LEDDAR_EVAL_KIT          7
#define LEDDAR_IS16              8
#define LEDDAR_MODULE            9

// Bits for mStates field
#define LEDDAR_STATES_INTENSITY_MASK 0xFF
#define LEDDAR_STATES_DEMERGE_SHIFT  9

// Register addresses for inputs
#define LEDDAR_INPUT_TEMPERATURE 0

typedef struct _LtDetection
{
    int mDistance;
    float mAmplitude;
    short mSegment;
    short mFlags;
} LtDetection;

typedef struct _LtAcquisition
{
    LtU32 mTimestamp;
    LtU16 mStates;
    LtU16 mDetectionCount;
    float mTemperature;

    LtDetection mDetections[LEDDAR_MAX_DETECTIONS];
} LtAcquisition;

LtResult
LeddarConnect( char *aPortName, LtByte aAddress );

void
LeddarDisconnect( void );

LtResult
LeddarGetResults( LtAcquisition *aAcquisition );

int
LeddarConfigurationLevel( void );

LtResult
LeddarGetTemperature( float *aValue );

#endif
