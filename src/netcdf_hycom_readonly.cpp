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
    string NETCDF_DIR = "../data/salt_temp_4D.nc";
    NcFile dataFile(NETCDF_DIR, NcFile::read, NcFile::nc4);

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
    // 4.1: [REMOVED] NO USER INPUT; READS ENTIRE .NC FILE [REMOVED]

    //---------------------------------------------------------------
    // 4.2: Initialize 3D arrays
    short int preSALT[depth_size][lat_size][lon_size];
    short int preTEMP[depth_size][lat_size][lon_size];
    float SALT[time_size][depth_size][lat_size][lon_size];
    float TEMP[time_size][depth_size][lat_size][lon_size];

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
    startp.push_back(0);   //start: overwritten in step (4.5)
    startp.push_back(0);   //start: depth = low depth index
    startp.push_back(0);   //start: lat   = low lat index
    startp.push_back(0);   //start: lon   = low lon index
    countp.push_back(1);          //count: one record at a time
    countp.push_back(depth_size); //count: depth index range
    countp.push_back(lat_size);   //count: latitude index range
    countp.push_back(lon_size);   //count: longitude index range

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
    for (int rec=0; rec<time_size; rec++){
      startp[0] = rec;
      cout << "startp[0] = " << startp[0] << endl;
      cout << "countp[0] = " << countp[0] << endl;
      saltVar.getVar(startp,countp,preSALT);
      tempVar.getVar(startp,countp,preTEMP);
      
      for (int i=0; i<depth_size; i++){
        for (int j=0; j<lat_size; j++){
          for (int k=0; k<lon_size; k++){

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
          
          cout << "SALT[" << rec << "]["
               << i << "][" << j << "][" << k << "] = "
               <<  SALT[rec][i][j][k] << " psu" << endl;

          cout << "TEMP[" << rec << "]["
               << i << "][" << j << "][" << k << "] = "
               << TEMP[rec][i][j][k] << " degC" << endl;
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
    cout << "  SALT[" << time_size << "][" << depth_size << "]["
         << lat_size << "][" << lon_size << "]\n";
    cout << "  TEMP[" << time_size << "][" << depth_size << "]["
         << lat_size << "][" << lon_size << "]\n";
    cout << endl;

    return 0;
  }

  catch(NcException e){
    e.what();
    cout << "*** [FAIL] ***" << endl;
    return NC_ERR;
  }
}
