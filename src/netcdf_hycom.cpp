/************************************************************/
/*    NAME: Blake Cole                                      */
/*    ORGN: MIT                                             */
/*    FILE: netcdf_hycom.cpp                                */
/*    DATE: 26 MAY 2019                                     */
/************************************************************/

#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <netcdf>
#include "boost/multi_array.hpp"

using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;

// LEAP YEAR CALCULATOR DECLARATIONS-----------------------
struct date{
  int y, m, d;
};

const int monthDays[12] = {31, 28, 31, 30, 31, 30,
                           31, 31, 30, 31, 30, 31};
unsigned int years;
unsigned long int n1, n2, hours;

// Declare functions (defined at bottom):
int countLeapYears(date d);
int getDifference(date date1, date date2);

//----------------------------------------------------------

// Return this code to the OS in case of failure.
static const int NC_ERR = 2;

int main(int argc, char **argv){

  for (int i=1; i<argc; i++){
    string argi_prerun = argv[i];
    if((argi_prerun.find("-h")==0)||(argi_prerun.find("--help")==0)){
      cout <<"\n  SUMMARY: This program is used to extract data from\n"
           <<"           HYCOM NetCDF files.\n"
           <<"           The user has two extraction options:\n"
           <<"           (1) Extract from local NetCDF file\n"
           <<"           (2) Extract directly from HYCOM online database.";
      cout <<"\n\n  USAGE: " << argv[0] << " [command line switches]\n"
           <<"  --tstart=[STRING]  : time: start time, [year:month:day]\n"
           <<"  --tstop=[STRING]   : time: end time, [year:month:day]\n"
           <<"  --depthmin=[FLOAT] : depth: shallowest depth\n"
           <<"  --depthmax=[FLOAT] : depth: deepest depth\n"
           <<"  --latmin=[FLOAT]   : latitude: southern edge\n"
           <<"  --latmax=[FLOAT]   : latitude: northern edge\n"
           <<"  --lonmin=[FLOAT]   : longitude: western edge\n"
           <<"  --lonmax=[FLOAT]   : longitude: eastern edge\n"
           <<"  --newfile=true     : write new netCDF file\n" << endl;

    cout << "  NOTE: If no input is detected on command line\n"
         << "        User will be prompted to specify bounds.\n" << endl;
    return(1);
    }
  }
  
  try{
    //---------------------------------------------------------------
    // 1. OPEN NETCDF FILE
    //---------------------------------------------------------------
    string dataURL = "http://tds.hycom.org/thredds/dodsC/GLBv0.08/expt_93.0";
    //string dataURL = "http://tds.hycom.org/thredds/dodsC/GLBv0.08/expt_92.9";
    //NcFile dataFile(dataURL, NcFile::read, NcFile::classic);
    NcFile dataFile(dataURL, NcFile::read);

    //---------------------------------------------------------------
    // 2. INSPECT NETCDF FILE
    //---------------------------------------------------------------
    cout << "-----------------------\n";
    cout << "NETCDF FILE INFO:" << endl;
    cout << "  " << dataFile.getVarCount()   << " variables"  << endl;
    cout << "  " << dataFile.getAttCount()   << " attributes" << endl;
    cout << "  " << dataFile.getDimCount()   << " dimensions" << endl;
    cout << "  " << dataFile.getGroupCount() << " groups"     << endl;
    cout << "  " << dataFile.getTypeCount()  << " types\n"    << endl;

    cout << "-----------------------\n";
    cout << "VARIABLES:" << endl;
    multimap<string,NcVar> varMap = dataFile.getVars();
    for (multimap<string,NcVar>::iterator it=varMap.begin();
         it!=varMap.end(); ++it)
      cout << "  " << (*it).first << endl;
    cout << endl;

    cout << "-----------------------\n";
    cout << "DIMENSIONS:" << endl;
    multimap<string,NcDim> dimMap = dataFile.getDims();
    for (multimap<string,NcDim>::iterator it=dimMap.begin();
         it!=dimMap.end(); ++it)
      cout << "  " << (*it).first << endl;
    cout << endl;

    //---------------------------------------------------------------
    // 3. READ & INSPECT INDEPENDENT (DIMENSION) VARIABLES
    //---------------------------------------------------------------
    NcVar depthVar, latVar, lonVar, timeVar;
    NcDim depthDim, latDim, lonDim, timeDim;

    cout << "-----------------------\n";
    cout << "DIMENSION SIZES:" << endl;
    // 3.1: Depth
    depthVar = dataFile.getVar("depth");
    if(depthVar.isNull())
      return NC_ERR;
    depthDim = depthVar.getDim(0);
    int depth_size = depthDim.getSize();
    float DEPTH[depth_size];
    depthVar.getVar(DEPTH);
    cout << "  depth = " << DEPTH[0] << ":" << DEPTH[depth_size-1]
         << "        [n=" << depth_size << "]" << endl;

    // 3.2: Latitude
    latVar = dataFile.getVar("lat");
    if(latVar.isNull())
      return NC_ERR;
    latDim = latVar.getDim(0);
    int lat_size = latDim.getSize();
    float LAT[lat_size];
    latVar.getVar(LAT);
    cout << "  lat   = " << LAT[0] << ":" << LAT[lat_size-1]
         << "        [n=" << lat_size << "]" << endl;
    
    // 3.3: Longitude
    lonVar = dataFile.getVar("lon");
    if(lonVar.isNull())
      return NC_ERR;
    lonDim = lonVar.getDim(0);
    int lon_size = lonDim.getSize();
    float LON[lon_size];
    lonVar.getVar(LON);
    cout << "  lon   = " << LON[0] << ":" << LON[lon_size-1]
         << "      [n=" << lon_size << "]" << endl;

    // 3.4: Time
    timeVar = dataFile.getVar("time");
    if(timeVar.isNull())
      return NC_ERR;
    timeDim = timeVar.getDim(0);
    int time_size = timeDim.getSize();
    float TIME[time_size];
    timeVar.getVar(TIME);
    cout << "  time  = " << TIME[0] << ":" << TIME[time_size-1]
         << " [n=" << time_size << "]" << endl;
    cout << endl;

    //---------------------------------------------------------------
    // 4. READ DEPENDENT VARIABLES
    //---------------------------------------------------------------
    
    //---------------------------------------------------------------
    // 4.1.1: Read spatial/temporal range from command line
    int y1, m1, d1, h1=0;
    int y2, m2, d2, h2=0;
    date ref_date = {2000,1,1}; //HYCOM reference date
    int tstart, tstop;
    float depth_min, depth_max;
    float lat_min, lat_max;
    float lon_min, lon_max;
    bool newfile = false;
    string newfile_response = "";
    int nparams = 0;
    for (int i=1; i<argc; i++){
      string input, subinput;
      string argi = argv[i];
      cout << "argv[" << i << "] = " << argi << endl;
      
      if(argi.find("--tstart=") == 0){
        input = argi.substr(9); //entire string token
        subinput = input.substr(1,4); //grab year
        y1 = atoi(subinput.c_str());  //convert to int
        subinput = input.substr(6,2); //grab month
        m1 = atoi(subinput.c_str());  //convert to int
        subinput = input.substr(9,2); //grab day
        d1 = atoi(subinput.c_str());  //convert to int
        nparams++;
      }
      else if(argi.find("--tstop=") == 0){
        input = argi.substr(8); //entire string token
        subinput = input.substr(1,4); //grab year
        y2 = atoi(subinput.c_str());  //convert to int
        subinput = input.substr(6,2); //grab month
        m2 = atoi(subinput.c_str());  //convert to int
        subinput = input.substr(9,2); //grab day
        d2 = atoi(subinput.c_str());  //convert to int
        nparams++;
      }
      else if(argi.find("--depthmin=") == 0){
        input = argi.substr(11);
        depth_min = atof(input.c_str());
        nparams++;
      }
      else if(argi.find("--depthmax=") == 0){
        input = argi.substr(11);
        depth_max = atof(input.c_str());
        nparams++;
      }
      else if(argi.find("--latmin=") == 0){
        input = argi.substr(9);
        lat_min = atof(input.c_str());
        nparams++;
      }
      else if(argi.find("--latmax=") == 0){
        input = argi.substr(9);
        lat_max = atof(input.c_str());
        nparams++;
      }
      else if(argi.find("--lonmin=") == 0){
        input = argi.substr(9);
        lon_min = atof(input.c_str());
        nparams++;
      }
      else if(argi.find("--lonmax=") == 0){
        input = argi.substr(9);
        lon_max = atof(input.c_str());
        nparams++;
      }
      else if(argi.find("--newfile=true") == 0){
        newfile=true;
      }
      else if(argi.find("--newfile=false") == 0){
        newfile=false;
      }
    }

    //4.1.2: If command line fails, manually input bounds
    if (nparams != 8){
      string input;
      cout << "\n(!)   ERROR: EXACTLY (8) PARAMETERS REQUIRED.   (!)\n"
           << "(!)                  "
           << "(" << nparams << ") PARAMETERS PROVIDED.   (!)\n"
           << "(!) PLEASE MANUALLY DEFINE SUBSET BOUNDS BELOW. (!)\n"
           << endl;

      // RECORD START DATE/TIME----------------------------
      while (true){
        cout << "  1A. Start Year [YYYY]: ";
        getline(cin, input);
        stringstream ss;
        ss.str(input);
        if (ss >> y1){
          if (y1 >= 0)
            break;
        }
        cout << "INVALID ENTRY, please try again" << endl;
      }

      while (true){
        cout << "  1B. Start Month [MM]: ";
        getline(cin, input);
        stringstream ss;
        ss.str(input);
        if (ss >> m1){
          if ((m1 >= 1)&&(m1 <= 12))
            break;
        }
        cout << "INVALID ENTRY, please try again" << endl;
      }

      while (true){
        cout << "  1C. Start Day [DD]: ";
        getline(cin, input);
        stringstream ss;
        ss.str(input);
        if (ss >> d1){
          if ((d1 >= 1)&&(d1 <= 31))
            break;
        }
        cout << "INVALID ENTRY, please try again" << endl;
      }

      while (true){
        cout << "  1D. Start Hour [0:23]: ";
        getline(cin, input);
        stringstream ss;
        ss.str(input);
        if (ss >> h1){
          if ((h1 >= 0)&&(h1 < 24))
            break;
        }
        cout << "INVALID ENTRY, please try again" << endl;
      }

      // RECORD STOP DATE/TIME----------------------------
      while (true){
        cout << "  2A. Stop Year [YYYY]: ";
        getline(cin, input);
        stringstream ss;
        ss.str(input);
        if (ss >> y2){
          if (y2 >= 0)
            break;
        }
        cout << "INVALID ENTRY, please try again" << endl;
      }

      while (true){
        cout << "  2B. Stop Month [MM]: ";
        getline(cin, input);
        stringstream ss;
        ss.str(input);
        if (ss >> m2){
          if ((m2 >= 1)&&(m2 <= 12))
            break;
        }
        cout << "INVALID ENTRY, please try again" << endl;
      }

      while (true){
        cout << "  2C. Stop Day [DD]: ";
        getline(cin, input);
        stringstream ss;
        ss.str(input);
        if (ss >> d2){
          if ((d2 >= 1)&&(d2 <= 31))
            break;
        }
        cout << "INVALID ENTRY, please try again" << endl;
      }

      while (true){
        cout << "  2D. Stop Hour [0:23]: ";
        getline(cin, input);
        stringstream ss;
        ss.str(input);
        if (ss >> h2){
          if ((h2 >= 0)&&(h2 < 24))
            break;
        }
        cout << "INVALID ENTRY, please try again" << endl;
      }

      // DEPTH RANGE--------------------------------------
      while (true){
        cout << "  3. Minimum Depth [meters] [" << DEPTH[0]
             << ":" << DEPTH[depth_size-1] << "]: ";
        getline(cin, input);
        stringstream ss;
        ss.str(input);
        if (ss >> depth_min){
          if((depth_min >= DEPTH[0])&&(depth_min <= DEPTH[depth_size-1]))
            break;
        }
        cout << "INVALID ENTRY, please try again" << endl;
      }

      while (true){
        cout << "  4. Maximum Depth [meters] [" << DEPTH[0]
             << ":" << DEPTH[depth_size-1] << "]: ";
        getline(cin, input);
        stringstream ss;
        ss.str(input);
        if (ss >> depth_max){
          if((depth_max >= DEPTH[0])&&(depth_max <= DEPTH[depth_size-1]))
            break;
        }
        cout << "INVALID ENTRY, please try again" << endl;
      }

      // LATITUDE RANGE-----------------------------------
      while (true){
        cout << "  5. Minimum Latitude [degrees] [" << LAT[0]
             << ":" << LAT[lat_size-1] << "]: ";
        getline(cin, input);
        stringstream ss;
        ss.str(input);
        if (ss >> lat_min){
          if((lat_min >= LAT[0])&&(lat_min <= LAT[lat_size-1]))
            break;
        }
        cout << "INVALID ENTRY, please try again" << endl;
      }

      while (true){
        cout << "  6. Maximum Latitude [degrees] [" << LAT[0]
             << ":" << LAT[lat_size-1] << "]: ";
        getline(cin, input);
        stringstream ss;
        ss.str(input);
        if (ss >> lat_max){
          if((lat_max >= LAT[0])&&(lat_max <= LAT[lat_size-1]))
            break;
        }
        cout << "INVALID ENTRY, please try again" << endl;
      }

      // LONGITUDE RANGE-----------------------------------
      while (true){
        cout << "  7. Minimum Longitude [degrees] [" << LON[0]
           << ":" << LON[lon_size-1] << "]: ";
        getline(cin, input);
        stringstream ss;
        ss.str(input);
        if (ss >> lon_min){
          if((lon_min >= LON[0])&&(lon_min <= LON[lon_size-1]))
            break;
        }
        cout << "INVALID ENTRY, please try again" << endl;
      }

      while (true){
        cout << "  8. Maximum Longitude [degrees] [" << LON[0]
             << ":" << LON[lon_size-1] << "]: ";
        getline(cin, input);
        stringstream ss;
        ss.str(input);
        if (ss >> lon_max){
          if((lon_max >= LON[0])&&(lon_max <= LON[lon_size-1]))
            break;
        }
        cout << "INVALID ENTRY, please try again" << endl;
      }

      // NEW NET CDF FILE DESIRED? -------------------------
      cout << "\n  WOULD YOU LIKE TO CREATE A NEW NETCDF FILE? [Y/n]: ";
      cin >> newfile_response;

      if ((newfile_response=="Y")  ||(newfile_response=="YES")
          ||(newfile_response=="y")||(newfile_response=="yes")){
        newfile = true;
      }
      else if ((newfile_response=="N")  ||(newfile_response=="NO")
          ||(newfile_response=="n")||(newfile_response=="no")){
        newfile = false;
      }
    }

    // 4.1.3: Convert YYYY/MM/DD HH to hours since 2000/1/1 00:00:00
    date date1 = {y1,m1,d1};
    tstart = getDifference(ref_date, date1);
    tstart += h1; // add hours

    date date2 = {y2,m2,d2};
    tstop = getDifference(ref_date, date2);
    tstop += h2; // add hours

    // 4.1.4: Search for closest HYCOM values
    int depth_ind_high=0;
    while (DEPTH[depth_ind_high] < depth_max)
      depth_ind_high++;
    int depth_ind_low=0;
    while (DEPTH[depth_ind_low] < depth_min)
      depth_ind_low++;

    int lat_ind_high=0;
    while (LAT[lat_ind_high] < lat_max)
      lat_ind_high++;
    int lat_ind_low=0;
    while (LAT[lat_ind_low] < lat_min)
      lat_ind_low++;
    
    int lon_ind_high=0;
    while (LON[lon_ind_high] < lon_max)
      lon_ind_high++;
    int lon_ind_low=0;
    while (LON[lon_ind_low] < lon_min)
      lon_ind_low++;

    int time_ind_high=0;
    while (TIME[time_ind_high] < tstop)
      time_ind_high++;
    int time_ind_low=0;
    while (TIME[time_ind_low] <= tstart)
      time_ind_low++;
    time_ind_low--; // one timestep back for inclusive range

    cout << "-----------------------\n";
    cout << "SPATIAL RANGE:" << endl;
    cout << "Requested:\n";
    cout << "  DEPTH = " << depth_min << ":" << depth_max << "\n";
    cout << "  LAT   = " << lat_min << ":" << lat_max << "\n";
    cout << "  LON   = " << lon_min << ":" << lon_max << "\n";
    cout << "  TIME  = " << tstart  << ":" << tstop   << "\n";
    
    
    cout << "Actual:\n";
    cout << "  DEPTH[" << depth_ind_low << ":" << depth_ind_high << "] = "
         << DEPTH[depth_ind_low] << ":" << DEPTH[depth_ind_high] << "\n";
    cout << "  LAT[" << lat_ind_low << ":" << lat_ind_high << "] = "
         << LAT[lat_ind_low] << ":" << LAT[lat_ind_high] << "\n";
    cout << "  LON[" << lon_ind_low << ":" << lon_ind_high << "] = "
         << LON[lon_ind_low] << ":" << LON[lon_ind_high] << "\n";
    cout << "  TIME[" << time_ind_low << ":" << time_ind_high << "] = "
         << TIME[time_ind_low] << ":" << TIME[time_ind_high] << "\n";
    cout << endl;

    // Index Ranges (+1 for inclusive)
    int lat_ind_range = lat_ind_high - lat_ind_low +1;
    int lon_ind_range = lon_ind_high - lon_ind_low +1;
    int depth_ind_range = depth_ind_high - depth_ind_low +1;
    int ntime = time_ind_high - time_ind_low +1;

    //---------------------------------------------------------------
    // 4.2: Initialize 3D arrays
    short int preSALT[depth_ind_range][lat_ind_range][lon_ind_range];
    short int preTEMP[depth_ind_range][lat_ind_range][lon_ind_range];
    //float SALT[ntime][depth_ind_range][lat_ind_range][lon_ind_range];
    //float TEMP[ntime][depth_ind_range][lat_ind_range][lon_ind_range];

    typedef boost::multi_array<float, 4> array_float4D;
    typedef array_float4D::index index;
    array_float4D SALT(boost::extents[ntime][depth_ind_range][lat_ind_range][lon_ind_range]);
    array_float4D TEMP(boost::extents[ntime][depth_ind_range][lat_ind_range][lon_ind_range]);

    NcVar saltVar, tempVar;
    saltVar = dataFile.getVar("salinity");
    if(saltVar.isNull())
      return NC_ERR;

    tempVar = dataFile.getVar("water_temp");
    if(tempVar.isNull())
      return NC_ERR;

    //---------------------------------------------------------------
    // 4.3: Write vectors to specify desired 4D data range
    vector<size_t> startp,countp;
    startp.push_back(0);             //start: overwritten in step (4.5)
    startp.push_back(depth_ind_low); //start: depth = shallow depth index
    startp.push_back(lat_ind_low);   //start: lat   = low lat index
    startp.push_back(lon_ind_low);   //start: lon   = low lon index
    countp.push_back(1);               //count: one record at a time
    countp.push_back(depth_ind_range); //count: depth index range
    countp.push_back(lat_ind_range);   //count: latitude index range
    countp.push_back(lon_ind_range);   //count: longitude index range

    for (int i=0; i<4; i++){
      cout << "startp[" << i << "] = " << startp[i] << endl;
      cout << "countp[" << i << "] = " << countp[i] << endl;
    }

    //---------------------------------------------------------------
    // 4.4 Determine offset & scale factor from variable attributes
    NcVarAtt scaleAtt, offsetAtt, missingAtt;
    float scale_factor_SALT[1], add_offset_SALT[1], no_val_SALT[1];
    float scale_factor_TEMP[1], add_offset_TEMP[1], no_val_TEMP[1];

    // Temperature:
    scaleAtt = tempVar.getAtt("scale_factor");
    if (scaleAtt.isNull()) return NC_ERR;
    scaleAtt.getValues(scale_factor_TEMP);

    offsetAtt = tempVar.getAtt("add_offset");
    if (offsetAtt.isNull()) return NC_ERR;
    offsetAtt.getValues(add_offset_TEMP);

    missingAtt = tempVar.getAtt("missing_value");
    if (missingAtt.isNull()) return NC_ERR;
    missingAtt.getValues(no_val_TEMP);

    // Salinity:
    scaleAtt = saltVar.getAtt("scale_factor");
    if (scaleAtt.isNull()) return NC_ERR;
    scaleAtt.getValues(scale_factor_SALT);

    offsetAtt = saltVar.getAtt("add_offset");
    if (offsetAtt.isNull()) return NC_ERR;
    offsetAtt.getValues(add_offset_SALT);

    missingAtt = saltVar.getAtt("missing_value");
    if (missingAtt.isNull()) return NC_ERR;
    missingAtt.getValues(no_val_SALT);

    cout << "Temperature Scale, Offset = "
         << scale_factor_TEMP[0] << "," << add_offset_TEMP[0] << endl;
    cout << "Salinity Scale, Offset = "
         << scale_factor_SALT[0] << "," << add_offset_SALT[0] << endl;


    //---------------------------------------------------------------
    // 4.5: Fill data arrays, multiply by scale factor, add offset

    // rather than using ntime, we need to:
    // (1) determine nearest real index brackets for time vector
    // (2) count number of intervening hourly timestamps
    // (3) loop through each
    cout << "--------------------------------\n";
    cout << "READING NETCDF DATA FILE... \n";
    
    for (index rec=0; rec<ntime; rec++){
      startp[0] = (time_ind_low + rec);
      saltVar.getVar(startp,countp,preSALT);
      tempVar.getVar(startp,countp,preTEMP);

      int time_ind = startp[0];
      cout << "TIME STAMP: " << TIME[time_ind]
           << " hours since 2000-01-01 00:00:00 "
           << "[" << rec << "/" << ntime << "]" << endl;
      
      for (index i=0; i<depth_ind_range; i++){
        for (index j=0; j<lat_ind_range; j++){
          for (index k=0; k<lon_ind_range; k++){

          if (preTEMP[i][j][k] != no_val_TEMP[0])
            TEMP[rec][i][j][k] = (preTEMP[i][j][k] * scale_factor_TEMP[0])
              + add_offset_TEMP[0];
          else
            TEMP[rec][i][j][k] = no_val_TEMP[0];

          if (preSALT[i][j][k] != no_val_SALT[0])
            SALT[rec][i][j][k] = (preSALT[i][j][k] * scale_factor_SALT[0])
              + add_offset_SALT[0];
          else
            SALT[rec][i][j][k] = no_val_SALT[0];
          }
        }
      }
    }


    //---------------------------------------------------------------
    // 5. ENSURE CORRECT UNITS
    //---------------------------------------------------------------
    NcVarAtt unitsAtt;
    string depthUnits, latUnits, lonUnits, saltUnits, tempUnits;

    //---------------------------------------------------------------
    // 5.1: Depth
    unitsAtt = depthVar.getAtt("units");
    if(unitsAtt.isNull()) return NC_ERR;
    
    unitsAtt.getValues(depthUnits);
    if (depthUnits != "m")
      {
        cout<<"WARNING! depth units = "<<depthUnits<<endl;
        return NC_ERR;
      }

    //---------------------------------------------------------------
    // 5.2: Latitude
    unitsAtt = latVar.getAtt("units");
    if(unitsAtt.isNull()) return NC_ERR;
    
    unitsAtt.getValues(latUnits);
    if (latUnits != "degrees_north")
      {
        cout<<"WARNING! latitude units = "<<latUnits<<endl;
        return NC_ERR;
      }
   
    //---------------------------------------------------------------
    // 5.3: Longitude
    unitsAtt = lonVar.getAtt("units");
    if(unitsAtt.isNull()) return NC_ERR;
   
    unitsAtt.getValues(lonUnits);
    if (lonUnits != "degrees_east")
      {
        cout<<"WARNING! longitude units = "<<lonUnits<<endl;
        return NC_ERR;
      }

    //---------------------------------------------------------------
    // 5.4: Salinity
    unitsAtt = saltVar.getAtt("units");
    if(unitsAtt.isNull()) return NC_ERR;
   
    unitsAtt.getValues(saltUnits);
    if (saltUnits != "psu")
      {
        cout<<"WARNING! salinity units = "<<saltUnits<<endl;
        return NC_ERR;
      }

    //---------------------------------------------------------------
    // 5.5: Temperature
    unitsAtt = tempVar.getAtt("units");
    if(unitsAtt.isNull()) return NC_ERR;
   
    unitsAtt.getValues(tempUnits);
    if (tempUnits != "degC")
      {
        cout<<"WARNING! temperature units = "<<tempUnits<<endl;
        return NC_ERR;
      }

    // The file will be automatically closed by the destructor. This
    // frees up any internal netCDF resources associated with the file,
    // and flushes any buffers.
    cout << "--------------------------------\n";
    cout << "* NETCDF DATA READ SUCCESSFUL! *\n";
    cout << "--------------------------------\n";
    cout << "ARRAYS CREATED:\n";
    cout << "  SALT[" << ntime << "][" << depth_ind_range << "]["
         << lat_ind_range << "][" << lon_ind_range << "]\n";
    cout << "  TEMP[" << ntime << "][" << depth_ind_range << "]["
         << lat_ind_range << "][" << lon_ind_range << "]\n";
    cout << endl;

    if (!newfile){
      return(0);
    }
    else{


    //---------------------------------------------------------------
    // 6. CREATE NEW NETCDF FILE
    //---------------------------------------------------------------
    cout << "--------------------------------\n";
    cout << "WRITING NEW NETCDF DATA FILE... \n";

    //---------------------------------------------------------------
    // 6.0.1: Define variable names
    string FILE_NAME  = "../data/salt_temp_4D.nc";
    string TIME_NAME  = "time";
    string DEPTH_NAME = "depth";
    string LAT_NAME   = "lat";
    string LON_NAME   = "lon";
    string SALT_NAME  = "salinity";
    string TEMP_NAME  = "water_temp";

    //---------------------------------------------------------------
    // 6.0.2: Define coordinate (independent) variable sizes
    int TIME_SIZE  = ntime;
    int DEPTH_SIZE = depth_ind_range;
    int LAT_SIZE   = lat_ind_range;
    int LON_SIZE   = lon_ind_range;

    //---------------------------------------------------------------
    // 6.0.3: Define variable units
    string TIME_UNITS  = "hours since 2000-01-01 00:00:00";
    string DEPTH_UNITS = "m";
    string LAT_UNITS   = latUnits;
    string LON_UNITS   = lonUnits;
    string SALT_UNITS  = saltUnits;
    string TEMP_UNITS  = tempUnits;

    //---------------------------------------------------------------
    // 6.0.4: Define variable scale and offsets
    float ADD_OFFSET = 20;
    float SCALE_FACTOR = 0.001;
    short NO_VALUE = -30000;

    cout << "-> Constants defined." << endl;
    
    //---------------------------------------------------------------
    // 6.1: Create NETCDF FILE
    //NcFile subsample(FILE_NAME, NcFile::replace, NcFile::nc4);
    NcFile subsample(FILE_NAME, NcFile::replace);

    if(subsample.isNull())
      return NC_ERR;

    cout << "-> NetCDF file allocated." << endl;

    //---------------------------------------------------------------
    // 6.2.1: Define DIMENSIONS
    NcDim timeDimOut  = subsample.addDim(TIME_NAME,  TIME_SIZE);
    NcDim depthDimOut = subsample.addDim(DEPTH_NAME, DEPTH_SIZE);
    NcDim latDimOut   = subsample.addDim(LAT_NAME,   LAT_SIZE);
    NcDim lonDimOut   = subsample.addDim(LON_NAME,   LON_SIZE);

    cout << "-> Dimensions allocated." << endl;

    //---------------------------------------------------------------
    // 6.2.2: Define coordinate (independent) VARIABLES
    NcVar timeVarOut  = subsample.addVar(TIME_NAME,  ncFloat, timeDimOut);
    NcVar depthVarOut = subsample.addVar(DEPTH_NAME, ncFloat, depthDimOut);
    NcVar latVarOut   = subsample.addVar(LAT_NAME,   ncFloat, latDimOut);
    NcVar lonVarOut   = subsample.addVar(LON_NAME,   ncFloat, lonDimOut);

    cout << "-> Coordinate variables allocated." << endl;

    //---------------------------------------------------------------
    // 6.2.3: Define data (independent) VARIABLES
    vector<NcDim> dimVector;
    dimVector.push_back(timeDimOut);
    dimVector.push_back(depthDimOut);
    dimVector.push_back(latDimOut);
    dimVector.push_back(lonDimOut);

    NcVar saltVarOut = subsample.addVar(SALT_NAME, ncShort, dimVector);
    NcVar tempVarOut = subsample.addVar(TEMP_NAME, ncShort, dimVector);

    cout << "-> Data variables allocated." << endl;

    //---------------------------------------------------------------
    // 6.3.1: Define GLOBAL ATTRIBUTES
    subsample.putAtt("classification_level", "UNCLASSIFIED");
    subsample.putAtt("distribution_statement",
                     "Approved for public release. Distribution unlimited.");
    subsample.putAtt("downgrade_date", "not applicable");
    subsample.putAtt("classification_authority", "not applicable");
    subsample.putAtt("institution", "Naval Oceanographic Office");
    subsample.putAtt("source", "HYCOM archive file");
    subsample.putAtt("history", "archv2ncdf2d");
    subsample.putAtt("comment", "p-grid");
    subsample.putAtt("field_type", "instantaneous");
    subsample.putAtt("Conventions", "CF-1.6 NAVO_netcdf_v1.1");

    cout << "-> Global attributes written." << endl;
    
    //---------------------------------------------------------------
    // 6.3.2: Define VARIABLE ATTRIBUTES
    // Time:
    timeVarOut.putAtt("long_name", "Valid Time");
    timeVarOut.putAtt("units", TIME_UNITS);
    timeVarOut.putAtt("time_origin", "2000-01-01 00:00:00");
    timeVarOut.putAtt("calendar", "gregorian");
    timeVarOut.putAtt("axis", "T");
    timeVarOut.putAtt("NAVO_code", ncInt, 13);

    // Depth:
    depthVarOut.putAtt("long_name", "Depth");
    depthVarOut.putAtt("standard_name", "depth");
    depthVarOut.putAtt("units", DEPTH_UNITS);
    depthVarOut.putAtt("positive", "down");
    depthVarOut.putAtt("axis", "Z");
    depthVarOut.putAtt("NAVO_code", ncInt, 5);

    // Lat:
    latVarOut.putAtt("long_name", "Latitude");
    latVarOut.putAtt("standard_name", "latitude");
    latVarOut.putAtt("units", LAT_UNITS);
    latVarOut.putAtt("axis", "Y");
    latVarOut.putAtt("NAVO_code", ncInt, 1);

    // Lon:
    lonVarOut.putAtt("long_name", "Longitude");
    lonVarOut.putAtt("standard_name", "longitude");
    lonVarOut.putAtt("units", LON_UNITS);
    lonVarOut.putAtt("modulo", "360 degrees");
    lonVarOut.putAtt("axis", "X");
    lonVarOut.putAtt("NAVO_code", ncInt, 2);

    // Salinity:
    saltVarOut.putAtt("_CoordinateAxes", "time depth lat lon");
    saltVarOut.putAtt("long_name", "Salinity");
    saltVarOut.putAtt("standard_name", "sea_water_salinity");
    saltVarOut.putAtt("units", SALT_UNITS);
    saltVarOut.putAtt("Fill_Value", ncShort, NO_VALUE);
    saltVarOut.putAtt("missing_value", ncShort, NO_VALUE);
    saltVarOut.putAtt("scale_factor", ncFloat, SCALE_FACTOR);
    saltVarOut.putAtt("add_offset", ncFloat, ADD_OFFSET);
    saltVarOut.putAtt("NAVO_code", ncInt, 16);

    // Temperature:
    tempVarOut.putAtt("_CoordinateAxes", "time depth lat lon");
    tempVarOut.putAtt("long_name", "Water Temperature");
    tempVarOut.putAtt("standard_name", "sea_water_temperature");
    tempVarOut.putAtt("units", TEMP_UNITS);
    tempVarOut.putAtt("Fill_Value", ncShort, NO_VALUE);
    tempVarOut.putAtt("missing_value", ncShort, NO_VALUE);
    tempVarOut.putAtt("scale_factor", ncFloat, SCALE_FACTOR);
    tempVarOut.putAtt("add_offset", ncFloat, ADD_OFFSET);
    tempVarOut.putAtt("NAVO_code", ncInt, 15);
    tempVarOut.putAtt("comment", "in-situ temperature");

    cout << "-> Variable attributes written." << endl;

    //---------------------------------------------------------------
    // 6.4.1: Fill coordinate (independent) VARIABLES
    float TIME_OUT[ntime];
    float DEPTH_OUT[depth_ind_range];
    float LAT_OUT[lat_ind_range];
    float LON_OUT[lon_ind_range];

    for (int rec=0; rec<ntime; rec++)
      TIME_OUT[rec] = TIME[time_ind_low+rec];
    for (int i=0; i<depth_ind_range; i++)
      DEPTH_OUT[i] = DEPTH[depth_ind_low+i];
    for (int j=0; j<lat_ind_range; j++)
      LAT_OUT[j] = LAT[lat_ind_low+j];
    for (int k=0; k<lon_ind_range; k++)
      LON_OUT[k] = LON[lon_ind_low+k];

    timeVarOut.putVar(TIME_OUT);
    depthVarOut.putVar(DEPTH_OUT);
    latVarOut.putVar(LAT_OUT);
    lonVarOut.putVar(LON_OUT);

    cout << "-> Coordinate variables written." << endl;

    //---------------------------------------------------------------
    // 6.4.1: Fill data (dependent) VARIABLES
    //short shortSALT[ntime][depth_ind_range][lat_ind_range][lon_ind_range];
    //short shortTEMP[ntime][depth_ind_range][lat_ind_range][lon_ind_range];
    
    vector<size_t> startp_write, countp_write;
    startp_write.push_back(0);
    startp_write.push_back(0);
    startp_write.push_back(0);
    startp_write.push_back(0);
    countp_write.push_back(1);
    countp_write.push_back(DEPTH_SIZE);
    countp_write.push_back(LAT_SIZE);
    countp_write.push_back(LON_SIZE);

    for (int rec=0; rec<ntime; rec++){
      startp_write[0] = rec;

      short shortSALT[depth_ind_range][lat_ind_range][lon_ind_range];
      short shortTEMP[depth_ind_range][lat_ind_range][lon_ind_range];

      for (int i=0; i<depth_ind_range; i++){
        for (int j=0; j<lat_ind_range; j++){
          for (int k=0; k<lon_ind_range; k++){

            if (TEMP[rec][i][j][k] != NO_VALUE)
              //shortTEMP[rec][i][j][k] = (TEMP[rec][i][j][k]-ADD_OFFSET)
              //  /SCALE_FACTOR;
              shortTEMP[i][j][k] = (TEMP[rec][i][j][k]-ADD_OFFSET)
                /SCALE_FACTOR;
            else
              //shortTEMP[rec][i][j][k] = NO_VALUE;
              shortTEMP[i][j][k] = NO_VALUE;

            if (SALT[rec][i][j][k] != NO_VALUE)
              //shortSALT[rec][i][j][k] = (SALT[rec][i][j][k]-ADD_OFFSET)
              //  /SCALE_FACTOR;
              shortSALT[i][j][k] = (SALT[rec][i][j][k]-ADD_OFFSET)
                /SCALE_FACTOR;
            else
              //shortSALT[rec][i][j][k] = NO_VALUE;
              shortSALT[i][j][k] = NO_VALUE;

          }
        }
      }

      saltVarOut.putVar(startp_write,countp_write,shortSALT);
      tempVarOut.putVar(startp_write,countp_write,shortTEMP);
    }

    cout << "-> Data variables written." << endl;

    // The file will be automatically closed by the destructor. This
    // frees up any internal netCDF resources associated with the file,
    // and flushes any buffers.
    cout << "--------------------------------\n";
    cout << "* NETCDF DATA WRITE SUCCESSFUL! *\n";
    cout << "---------------------------------\n";
    cout << endl;
    return(0);
    }
  }

  catch(NcException e){
    e.what();
    cout << "*** [FAIL] ***" << endl;
    return NC_ERR;
  }
}


//---------------------------------------------------------------
//---------------------------------------------------------------


//---------------------------------------------------------------
// HELPER FUNCTIONS
//---------------------------------------------------------------
// Find number of days between two given dates
// Adapted from Abhay Rathi of www.geeksforgeeks.org

// Count number of leap years before date d={yyyy, mm, dd}
int countLeapYears(date d)
{ 
    years = d.y;
    
    // Check if the current year needs to be considered 
    // for the count of leap years or not
    if (d.m <= 2)
        years--;
    
    // An year is a leap year if it is a multiple of 4,
    // multiple of 400 and not a multiple of 100.
    return (years/4) - (years/100) + (years/400);
}

// This function returns number of days between two dates 
int getDifference(date date1, date date2)
{
  // COUNT TOTAL NUMBER OF DAYS BEFORE FIRST DATE 'date1' 
  // initialize count using years and day
  n1 = date1.y*365 + date1.d;

  // Add days for months in given date
  for (int i=0; i<(date1.m - 1); i++)
    n1 += monthDays[i];

  // Since every leap year is of 366 days,
  // Add a day for every leap year
  n1 += countLeapYears(date1);

  // COUNT TOTAL NUMBER OF DAYS BEFORE 'date2'
  n2 = date2.y*365 + date2.d;
  for (int i=0; i<(date2.m - 1); i++)
    n2 += monthDays[i];
  n2 += countLeapYears(date2);

  hours = 24*(n2-n1);

  // return difference between two counts
  return (hours);
}
