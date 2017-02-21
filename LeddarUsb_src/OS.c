#include <stdio.h>
#include "OS.h"
#include <wchar.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/time.h>

LtResult
OpenSerialPort( char *aPortName, LtHandle *aHandle )
{
    char lPortName[LT_MAX_PORT_NAME_LEN+6];

    sprintf( lPortName, "/dev/%s", aPortName );
    *aHandle = open( lPortName, O_RDWR | O_NOCTTY );

    if ( *aHandle >= 0 )
    {
        struct termios tio;

        if ( !tcgetattr( *aHandle, &tio ) )
        {
        	// 8 bits per char, ignore modem control lines, enable receiver.
			tio.c_cflag = CS8 | CLOCAL | CREAD;
			switch( LT_SERIAL_SPEED )
			{
			    case 9600:
			    	tio.c_cflag |= B9600;
				    break;
			    case 19200:
			    	tio.c_cflag |= B19200;
				    break;
			    case 38400:
			    	tio.c_cflag |= B38400;
				    break;
			    case 57600:
			    	tio.c_cflag |= B57600;
				    break;
			    default:
				    tio.c_cflag |= B115200;
				    break;
			}
			if ( LT_STOP_BITS == 2 )
			{
				tio.c_cflag |= CSTOPB;
			}
			switch( LT_PARITY )
			{
				case LT_PARITY_NONE:
					// do nothing
					break;
			    case LT_PARITY_ODD:
			    	tio.c_cflag |= PARENB | PARODD;
			    	break;
			    case LT_PARITY_EVEN:
			    	tio.c_cflag |= PARENB;
			    	break;
			}
			// Enable parity checking on input
			tio.c_iflag = INPCK;
			// So special output processing
			tio.c_oflag = 0;
			// Raw mode
			tio.c_lflag = 0;
			// None of the 4 modes provided by VMIN and VTIME correspond to
			// what we need with Modbus, so we set 0 on both which gives
			// immediate return on call to read whatever the availability
			// of data.
			tio.c_cc[VMIN] = 0;
			tio.c_cc[VTIME] = 0;

			if ( !tcsetattr( *aHandle, TCSANOW, &tio ) )
			{
		    	return LT_SUCCESS;
			}
        }

    	close( *aHandle );
    }

    return LT_ERROR;
}

void
CloseSerialPort( LtHandle aHandle )
{
    if ( aHandle != LT_INVALID_HANDLE )
    {
        close( aHandle );
    }
}

LtResult
WriteToSerialPort( LtHandle aHandle, LtByte *aData, int aLength )
{
    if ( tcflush( aHandle, TCIFLUSH ) )
    {
    	return LT_ERROR;
    }
    return write( aHandle, aData, aLength );
}

LtResult
ReadFromSerialPort( LtHandle aHandle, LtByte *aData, int aMaxLength )
{
    // Wait for the first byte with a long timeout to let time for the sensor
    // to process the command.
    struct timeval lTimeout;
    fd_set         lFds;
    LtResult       lRead = 0;
    int            lMicroseconds = 20000000/LT_SERIAL_SPEED;

    lTimeout.tv_sec = 1;
    lTimeout.tv_usec = 0;

    FD_ZERO( &lFds );
    FD_SET( aHandle, &lFds );

    if ( select( aHandle+1, &lFds, NULL, NULL, &lTimeout ) <= 0 )
    {
    	return LT_ERROR;
    }

    // In theory we want an inter-character timeout of 2 character but
    // in practice setting a value too low is not reliable.
    lMicroseconds = lMicroseconds < 2000 ? 2000 : lMicroseconds;
    // Now read the data with a short inter-byte timeout.
    // We end either when we have received the number of bytes or
    // there is a too long interval between 2 bytes (indicating
    // the end of the message).
    while( lRead < aMaxLength )
    {
    	int lResult;

        lTimeout.tv_sec = 0;
        lTimeout.tv_usec = lMicroseconds;

        FD_ZERO( &lFds );
        FD_SET( aHandle, &lFds );

        lResult = select( aHandle+1, &lFds, NULL, NULL, &lTimeout );

        if ( lResult < 0 )
        {
        	return LT_ERROR;
        }
        else if ( lResult == 0 )
        {
        	return lRead;
        }

    	lResult = read( aHandle, aData+lRead, aMaxLength-lRead );

    	if ( lResult < 0 )
    	{
    		return LT_ERROR;
    	}

    	lRead += lResult;
    }

    return lRead;
}
