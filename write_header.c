/*

    SLIDEM Processor: write_header.c

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

#include "write_header.h"
#include "slidem_settings.h"
#include "utilities.h"

#include <libxml/xmlwriter.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>
#include <cdf.h>

int writeSlidemHeader(const char *slidemFilename, const char *fpFilename, const char *hmFilename, const char *modFilename, const char *modFilenamePrevious, const char *magFilename, time_t processingStartTime, double firstMeasurementTime, double lastMeasurementTime, long nVnecRecsPrev)
{

    // Level 2 product ZIP file neads a HDR file.
    size_t sLen = strlen(slidemFilename);
    if (sLen < SLIDEM_BASE_FILENAME_LENGTH)
        return HEADER_FILENAME;

    char slidemCdfFilename[FILENAME_MAX];
    snprintf(slidemCdfFilename, sLen + 5, "%s.cdf", slidemFilename);
    char sizeString[22];
    struct stat fileInfo;
    int statstatus = stat(slidemCdfFilename, &fileInfo);
    if (statstatus == -1)
    {
        perror(NULL);
        return HEADER_CDFFILEINFO;
    }
    sprintf(sizeString, "%+021d", (int)fileInfo.st_size);

    char headerFilename[FILENAME_MAX];
    snprintf(headerFilename, sLen + 5, "%s.HDR", slidemFilename);

    char creationDate[UTC_DATE_LENGTH];
    utcDateString(processingStartTime, creationDate);
    char nowDate[UTC_DATE_LENGTH];
    utcNowDateString(nowDate);

    char validityStart[UTC_DATE_LENGTH];
    double firstMeasurementTimeUnix = 0;
    EPOCHtoUnixTime(&firstMeasurementTime, &firstMeasurementTimeUnix, 1);
    time_t validityStartTime = (time_t) floor(firstMeasurementTimeUnix);
    utcDateString(validityStartTime, validityStart);
    char validityStop[UTC_DATE_LENGTH];
    double lastMeasurementTimeUnix = 0;
    EPOCHtoUnixTime(&lastMeasurementTime, &lastMeasurementTimeUnix, 1);
    time_t validityStopTime = (time_t) floor(lastMeasurementTimeUnix);
    utcDateString(validityStopTime, validityStop);

    char sensingStart[UTC_DATE_LENGTH + 7];
    utcDateStringWithMicroseconds(firstMeasurementTimeUnix, sensingStart);
    char sensingStop[UTC_DATE_LENGTH + 7];
    utcDateStringWithMicroseconds(lastMeasurementTimeUnix, sensingStop);


    // CRC of all records in CDF file (Table 5-1 entry 1.21 of L1b product specification \cite{esal1bspec}).
    // In contrast with a DBL file, this is not computed for CDF
    // which contains additional info besides record values)
    char crcString[7];
    sprintf(crcString, "%+06d", -1);

    int status = HEADER_OK;
    int bytes = 0;

    xmlTextWriterPtr hdr = xmlNewTextWriterFilename(headerFilename, 0);
    if (hdr == NULL)
        return HEADER_CREATE;

    bytes = xmlTextWriterStartDocument(hdr, "1.0", "UTF-8", "no");
    if (bytes == -1)
    {
        status = HEADER_START;
        goto cleanup;
    }
    xmlTextWriterSetIndent(hdr, 1);
    xmlTextWriterSetIndentString(hdr, "  ");

    bytes = xmlTextWriterStartElement(hdr, "Earth_Explorer_Header");
    if (bytes == -1)
    {
        status = HEADER_WRITE_ERROR;
        goto cleanup;
    }
    xmlTextWriterWriteAttribute(hdr, "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");


        // <Fixed_Header>
        xmlTextWriterStartElement(hdr, "Fixed_Header");

            xmlTextWriterWriteElement(hdr, "FileName", slidemFilename + sLen - SLIDEM_BASE_FILENAME_LENGTH);

            xmlTextWriterWriteElement(hdr, "File_Description", "Swarm Langmuir probe ion drift, density and effective mass product.");

            xmlTextWriterStartElement(hdr, "Notes");
            xmlTextWriterEndElement(hdr);

            xmlTextWriterWriteElement(hdr, "Mission", "Swarm");

            xmlTextWriterWriteElement(hdr, "File_Class", SLIDEM_PRODUCT_TYPE);
            xmlTextWriterWriteElement(hdr, "File_Type", SLIDEM_FILE_TYPE);

            xmlTextWriterStartElement(hdr, "Validity_Period");
                xmlTextWriterWriteElement(hdr, "Validty_Start", validityStart);
                xmlTextWriterWriteElement(hdr, "Validty_Stop", validityStop);
            xmlTextWriterEndElement(hdr);

            xmlTextWriterWriteElement(hdr, "File_Version", EXPORT_VERSION_STRING);

            xmlTextWriterStartElement(hdr, "Source");

                xmlTextWriterWriteElement(hdr, "System", "SPC");
                xmlTextWriterWriteElement(hdr, "Creator", "SPC_UOC");
                xmlTextWriterWriteElement(hdr, "Creator_Version", SOFTWARE_VERSION);
                xmlTextWriterWriteElement(hdr, "Creation_Date", creationDate);

            xmlTextWriterEndElement(hdr);


        // </Fixed_Header>
        xmlTextWriterEndElement(hdr);


        // <Variable_Header>
        xmlTextWriterStartElement(hdr, "Variable_Header");

            xmlTextWriterStartElement(hdr, "MPH");
                xmlTextWriterWriteElement(hdr, "Product", slidemFilename + sLen - SLIDEM_BASE_FILENAME_LENGTH);
                xmlTextWriterWriteElement(hdr, "Product_Format", "CDF");
                xmlTextWriterWriteElement(hdr, "Proc_Stage_Code", SLIDEM_PRODUCT_TYPE);
                xmlTextWriterWriteElement(hdr, "Ref_Doc", "SW-DS-DTU-GS-0001");
                xmlTextWriterWriteElement(hdr, "Proc_Center", "UOC");
                xmlTextWriterWriteElement(hdr, "Proc_Time", creationDate);
                xmlTextWriterWriteElement(hdr, "Software_Version", "UOC_SLIDEM/" SOFTWARE_VERSION);
                xmlTextWriterWriteElement(hdr, "Product_Err", "0");

                xmlTextWriterStartElement(hdr, "Tot_Size");
                    xmlTextWriterWriteAttribute(hdr, "unit", "bytes");
                    xmlTextWriterWriteString(hdr, sizeString);
                xmlTextWriterEndElement(hdr);
                xmlTextWriterWriteElement(hdr, "CRC", crcString);                

            xmlTextWriterEndElement(hdr);


            xmlTextWriterStartElement(hdr, "SPH");
                xmlTextWriterWriteElement(hdr, "SPH_Descriptor", SLIDEM_FILE_TYPE);                
                xmlTextWriterWriteElement(hdr, "Original_Filename", slidemCdfFilename + sLen - SLIDEM_BASE_FILENAME_LENGTH);

            xmlTextWriterStartElement(hdr, "Sensing_Time_Interval");
                xmlTextWriterWriteElement(hdr, "Sensing_Start", sensingStart);
                xmlTextWriterWriteElement(hdr, "Sensing_Stop", sensingStop);
            xmlTextWriterEndElement(hdr);

            xmlTextWriterStartElement(hdr, "Product_Confidence_Data");
                xmlTextWriterWriteElement(hdr, "Quality_Indicator", "000");
            xmlTextWriterEndElement(hdr);

            xmlTextWriterStartElement(hdr, "Product_Confidence_Data");
                xmlTextWriterStartElement(hdr, "List_of_Input_File_Names");

                    int nInputFiles = 6;
                    if (nVnecRecsPrev > 0)
                        nInputFiles = 7;
                    char nInputFilesStr[20];
                    sprintf(nInputFilesStr, "%d", nInputFiles);
                    xmlTextWriterWriteAttribute(hdr, "count", nInputFilesStr);
                    xmlTextWriterWriteElement(hdr, "File_Name", fpFilename + strlen(fpFilename) - FP_FILENAME_LENGTH);
                    xmlTextWriterWriteElement(hdr, "File_Name", hmFilename + strlen(hmFilename) - HM_FILENAME_LENGTH);
                    if (nVnecRecsPrev > 0)
                        xmlTextWriterWriteElement(hdr, "File_Name", modFilenamePrevious + strlen(modFilenamePrevious) - MOD_FILENAME_LENGTH);
                    xmlTextWriterWriteElement(hdr, "File_Name", modFilename + strlen(modFilename) - MOD_FILENAME_LENGTH);
                    xmlTextWriterWriteElement(hdr, "File_Name", magFilename + strlen(magFilename) - MAG_FILENAME_LENGTH);
                    xmlTextWriterWriteElement(hdr, "File_Name", "apf107.dat");
                    xmlTextWriterWriteElement(hdr, "File_Name", ".slidem_modified_oml_configrc");

                xmlTextWriterEndElement(hdr);

                xmlTextWriterStartElement(hdr, "List_of_Output_File_Names");

                    xmlTextWriterWriteAttribute(hdr, "count", "1");
                    xmlTextWriterWriteElement(hdr, "File_Name", slidemCdfFilename + sLen - SLIDEM_BASE_FILENAME_LENGTH);

                xmlTextWriterEndElement(hdr);

            xmlTextWriterEndElement(hdr);

            xmlTextWriterEndElement(hdr);

        // </Variable_Header>
        xmlTextWriterEndElement(hdr);

    // </Earth_Explorer_Header>
    xmlTextWriterEndElement(hdr);

    xmlTextWriterEndDocument(hdr);
    bytes = xmlTextWriterFlush(hdr);

cleanup:
    xmlFreeTextWriter(hdr);

    return status;    
}
