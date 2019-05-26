# netcdf_hycom

/************************************************************/
/*    NAME: Blake Cole                                      */
/*    ORGN: MIT                                             */
/*    FILE: README.md                                       */
/*    DATE: 26 MAY 2019                                     */
/************************************************************/

C++ NetCDF Interface for the HYCOM Global Ocean Model

This program is designed to extract 4D oceanographic data from the most recent HYCOM [https://hycom.org/] hindcast (GOFS 3.1: 41-layer HYCOM + NCODA Global 1/12° Analysis).

This program creates 4D c-style arrays for two fields, salinity and temperature, over a user-specifed spatial and temporal range.

If desired, the user can request that a new NetCDF file be created, representing the desired data subset of interest.

--

[User Manual PDF](./netcdf_hycom_readme.pdf "User Manual PDF")

--

Compiling on MacOSX:

1. Use MacPorts to install the NetCDF Libraries for C++.
	- $ sudo port install netcdf-cxx4
	- dependencies: hdf5, netcdf

2. Because netcdf-cxx4 is a C++ library located in a non-default directory [opt/local/lib], a few command line switches are required to compile correctly
	a) Show (almost) all compiler warnings.
		-Wall
	b) Augment the g++ “include” path (where the .h header file lives).
		-I/opt/local/include
	c) Augment the g++ “link” path (where the library file lives).
		-L/opt/local/lib
	d) Specify which library needs to be included.
		-lnetcdf_c++4
	e) So, the full command line argument might look like:
               g++ -o ../bin/netcdf_hycom netcdf_hycom.cpp -Wall -I/opt/local/include -L/opt/local/lib -lnetcdf_c++4
               g++ -o ../bin/netcdf_hycom_readonly netcdf_hycom_readonly.cpp -Wall -I/opt/local/include -L/opt/local/lib -lnetcdf_c++4


--

Compiling on Ubuntu-Linux:

1. Use 'apt' or 'apt-get' to install the NetCDF Libraries for C++
        - $ sudo apt install libnetcdf-c++4-dev
        - dependencies: hdf5-helpers, libaec-dev, libcurl4-gnutls-dev, libhdf5-cpp-100, libhdf5-dev, libnetcdf-dev
	- suggested additional downloads:
		- libnetcdf-c++4-1
		- ncview
		- nco
		- cdo

2. In Ubuntu, the NetCDF libraries and header files have slightly different names than they do in MacOSX; further, if they are downloaded via ‘apt’ or ‘apt-get’, they will be located in different directories.  See below:
	a) Show (almost) all compiler warnings.
		-Wall
	b) Augment the g++ “include” path (where the .h header file lives).
		-I/usr/include
	c) Augment the g++ “link” path (where the library file lives).
		-L/usr/lib/x86_64-linux-gnu
	d) Specify which library needs to be included.
		-lnetcdf_c++4
	e) So, the full command line argument might look like:
		$ g++ -o ../bin/netcdf_hycom netcdf_hycom.cpp -Wall -I/usr/include -L/usr/lib/x86_64-linux-gnu -lnetcdf_c++4
                $ g++ -o ../bin/netcdf_hycom_readonly netcdf_hycom_readonly.cpp -Wall -I/usr/include -L/usr/lib/x86_64-linux-gnu -lnetcdf_c++4


This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.


--

Important Usage / Release Notes -- 26 MAY 2019:

1. If specifying a date range from the command line, using '--tstart=' and '--tstop=', it is CRUCIAL that the correct format be used: [YYYY:MM:DD] (brackets included).
   e.g. '--tstart=[2019:05:26] --tstop=[2019:05:27]'
   Any and all other formats, or the accidental exclusion of brackets, will cause the program will fail.

2. It is not currently possible to specify start/stop 'hours' via the command line switches.
   If this level of precision is required, simply run the program without command line switches, and enter the data when prompted by the program.

3. Command line switches are IN NO WAY 'quality-controlled' by the program; it is assumed that the user is providing appropriate values.
   Inappropriate inputs will cause the program to fail.

4. If an insufficient number of command line switches are detected, the user will be prompted to enter each spatio-temporal boundary parameter individually.
   These inputs ARE 'quality-controlled'; the program will continue to query the user until a valid entry is detected.
   This is honestly the safest and best way to input the 4D bounding box parameters; the command line switches are simply a useful tool if running many queries repeatedly.

5. HYCOM model data is partitioned into one-year chunks, and stored in corresponding directories.
   Accordingly, if earlier (pre-2018/2019) data is required, one must edit the field 'dataURL' (line 68) of netcdf_hycom.cpp to reference the appropriate directory.
