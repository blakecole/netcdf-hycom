/************************************************************/
/*    NAME: Blake Cole                                      */
/*    ORGN: MIT                                             */
/*    FILE: netcdf_hycom.cpp                                */
/*    DATE: 6 JULY 2018                                     */
/************************************************************/

#include <iostream>
#include <cstdlib>
#include <string>
#include <netcdf>
using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;

// Return this code to the OS in case of failure.
static const int NC_ERR = 2;

int main(int argc, char **argv){
  try{
    //---------------------------------------------------------------
    // 1. OPEN NETCDF FILE
    //---------------------------------------------------------------
    string dataURL = "http://tds.hycom.org/thredds/dodsC/GLBv0.08/expt_93.0";
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
    int tau_min, tau_max;
    float depth_min, depth_max;
    float lat_min, lat_max;
    float lon_min, lon_max;
    int nparams = 0;
    for (int i=1; i<argc; i++){
      string input;
      string argi = argv[i];
      cout << "argv[" << i << "] = " << argi << endl;
      if(argi.find("--time=current") == 0){
        tau_min = 0;
        tau_max = 0;
        nparams+=2;
      }
      else if(argi.find("--hours=") == 0){
        input = argi.substr(8);
        tau_min = 0;
        tau_max = atoi(input.c_str());
        nparams+=2;
      }
      else if(argi.find("--days=") == 0){
        input = argi.substr(7);
        tau_min = 0;
        tau_max = atoi(input.c_str())*24 - 1;
        nparams+=2;
      }
      else if(argi.find("--weeks=") == 0){
        input = argi.substr(8);
        tau_min = 0;
        tau_max = atoi(input.c_str())*24*7 - 1;
        nparams+=2;
      }
      else if(argi.find("--tmin=") == 0){
        input = argi.substr(7);
        tau_min = atoi(input.c_str());
        nparams++;
      }
      else if(argi.find("--tmax=") == 0){
        input = argi.substr(7);
        tau_max = atof(input.c_str());
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
    }

    //4.1.2: If command line fails, manually input bounds
    if (nparams != 8){
      cout << "\n(!) ERROR: EXACTLY (8) PARAMETERS REQUIRED (!)\n"
           << "                   (" << nparams << ") PARAMETERS PROVIDED.\n"
           << "PLEASE MANUALLY DEFINE SUBSET BOUNDS BELOW.\n" << endl;
      
      cout << "  1. Time Start [hours since analysis]" << " [0:" << time_size << "]: ";
      cin >> tau_max;

      if ((tau_max > time_size)||(tau_max < 0)){
        cout << "WARNING: VALUE MUST BE WITHIN ACCEPTABLE RANGE. TRY AGAIN.\n";
        cout << "  1. Time Start [hours since analysis]" << " [0:" << time_size << "]: ";
        cin >> tau_max;
      }
        
      cout << "  2. Time Stop [hours since analysis]" << " [0:" << time_size << "]: ";
      cin >> tau_min;
      if ((tau_min > time_size)||(tau_min < 0)){
        cout << "WARNING: VALUE MUST BE WITHIN ACCEPTABLE RANGE. TRY AGAIN.\n";
        cout << "  2. Time Stop [hours since analysis]" << " [0:" << time_size << "]: ";
        cin >> tau_min;
      }

      if (tau_min > tau_max){
        cout << "WARNING: 'Time Start' must exceed 'Time Stop'.\n";
        cout << "  Time Start [hours since analysis]"
             << " [0:" << time_size << "]: ";
        cin >> tau_max;
        cout << "  Time Stop [hours since analysis]"
             << " [0:" << time_size << "]: ";
        cin >> tau_min;
      }

      cout << "  3. Minimum Depth [meters] [" << DEPTH[0] << ":" << DEPTH[depth_size-1] << "]: ";
      cin >> depth_min;
      if ((depth_min < DEPTH[0])||(depth_min > DEPTH[depth_size-1])){
        cout << "WARNING: VALUE MUST BE WITHIN ACCEPTABLE RANGE. TRY AGAIN.\n";
        cout << "  3. Minimum Depth [meters] [" << DEPTH[0] << ":" << DEPTH[depth_size-1] << "]: ";
        cin >> depth_min;
      }
      cout << "  4. Maximum Depth [meters] [" << DEPTH[0] << ":" << DEPTH[depth_size-1] << "]: ";
      cin >> depth_max;
      if ((depth_max < DEPTH[0])||(depth_max > DEPTH[depth_size-1])){
        cout << "WARNING: VALUE MUST BE WITHIN ACCEPTABLE RANGE. TRY AGAIN.\n";
        cout << "  4. Maximum Depth [meters] [" << DEPTH[0] << ":" << DEPTH[depth_size-1] << "]: ";
        cin >> depth_max;
      }

      cout << "  5. Minimum Latitude [degrees] [" << LAT[0] << ":" << LAT[lat_size-1] << "]: ";
      cin >> lat_min;
      if ((lat_min < LAT[0])||(lat_min > LAT[lat_size-1])){
        cout << "WARNING: VALUE MUST BE WITHIN ACCEPTABLE RANGE. TRY AGAIN.\n";
        cout << "  5. Minimum Latitude [degrees] [" << LAT[0] << ":" << LAT[lat_size-1] << "]: ";
        cin >> lat_min;
      }
      cout << "  6. Maximum Latitude [degrees] [" << LAT[0] << ":" << LAT[lat_size-1] << "]: ";
      cin >> lat_max;
      if ((lat_max < LAT[0])||(lat_max > LAT[lat_size-1])){
        cout << "WARNING: VALUE MUST BE WITHIN ACCEPTABLE RANGE. TRY AGAIN.\n";
        cout << "  6. Maximum Latitude [degrees] [" << LAT[0] << ":" << LAT[lat_size-1] << "]: ";
        cin >> lat_max;
      }
        
      cout << "  7. Minimum Longitude [degrees] [" << LON[0] << ":" << LON[lon_size-1] << "]: ";
      cin >> lon_min;
      if ((lon_min < LON[0])||(lon_min > LON[lon_size-1])){
        cout << "WARNING: VALUE MUST BE WITHIN ACCEPTABLE RANGE. TRY AGAIN.\n";
        cout << "  7. Minimum Longitude [degrees] [" << LON[0] << ":" << LON[lon_size-1] << "]: ";
        cin >> lon_min;
      }
        
      cout << "  8. Maximum Longitude [degrees] [" << LON[0] << ":" << LON[lon_size-1] << "]: ";
      cin >> lon_max;
      if ((lon_max < LON[0])||(lon_max > LON[lon_size-1])){
        cout << "WARNING: VALUE MUST BE WITHIN ACCEPTABLE RANGE. TRY AGAIN.\n";
        cout << "  8. Maximum Longitude [degrees] [" << LON[0] << ":" << LON[lon_size-1] << "]: ";
        cin >> lon_max;
      }
              
    }

    // 4.1.3: Search for closest HYCOM values
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

    cout << "-----------------------\n";
    cout << "SPATIAL RANGE:" << endl;
    cout << "Requested:\n";
    cout << "  LAT   = " << lat_min << ":" << lat_max << "\n";
    cout << "  LON   = " << lon_min << ":" << lon_max << "\n";
    cout << "  DEPTH = " << depth_min << ":" << depth_max << "\n";
    
    cout << "Actual:\n";
    cout << "  LAT[" << lat_ind_low << ":" << lat_ind_high << "] = "
         << LAT[lat_ind_low] << ":" << LAT[lat_ind_high] << "\n";
    cout << "  LON[" << lon_ind_low << ":" << lon_ind_high << "] = "
         << LON[lon_ind_low] << ":" << LON[lon_ind_high] << "\n";
    cout << "  DEPTH[" << depth_ind_low << ":" << depth_ind_high << "] = "
         << DEPTH[depth_ind_low] << ":" << DEPTH[depth_ind_high] << "\n";
    cout << endl;

    // Index Ranges (+1 for inclusive)
    int lat_ind_range = lat_ind_high - lat_ind_low +1;
    int lon_ind_range = lon_ind_high - lon_ind_low +1;
    int depth_ind_range = depth_ind_high - depth_ind_low +1;
    int ntime = tau_max - tau_min + 1;

    //---------------------------------------------------------------
    // 4.2: Initialize 3D arrays
    short int preSALT[depth_ind_range][lat_ind_range][lon_ind_range];
    short int preTEMP[depth_ind_range][lat_ind_range][lon_ind_range];
    float SALT[ntime][depth_ind_range][lat_ind_range][lon_ind_range];
    float TEMP[ntime][depth_ind_range][lat_ind_range][lon_ind_range];

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
    for (int rec=0; rec<ntime; rec++){
      startp[0] = (time_size-1) - tau_max + rec;
      cout << "startp[0] = " << startp[0] << endl;
      cout << "countp[0] = " << countp[0] << endl;
      saltVar.getVar(startp,countp,preSALT);
      tempVar.getVar(startp,countp,preTEMP);
      
      for (int i=0; i<depth_ind_range; i++){
        for (int j=0; j<lat_ind_range; j++){
          for (int k=0; k<lon_ind_range; k++){

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
          
          cout << "SALT[" << rec << "][" << i << "][" << j << "][" << k << "] = "
               <<  SALT[rec][i][j][k] << " psu" << endl;

          cout << "TEMP[" << rec << "][" << i << "][" << j << "][" << k << "] = "
               << TEMP[rec][i][j][k] << " degC" << endl;
          }
        }
      }
    }


    //---------------------------------------------------------------
    // 5. ENSURE CORRECT UNITS
    //---------------------------------------------------------------
    NcVarAtt unitsAtt;
    string units;

    //---------------------------------------------------------------
    // 5.1: Latitude
    unitsAtt = latVar.getAtt("units");
    if(unitsAtt.isNull()) return NC_ERR;
    
    unitsAtt.getValues(units);
    if (units != "degrees_north")
      {
        cout<<"WARNING! latitude units = "<<units<<endl;
        return NC_ERR;
      }
   
    //---------------------------------------------------------------
    // 5.2: Longitude
    unitsAtt = lonVar.getAtt("units");
    if(unitsAtt.isNull()) return NC_ERR;
   
    unitsAtt.getValues(units);
    if (units != "degrees_east")
      {
        cout<<"WARNING! longitude units = "<<units<<endl;
        return NC_ERR;
      }

    //---------------------------------------------------------------
    // 5.3: Salinity
    unitsAtt = saltVar.getAtt("units");
    if(unitsAtt.isNull()) return NC_ERR;
   
    unitsAtt.getValues(units);
    if (units != "psu")
      {
        cout<<"WARNING! salinity units = "<<units<<endl;
        return NC_ERR;
      }

    //---------------------------------------------------------------
    // 5.4: Temperature
    unitsAtt = tempVar.getAtt("units");
    if(unitsAtt.isNull()) return NC_ERR;
   
    unitsAtt.getValues(units);
    if (units != "degC")
      {
        cout<<"WARNING! temperature units = "<<units<<endl;
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
    return 0;
  }
  catch(NcException e){
    e.what();
    cout << "*** [FAIL] ***" << endl;
    return NC_ERR;
  }
}
