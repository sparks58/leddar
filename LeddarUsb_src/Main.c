#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#include "Leddar.h"

void *handleClient(void *arg);
void receiveNextCommand(char*, int);
void *capture(void *arg);

static pthread_mutex_t dataLock;
static int distances[16];
static double amplitudes[16];

int
main( int argc, char *argv[] ) {
	int n, s;
	socklen_t len;
	struct sockaddr_in name;
	pthread_mutex_init(&dataLock, NULL);
	// create the socket
	if ( (s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}

	memset(&name, 0, sizeof(struct sockaddr_in));
	name.sin_family = AF_INET;
	name.sin_port = htons(9004);
	len = sizeof(struct sockaddr_in);

	// listen on all network interfaces
	n = INADDR_ANY;
	memcpy(&name.sin_addr, &n, sizeof(long));

	int reuse = 1;
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0) {
	perror("setsockopt(SO_REUSEADDR)");
	exit(1);
	}

	if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (const char*)&reuse, sizeof(reuse)) < 0) {
	perror("setsockopt(SO_KEEPALIVE)");
	exit(1);
	}

	// bind socket the network interface
	if (bind(s, (struct sockaddr *) &name, len) < 0) {
	perror("bind");
	exit(1);
	}

	// listen for connections
	if (listen(s, 5) < 0) {
	perror("listen");
	exit(1);
	}

	pthread_t captureThreadId;
	int i = 3;
	int captureThread = pthread_create(&captureThreadId, NULL, capture, (void*)&i);

	// it is important to detach the thread to avoid memory leak
	pthread_detach(captureThreadId);

	while(1) {

	// block until get a request
	int ns = accept(s, (struct sockaddr *) &name, &len);

	if ( ns < 0 ) {
	  perror("accept");
	  exit(1);
	}

	// each client connection is handled by a seperate thread since
	// a single client can hold a connection open indefinitely making multiple
	// data requests prior to closing the connection
	pthread_t threadId;
	int thread = pthread_create(&threadId, NULL, handleClient, (void*) &ns);
	// it is important to detach the thread to avoid a memory leak
	pthread_detach(threadId);
	}

	close(s);
	exit(0);
}

void *capture(void *arg) {
	char portName[] = "ttyUSB0";
	LtByte address = 1;

	if ( LeddarConnect( portName, address ) == LT_SUCCESS ) {
		while (1) {
			// printf("hi\n");
			LtResult lResult;
			LtAcquisition lAcquisition;
			lResult = LeddarGetResults( &lAcquisition );

			if ( lResult == LT_SUCCESS ) {
				LtDetection *lDetections = lAcquisition.mDetections;
				int i;

				pthread_mutex_lock(&dataLock);
				for( i=0; i<lAcquisition.mDetectionCount; ++i ) {
					short segment = lDetections[i].mSegment;
					int distance = lDetections[i].mDistance;
					double amplitude = lDetections[i].mAmplitude;
					amplitudes[segment] = amplitude;
					distances[segment] = distance;
				}
				pthread_mutex_unlock(&dataLock);
			

			} else {
				puts( "Communication error, aborting." );
			}
		}
		LeddarDisconnect();
	} else {
		puts( "\nConnection failed!" );
	}

	return 0;
}

void *handleClient(void *arg) {
  // printf("Thread starting\n");
  int ns = *((int*) arg);
  char sendBuffer[1024];
  char command[128];

  // start conversation with client
  while(1) {

    receiveNextCommand(command, ns);

    if ( strcmp(command, "STOP") == 0 ) {
      //printf("Received STOP command\n");
      break;
    } else if ( strcmp(command, "DATA") == 0 ) {
      //printf("Received DATA command\n");

      // obtain the lock and copy the data
      pthread_mutex_lock(&dataLock);
      int copyDistances[16];
      double copyAmplitudes[16];
      for ( int i = 0; i < 16; i++ ) {
    	  copyDistances[i] = distances[i];
    	  copyAmplitudes[i] = amplitudes[i];
      }
      pthread_mutex_unlock(&dataLock);

      // the protocol will send an empty line when the data transfer is complete
      int sendBufferLen = 0;
      for ( int i = 0; i < 16; i++ ) {
        sendBufferLen += sprintf(sendBuffer + sendBufferLen, "%d=%d\n", i, copyDistances[i]);
      }
      for( int j = 0; j < 16; j++ )
      {
      	sendBufferLen += sprintf(sendBuffer + sendBufferLen, "%d=%f\n", j, copyAmplitudes[j]);
      }
      sendBufferLen += sprintf(sendBuffer + sendBufferLen, "\n");

      // write response to client
      write(ns, sendBuffer, sendBufferLen);
    } else {
      //printf("Received unknown command '%s'\n", command);
      break;
    }

  }
  close(ns);
  //printf("Thread ending\n");
  return 0;
}

void receiveNextCommand(char *command, int ns) {
  int receiveLength = read(ns, command, 1024);
  int commandLength = 0;
  while(commandLength < receiveLength) {
    char value = command[commandLength];
    if ( value == 0x0d || value == 0x0a ) {
      break;
    }
    commandLength++;
  }

  // add the terminating 0 to mark the end of the string value in the char *
  command[commandLength] = 0;
}
