# netcdf_hycom
C++ NetCDF Interface for the HYCOM Global Ocean Model

This program is designed to extract 4D oceanographic data from the most recent HYCOM [https://hycom.org/] hindcast (GOFS 3.1: 41-layer HYCOM + NCODA Global 1/12° Analysis).

This program creates 4D c-style arrays for two fields, pressure and temperature, over a user-specifed spatial and temporal range.

If desired, the user can request that a new NetCDF file be created, representing the desired data subset of interest.


