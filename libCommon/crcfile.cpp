#include "crcfile.h"


CRCFile::CRCFile()
{
	initCRCTable();
  
}

// Open the file, read it, and obtain the CRC from getCRC()
bool CRCFile::calcCRC(char* fileName, unsigned long* CRC)
{
	bool res = true;

	HANDLE hFile = {NULL};
	DWORD dwSize, bytes_read;

    unsigned char*  pData = NULL;

	// Open the file and get ready to read it.
	// Don't be confused by the CreateFile() function,
	// we're only opening an existing file.
	hFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ /*| FILE_SHARE_WRITE*/,
		NULL, OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		res = false;
		return res ;
	}
	// Get the file size so we know how long to make our buffer.
	dwSize = GetFileSize(hFile, NULL);

	pData = new unsigned char[dwSize];

	// Read the file into the buffer
	ReadFile(hFile, pData, dwSize, &bytes_read, NULL);
	// We're done with the file handle so close it.
	CloseHandle(hFile);
	// Check to be sure we read the whole file.
	if(dwSize != bytes_read)
	{
		res = false;
        delete pData;
		return res;
	}
	*CRC = getCRC(pData, dwSize);

	delete pData;


	return res ;

}




void CRCFile::initCRCTable()
{
	// This is the official polynomial used by CRC-32 
	// in PKZip, WinZip and Ethernet. 
	unsigned long ulPolynomial = 0x04c11db7;

	// 256 values representing ASCII character codes.
	for(int i = 0; i <= 0xFF; i++)
	{
		crc32_table[i]=reflect(i, 8) << 24;
		for (int j = 0; j < 8; j++)
			crc32_table[i] = (crc32_table[i] << 1) ^ (crc32_table[i] & (1 << 31) ? ulPolynomial : 0);
		crc32_table[i] = reflect(crc32_table[i], 32);
	}
}

// Reflection is a requirement for the official CRC-32 standard.
// You can create CRCs without it, but they won't conform to the standard.
unsigned long CRCFile::reflect(unsigned long ref, char ch)
{

	unsigned long value(0);

	// Swap bit 0 for bit 7
	// bit 1 for bit 6, etc.
	for(int i = 1; i < (ch + 1); i++)
	{
		if(ref & 1)
			value |= 1 << (ch - i);
		ref >>= 1;
	}
	return value;
}



// This function uses the crc32_table lookup table
// to generate a CRC for csData
unsigned long CRCFile::getCRC(unsigned char* buffer, DWORD dwSize)
{
	// Be sure to use unsigned variables,
	// because negative values introduce high bits
	// where zero bits are required.
	unsigned long  crc(0xffffffff);
	int len;
	//unsigned char* buffer;

	len = dwSize;
	// Save the text in the buffer.
	//buffer = (unsigned char*)(LPCTSTR)csData;
	// Perform the algorithm on each character
	// in the string, using the lookup table values.
	while(len--)
		crc = (crc >> 8) ^ crc32_table[(crc & 0xFF) ^ *buffer++];
	// Exclusive OR the result with the beginning value.
	return crc^0xffffffff;
}


