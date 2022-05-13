/*

    SLIDEM Processor: cdf_attrs.c

    Copyright (C) 2022  Johnathan K Burchill

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// Adapted from the Swarm Thermal Ion Imager Cross-track Ion Drift processor source code

#include "cdf_attrs.h"
#include "utilities.h"
#include "slidem_settings.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>

CDFstatus addgEntry(CDFid id, long attrNum, long entryNum, const char *entry)
{
    CDFstatus status = CDFputAttrgEntry(id, attrNum, entryNum, CDF_CHAR, strlen(entry), (void *)entry);
    return status;
}

CDFstatus addVariableAttributes(CDFid id, varAttr attr)
{
    CDFstatus status;
    char * variableName = attr.name;
    long varNum = CDFvarNum(id, variableName);
    status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "FIELDNAM"), varNum, CDF_CHAR, strlen(variableName), variableName);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        return status;
    }
    status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "LABLAXIS"), varNum, CDF_CHAR, strlen(variableName), variableName);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        return status;
    }
    status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "VAR_TYPE"), varNum, CDF_CHAR, 4, "data");
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        return status;
    }
    if (varNum != 0) // Everything but time
    {
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "TIME_BASE"), varNum, CDF_CHAR, 3, "N/A");
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "DISPLAY_TYPE"), varNum, CDF_CHAR, 11, "time_series");
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "DEPEND_0"), varNum, CDF_CHAR, 9, "Timestamp");
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
    }
    else // Add the time base to Time
    {
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "TIME_BASE"), varNum, CDF_CHAR, 3, "AD0");
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "DISPLAY_TYPE"), varNum, CDF_CHAR, 3, "N/A");
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "DEPEND_0"), varNum, CDF_CHAR, 3, "N/A");
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
    }
    status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "TYPE"), varNum, CDF_CHAR, strlen(attr.type), attr.type);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        return status;
    }
    if (attr.units[0] == '*')
    {
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "UNITS"), varNum, CDF_CHAR, 1, " ");
    }
    else
    {
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "UNITS"), varNum, CDF_CHAR, strlen(attr.units), attr.units);
    }
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        return status;
    }
    status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "CATDESC"), varNum, CDF_CHAR, strlen(attr.desc), attr.desc);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        return status;
    }

    // data type for valid min and max
    if (strcmp(attr.type, "CDF_EPOCH") == 0)
    {
        double val = attr.validMin;
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "VALIDMIN"), varNum, CDF_EPOCH, 1, &val);
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
        val = attr.validMax;
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "VALIDMAX"), varNum, CDF_EPOCH, 1, &val);
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
    }
    else if (strcmp(attr.type, "CDF_UINT2") == 0)
    {
        uint16_t val = (uint16_t) attr.validMin;
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "VALIDMIN"), varNum, CDF_UINT2, 1, &val);
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
        val = (uint16_t) attr.validMax;
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "VALIDMAX"), varNum, CDF_UINT2, 1, &val);
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
    }
    else if (strcmp(attr.type, "CDF_UINT4") == 0)
    {
        uint32_t val = (uint32_t) attr.validMin;
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "VALIDMIN"), varNum, CDF_UINT4, 1, &val);
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
        val = (uint32_t) attr.validMax;
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "VALIDMAX"), varNum, CDF_UINT4, 1, &val);
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
    }
    else if (strcmp(attr.type, "CDF_REAL8") == 0)
    {
        double val = (double) attr.validMin;
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "VALIDMIN"), varNum, CDF_REAL8, 1, &val);
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
        val = (double) attr.validMax;
        status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "VALIDMAX"), varNum, CDF_REAL8, 1, &val);
        if (status != CDF_OK)
        {
            printErrorMessage(status);
            return status;
        }
    }    
    status = CDFputAttrzEntry(id, CDFgetAttrNum(id, "FORMAT"), varNum, CDF_CHAR, strlen(attr.format), attr.format);
    if (status != CDF_OK)
    {
        printErrorMessage(status);
        return status;
    }

    return status;
}

void addAttributes(CDFid id, const char *softwareVersion, const char satellite, const char *version, double minTime, double maxTime)
{
    long attrNum;
    char buf[1000];

    // Global attributes
    CDFcreateAttr(id, "File_naming_convention", GLOBAL_SCOPE, &attrNum);
    sprintf(buf, "SW_%s_EFIXIDM", SLIDEM_PRODUCT_TYPE);
    addgEntry(id, attrNum, 0, buf);
    CDFcreateAttr(id, "Logical_file_id", GLOBAL_SCOPE, &attrNum);
    sprintf(buf, "swarm%c_IDM_H0__v%s", tolower(satellite), version);
    addgEntry(id, attrNum, 0, buf);
    CDFcreateAttr(id, "Logical_source", GLOBAL_SCOPE, &attrNum);
    sprintf(buf, "Swarm%c_IDM_H0", satellite);
    addgEntry(id, attrNum, 0, buf);
    CDFcreateAttr(id, "Logical_source_description", GLOBAL_SCOPE, &attrNum);
    sprintf(buf, "Swarm %c Ion Drift, Density and Effective Mass High resolution data product", satellite);
    addgEntry(id, attrNum, 0, buf);
    CDFcreateAttr(id, "Mission_group", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "Swarm");
    CDFcreateAttr(id, "MODS", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "Initial release.");
    CDFcreateAttr(id, "PI_name", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "Johnathan Burchill");   
    CDFcreateAttr(id, "PI_affiliation", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "University of Calgary");
    CDFcreateAttr(id, "Acknowledgement", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "ESA Swarm EFI IDM data are available from https://swarm-diss.eo.esa.int");
    CDFcreateAttr(id, "Source_name", GLOBAL_SCOPE, &attrNum);
    sprintf(buf, "Swarm%c>Swarm %c", satellite, satellite);
    addgEntry(id, attrNum, 0, buf);
    CDFcreateAttr(id, "Data_type", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "H0>High resolution data");
    CDFcreateAttr(id, "Data_version", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, version);
    CDFcreateAttr(id, "Descriptor", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "IDM>Swarm Ion Drift, Effective Mass and Revised Ion Density");
    CDFcreateAttr(id, "Discipline", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "Space Physics>Ionospheric Science");
    CDFcreateAttr(id, "Generated_by", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "University of Calgary");
    CDFcreateAttr(id, "Generation_date", GLOBAL_SCOPE, &attrNum);
    // Get rid of trailing newline from creation date
    time_t created;
    time(&created);
    struct tm * dp = gmtime(&created);
    char dateCreated[255] = { 0 };
    sprintf(dateCreated, "UTC=%04d-%02d-%02dT%02d:%02d:%02d", dp->tm_year+1900, dp->tm_mon+1, dp->tm_mday, dp->tm_hour, dp->tm_min, dp->tm_sec);
    addgEntry(id, attrNum, 0, dateCreated);
    CDFcreateAttr(id, "LINK_TEXT", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "2 Hz EFI IDM ion drift and effective mass data available at");
    CDFcreateAttr(id, "LINK_TITLE", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "ESA Swarm Data Access");
    CDFcreateAttr(id, "HTTP_LINK", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "https://swarm-diss.eo.esa.int");
    CDFcreateAttr(id, "Instrument_type", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "Electric Fields (space)");
    addgEntry(id, attrNum, 1, "Particles (space)");
    CDFcreateAttr(id, "Instrument_type", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "Plasma and Solar Wind");
    CDFcreateAttr(id, "TEXT", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "Swarm Langmuir Probe ion drift, effective mass, and revised ion density data.");
    addgEntry(id, attrNum, 1, "Along-track component of ion drift is parallel to the satellite velocity vector.");
    if (POST_PROCESS_ION_DRIFT)
    {
        addgEntry(id, attrNum, 2, "Ion drift has been adjusted to remove a high-latitude linear trend estimated from measurements between quasipole latitudes of 50 and 54 degrees on either side of each magnetic pole.");
    }
    else
    {
        addgEntry(id, attrNum, 2, "No offset removal has been performed on the ion drift. Large non-geophysical drifts are often present even at quasidipole latitudes near 50 degrees.");
    }
    if (MIEFF_FROM_TBT2015_MODEL)
    {
        addgEntry(id, attrNum, 3, "Ion along-track drift estimation assumes an ion effective mass estimated from the TBT-2015 high-altitude ion composition empirical model (CALION in IRI-2016).");
    }
    else
    {
        addgEntry(id, attrNum, 3, "Ion along-track drift estimation assumes an ion effective mass of 16.0 a.m.u.");
    }
    if (MODIFIED_OML_GEOMETRIES)
    {
        addgEntry(id, attrNum, 4, "Calculations use effective faceplate area and Langmuir probe radius estimated using modified OML expressions of Lira-Resendiz and Marchand.");
    }
    else
    {
        addgEntry(id, attrNum, 4, "Calculations use geometric faceplate area and Langmuir probe radius.");
    }
    addgEntry(id, attrNum, 5, "Product flag codes:\n\
1      Faceplate current unavailable\n\
2      IDM product calculation did not converge\n\
4      IDM product estimate is not finite and real\n\
8      IDM uncertainty estimate is not finite and real\n\
16     Modified OML faceplate area is not finite and real\n\
32     Modified OML LP probe radius is not finite and real\n\
64     QDLatitude is not within region of validity\n\
128    Modified OML faceplate area estimate is not valid\n\
256    Modified OML LP probe radius estimate is not valid\n\
512    IDM product estimate is large. Interpret with caution\n\
1024   IDM product estimate is small. Interpret with caution\n\
2048   Extended LP dataset inputs are invalid\n\
4096   LP Probe potentials differ by more than 0.3 V\n\
8192   Spacecraft potential is too negative\n\
16384  Spacecraft potential is too positive\n\
32768  Spacecraft velocity unavailable\n\
65536  Post processing error / post-processing not done\n\
131072 Magnetic field input invalid.");

    addgEntry(id, attrNum, 6, "Pakhotin, Burchill, Foerster and Lomidze. Swarm Langmuir Probe Ion Drift, Density, and Effective Mass (IDM) product validation. Submitted to Earth, Planets, Space.");
    addgEntry(id, attrNum, 7, "Knudsen, D.J., Burchill, J.K., Buchert, S.C., Eriksson, A.I., Gill, R., Wahlund, J.E., Ahlen, L., Smith, M. and Moffat, B., 2017. Thermal ion imagers and Langmuir probes in the Swarm electric field instruments. Journal of Geophysical Research: Space Physics, 122(2), pp.2655-2673.");
    CDFcreateAttr(id, "Time_resolution", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "0.5 seconds");
    CDFcreateAttr(id, "TITLE", GLOBAL_SCOPE, &attrNum);
    sprintf(buf, "Swarm %c IDM High resolution data.", satellite);
    addgEntry(id, attrNum, 0, buf);
    CDFcreateAttr(id, "Project", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, "ESA Living Planet Programme");
    CDFcreateAttr(id, "Software_version", GLOBAL_SCOPE, &attrNum);
    addgEntry(id, attrNum, 0, softwareVersion);
    CDFcreateAttr(id, "spase_DatasetResourceID", GLOBAL_SCOPE, &attrNum);
    sprintf(buf, "spase://ESA/Instrument/Swarm%c/IDM/0.5s", satellite);
    addgEntry(id, attrNum, 0, buf);

    CDFcreateAttr(id, "FIELDNAM", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "CATDESC", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "TYPE", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "UNITS", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "VAR_TYPE", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "DEPEND_0", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "DISPLAY_TYPE", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "LABLAXIS", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "VALIDMIN", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "VALIDMAX", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "FORMAT", VARIABLE_SCOPE, &attrNum);
    CDFcreateAttr(id, "TIME_BASE", VARIABLE_SCOPE, &attrNum);

    const varAttr variableAttrs[NUM_EXPORT_VARIABLES] = {
        {"Timestamp", "CDF_EPOCH", "*", "UT", minTime, maxTime, "%f"},
        {"Latitude", "CDF_REAL8", "degrees", "Geodetic latitude.", -90., 90., "%5.1f"},
        {"Longitude", "CDF_REAL8", "degrees", "Geodetic longitude.", -180., 180., "%6.1f"},
        {"Radius", "CDF_REAL8", "m", "Geocentric radius.", 6400000., 7400000., "%8.1f"},
        {"Height", "CDF_REAL8", "m", "Height above WGS84 reference ellipsoid.", 0., 1000000.0, "%8.1f"},
        {"QDLatitude", "CDF_REAL8", "degrees", "Quasi-dipole magnetic latitude.", -90., 90., "%5.1f"},
        {"MLT", "CDF_REAL8", "hour", "Magnetic local time.", 0., 24., "%4.1f"},
        {"V_sat_nec", "CDF_REAL8", "m/s", "Satellite velocity in north, east, centre (NEC) reference frame.", -10000.0, 10000.0, "%7.1f"},
        {"M_i_eff", "CDF_REAL8", "a.m.u.", "Ion effective mass.", FLAGS_MINIMUM_MIEFF, FLAGS_MAXIMUM_MIEFF, "%4.1f"},
        {"M_i_eff_err", "CDF_REAL8", "a.m.u.", "Ion effective mass uncertainty.", 0.0, FLAGS_MAXIMUM_MIEFF, "%3.1f"},
        {"M_i_eff_Flags", "CDF_UINT4", " ", "Ion effective mass validity flag.", 0, 65535, "%d"},
        {"M_i_eff_tbt_model", "CDF_REAL8", "a.m.u.", "Ion effective mass from Truhlik et al. (2015) topside empirical model.", 1.0, 40.0, "%4.1f"},
        {"V_i", "CDF_REAL8", "m/s", "Ion along-track drift.", -10000., 10000., "%7.1f"},
        {"V_i_err", "CDF_REAL8", "m/s", "Ion along-track drift uncertainty.", 0.0, 10000., "%5.1f"},
        {"V_i_Flags", "CDF_UINT4", " ", "Ion along-track drift validity flag.", 0, 65535, "%d"},
        {"V_i_raw", "CDF_REAL8", "m/s", "Ion along-track drift without high-latitude detrending.", -10000., 10000., "%7.1f"},
        {"N_i", "CDF_REAL8", "cm^-3", "Ion density.", FLAGS_MINIMUM_NI/1e6, FLAGS_MAXIMUM_NI/1e6, "%5.2g"},
        {"N_i_err", "CDF_REAL8", "cm^-3", "Ion density unertainty.", 0., 50000000.0, "%5.2gf"},
        {"N_i_Flags", "CDF_UINT4", " ", "Ion density validity flag.", 0, 65535, "%d"},
        {"A_fp", "CDF_REAL8", "m^2", "Modified-OML EFI faceplate area.", 0., 1., "%6.4f"},
        {"R_p", "CDF_REAL8", "m", "Modified-OML Langmuir spherical probe radius.", 0., 0.01, "%6.4f"},
        {"T_e", "CDF_REAL8", "K", "Electron temperature.", FLAGS_MINIMUM_LP_TE, FLAGS_MAXIMUM_LP_TE,"%7.1f"},
        {"Phi_sc", "CDF_REAL8", "V", "Spacecraft floating potential with respect to plasma potential far from satellite.", FLAGS_MINIMUM_LP_SPACECRAFT_POTENTIAL, FLAGS_MAXIMUM_LP_SPACECRAFT_POTENTIAL, "%5.1f"}
    };

    for (uint8_t i = 0; i < NUM_EXPORT_VARIABLES; i++)
    {
        addVariableAttributes(id, variableAttrs[i]);
    }

}


