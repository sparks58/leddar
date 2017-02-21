# Leddar USB
Leddar USB is used to create a server on the Jetson which can read Leddar values from
the Jetson's USB port and publish via a TCP/IP server socket.  The protocol is TBD.

## Commands to Create an Eclipse project
- ```mkdir build```
- ```cd build```
- ```cmake -G"Eclipse CDT4 - Unix Makefiles -D CMAKE_BUILD_TYPE=Debug ../LeddarUsb_src``` will create the .project and .cproject files in the build folder
- Import into eclipse at the folder higher than build
# leddar
