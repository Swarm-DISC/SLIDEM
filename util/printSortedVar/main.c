/*

    SLIDEM Processor: util/printSortedVar/main.c

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

// CDF reading code is derived from SLIDEM load_inputs.c

#define SOFTWARE_VERSION "1.0"

#include <stdio.h>

#include <cdf.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <float.h>

int variables(const char *filename);
int load(const char *filename, char *variable, void **varData, long *count, long *variableBytes);

int compare(const void *a, const void *b)
{
	uint8_t ignorea = *(uint8_t*)(a+1);
	uint8_t ignoreb = *(uint8_t*)(b+1);

	// Place ignored data at the beginning
	if (ignorea > ignoreb)
		return -1;

	else if (ignorea < ignoreb)
		return 1;

	// Otherwise compare values

	uint8_t type = *(uint8_t*)a;

	double x, y;

	void *pa = (void*)a + 2;
	void *pb = (void*)b + 2;

	switch(type)
	{
		case CDF_REAL8:
		case CDF_DOUBLE:
		case CDF_EPOCH:
			x = *(double*)pa;
			y = *(double*)pb;
			break;

		case CDF_REAL4:
		case CDF_FLOAT:
			x = (double)*(float*)pa;
			y = (double)*(float*)pb;
			break;

		case CDF_UINT1:
		case CDF_UCHAR:
		case CDF_BYTE:
			x = (double)*(uint8_t*)pa;
			y = (double)*(uint8_t*)pb;
			break;

		case CDF_UINT2:
			x = (double)*(uint16_t*)pa;
			y = (double)*(uint16_t*)pb;
			break;

		case CDF_UINT4:
			x = (double)*(uint32_t*)pa;
			y = (double)*(uint32_t*)pb;
			break;

		case CDF_INT1:
		case CDF_CHAR:
			x = (double)*(int8_t*)pa;
			y = (double)*(int8_t*)pb;
			break;

		case CDF_INT2:
			x = (double)*(int16_t*)pa;
			y = (double)*(int16_t*)pb;
			break;

		case CDF_INT4:
			x = (double)*(int32_t*)pa;
			y = (double)*(int32_t*)pb;
			break;

		case CDF_INT8:
			x = (double)*(int64_t*)pa;
			y = (double)*(int64_t*)pb;
			break;

		default:
			x = 0.0;
			y = 0.0;
	}

	if (x > y)
		return 1;
	else if (x < y)
		return -1;
	else
		return 0;
}

void printValue(void *pointer, size_t index, long variableBytes)
{
	void *offset = pointer + index*(variableBytes + 2);
	uint8_t type = *(uint8_t*)offset;
	switch(type)
	{
		case CDF_REAL8:
		case CDF_DOUBLE:
		case CDF_EPOCH:
			printf("%lf", *(double*)(offset+2));
			break;

		case CDF_REAL4:
		case CDF_FLOAT:
			printf("%f", *(float*)(offset+2));
			break;

		case CDF_UINT1:
		case CDF_UCHAR:
		case CDF_BYTE:
			printf("%u", *(uint8_t*)(offset+2));
			break;

		case CDF_UINT2:
			printf("%u", *(uint16_t*)(offset+2));
			break;

		case CDF_UINT4:
			printf("%u", *(uint32_t*)(offset+2));
			break;

		case CDF_INT1:
		case CDF_CHAR:
			printf("%d", *(int8_t*)(offset+2));
			break;

		case CDF_INT2:
			printf("%d", *(int16_t*)(offset+2));
			break;

		case CDF_INT4:
			printf("%d", *(int32_t*)(offset+2));
			break;

		case CDF_INT8:
			printf("%ld", *(int64_t*)(offset+2));
			break;

		default:
			printf("x");
	}


}

int main(int argc, char *argv[])
{

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--about") == 0)
        {
            fprintf(stdout, "printSortedVar version %s.\n", SOFTWARE_VERSION);
            fprintf(stdout, "Copyright (C) 2022  Johnathan K Burchill\n");
            fprintf(stdout, "This program comes with ABSOLUTELY NO WARRANTY.\n");
            fprintf(stdout, "This is free software, and you are welcome to redistribute it\n");
            fprintf(stdout, "under the terms of the GNU General Public License.\n");
            exit(0);
        }
    }

	if (argc <2 || argc > 4)
	{
		printf("usage:\t%s cdffile\n\t\tprints list of varibles in cdffile.\n", argv[0]);
		printf("\t%s cdffile variable\n\t\tprints a sorted list (minimum to maximum) for the variable.\n", argv[0]);
		printf("\t%s cdffile variable ignoredValue\n\t\tprints list of varibles in cdffile.\n", argv[0]);
		printf("\t%s --about\n\t\tprints copyright and license information.\n", argv[0]);
		exit(0);
	}

	if (argc == 2)
	{
		variables(argv[1]);
		exit(0);
	}

	void *param = NULL;
	long count = 0;
	long variableBytes = 0;
	int status = load(argv[1], argv[2], &param, &count, &variableBytes);

	printf("%ld \"%s\" records\n", count, argv[2]);

	double ignored = -DBL_MAX;
	if (argc == 4)
		ignored = atof(argv[3]);

	bool ignore = false;
	long type = 0;
	void *offset = 0;
	for (size_t i = 0; i < count; i++)
	{
		offset = param + i * (variableBytes + 2);
		type = (long) (*(uint8_t*)offset);
		switch(type)
		{
			case CDF_DOUBLE:
			case CDF_REAL8:
			case CDF_EPOCH:
				ignore = (*(double*)(offset + 2) == ignored);
				break;

			case CDF_REAL4:
			case CDF_FLOAT:
				ignore = ((double)(*(float*)(offset + 2)) == ignored);
				break;

			case CDF_UINT1:
			case CDF_UCHAR:
			case CDF_BYTE:
				ignore = ((double)(*(uint8_t*)(offset + 2)) == ignored);
				break;

			case CDF_UINT2:
				ignore = ((double)(*(uint16_t*)(offset + 2)) == ignored);
				break;

			case CDF_UINT4:
				ignore = ((double)(*(uint32_t*)(offset + 2)) == ignored);
				break;

			case CDF_INT1:
			case CDF_CHAR:
				ignore = ((double)(*(int8_t*)(offset + 2)) == ignored);
				break;

			case CDF_INT2:
				ignore = ((double)(*(int16_t*)(offset + 2)) == ignored);
				break;

			case CDF_INT4:
				ignore = ((double)(*(int32_t*)(offset + 2)) == ignored);
				break;

			case CDF_INT8:
				ignore = ((double)(*(int64_t*)(offset + 2)) == ignored);
				break;

			default:
				ignore = 0;
		}
		if (ignore)
			*(uint8_t*)(offset+1) = 1;
	}

	qsort(param, count, variableBytes + 2, &compare);

	size_t index = 0;
	if (count > 0)
		while (((bool)(*(uint8_t*)(param + (index++) * (variableBytes + 2) + 1))) && index < count);
	if (index < count)
	{
		printf("Min:\t");
		printValue(param, index, variableBytes);
		printf("\n");
		printf("Max:\t");
		printValue(param, count-1, variableBytes);
		printf("\n");
	}

	for (size_t i = 0; i < count; i++)
	{
		offset = param + i * (variableBytes + 2); 
		if (*(uint8_t*)(offset+1) == 0)
		{
			printValue(param, i, variableBytes);
			printf("\n");
		}

	}


	free(param);

	return 0;

}

int variables(const char *filename)
{

    // Open the CDF file with validation
    CDFsetValidate(VALIDATEFILEoff);
    CDFid cdfId;
    CDFstatus status;
    // Attributes
    long attrN;
    long entryN;
    char attrName[CDF_ATTR_NAME_LEN256+1];
    long attrScope, maxEntry;

    // Check CDF info
    long decoding, encoding, majority, maxrRec, numrVars, maxzRec, numzVars, numAttrs, format;

    long numBytesToAdd, numVarBytes, numValues;
    long varNum, dataType, numElems, numRecs, numDims, recVary;
    long dimSizes[CDF_MAX_DIMS], dimVarys[CDF_MAX_DIMS];
    CDFdata data;

    status = CDFopenCDF(filename, &cdfId);
    if (status != CDF_OK) 
    {
        return status;
    }

    status = CDFgetFormat(cdfId, &format);
    status = CDFgetDecoding(cdfId, &decoding);
    status = CDFinquireCDF(cdfId, &numDims, dimSizes, &encoding, &majority, &maxrRec, &numrVars, &maxzRec, &numzVars, &numAttrs);
    if (status != CDF_OK)
    {
    	CDFcloseCDF(cdfId);
        return status;
    }
    uint8_t nVars = numzVars;
	char name[CDF_VAR_NAME_LEN256];
	for (int i = 0; i < nVars; i++)
	{
		status = CDFgetzVarName(cdfId, i, name);
		if (status != CDF_OK)
		{
			CDFcloseCDF(cdfId);
			CDFdataFree(data);
			return status;
		}
		printf("%s\n", name);
	}

	CDFcloseCDF(cdfId);

	return status;
    
}

int load(const char *filename, char *variable, void **varData, long *count, long *variableBytes)
{

    // Open the CDF file with validation
    CDFsetValidate(VALIDATEFILEoff);
    CDFid cdfId;
    CDFstatus status;
    // Attributes
    long attrN;
    long entryN;
    char attrName[CDF_ATTR_NAME_LEN256+1];
    long attrScope, maxEntry;

    // Check CDF info
    long decoding, encoding, majority, maxrRec, numrVars, maxzRec, numzVars, numAttrs, format;

    long numBytesToAdd, numVarBytes, numValuesPerRec;
    long varNum, dataType, numElems, numRecs, numDims, recVary;
    long dimSizes[CDF_MAX_DIMS], dimVarys[CDF_MAX_DIMS];
    CDFdata data;

    status = CDFopenCDF(filename, &cdfId);
    if (status != CDF_OK) 
    {
        return status;
    }

    status = CDFgetFormat(cdfId, &format);
    status = CDFgetDecoding(cdfId, &decoding);
    status = CDFinquireCDF(cdfId, &numDims, dimSizes, &encoding, &majority, &maxrRec, &numrVars, &maxzRec, &numzVars, &numAttrs);
    if (status != CDF_OK)
    {
    	CDFcloseCDF(cdfId);
        return status;
    }
    uint8_t nVars = numzVars;

	varNum = CDFgetVarNum(cdfId, variable);
	status = CDFreadzVarAllByVarID(cdfId, varNum, &numRecs, &dataType, &numElems, &numDims, dimSizes, &recVary, dimVarys, &data);
	if (status != CDF_OK)
	{
    	CDFcloseCDF(cdfId);
		CDFdataFree(data);
		return status;
	}
	// Calculate new size of memory to allocate
	status = CDFgetDataTypeSize(dataType, &numVarBytes);
	numValuesPerRec = 1;
	for (uint8_t j = 0; j < numDims; j++)
	{
		numValuesPerRec *= dimSizes[j];
	}
	numBytesToAdd = numValuesPerRec * numRecs * numVarBytes;
	// Prepend two bytes for each record to track the data type and whether the record should be ignored
	if (varData != NULL)
	{
		*varData = (void*) malloc((size_t) (numBytesToAdd + 2*numRecs*numValuesPerRec));
		if (*varData == NULL)
		{
			printf("Could not allocate heap.\n");
			CDFdataFree(data);
			CDFcloseCDF(cdfId);
			exit(42);
		}
		uint8_t *p = (uint8_t *)(*(uint8_t**)varData);
		bzero(p, (numBytesToAdd + numRecs*numValuesPerRec));
		for (size_t i = 0; i < numRecs * numValuesPerRec; i++)
		{
			p[i*(numVarBytes+2)] = (uint8_t)dataType;
			p[i*(numVarBytes+2) + 1] = 0;
			for (int j = 0; j < numVarBytes; j++)
			{
				p[i*(numVarBytes+2) + j + 2] = ((uint8_t*)data)[i*numVarBytes+j];
			}
		}
		
		// memcpy(*varData, data, numBytesToAdd);
	}
	CDFdataFree(data);


	CDFcloseCDF(cdfId);

	if (count != NULL)
	    *count += numRecs * numValuesPerRec;

	if (variableBytes != NULL)
		*variableBytes = numVarBytes;

	return status;
    
}
