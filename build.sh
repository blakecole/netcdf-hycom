#!/bin/bash

# NOTE! Not currently functional.

if [[ "$OSTYPE" == "linux-gnu" ]]; then
    g++ -o ./bin/netcdf_hycom ./src/netcdf_hycom.cpp
    -Wall
    -I/usr/include
    -L/usr/lib/x86_64-linux-gnu
    -lnetcdf_c++4;

    g++ -o ./bin/netcdf_hycom_readonly ./src/netcdf_hycom_readonly.cpp
    -Wall
    -I/usr/include
    -L/usr/lib/x86_64-linux-gnu
    -lnetcdf_c++4;

elif [[ "$OSTYPE" == "darwin"* ]]; then
    g++ -o ./bin/netcdf_hycom ./src/netcdf_hycom.cpp
    -Wall
    -I/opt/local/include
    -L/opt/local/lib
    -lnetcdf_c++4;

    g++ -o ./bin/netcdf_hycom_readonly ./src/netcdf_hycom_readonly.cpp
    -Wall
    -I/opt/local/include
    -L/opt/local/lib
    -lnetcdf_c++4;

#elif [[ "$OSTYPE" == "cygwin" ]]; then
        # POSIX compatibility layer / Linux emulation for Windows;
#elif [[ "$OSTYPE" == "msys" ]]; then
        # Light shell/ GNU utilities compiled for Windows (part of MinGW)
#elif [[ "$OSTYPE" == "win32" ]]; then
        # I'm not sure this can happen.
#elif [[ "$OSTYPE" == "freebsd"* ]]; then
        # ...
else
        # Unknown.
fi
