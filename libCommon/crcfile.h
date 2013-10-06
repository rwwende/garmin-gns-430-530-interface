#ifndef __CRCFILE__
#define __CRCFILE__

#include <windows.h>

class CRCFile 
{
public:
	CRCFile();	

	bool calcCRC(char* fileName, unsigned long* CRC); // Opens the file and reads it
	
private:
	void initCRCTable(); // Builds Lookup table array
	unsigned long reflect(unsigned long ref, char ch); // Reflects CRC bits in the lookup table
	unsigned long  getCRC(unsigned char* buffer, unsigned long dwSize); // Creates a CRC from a string buffer

private:
    unsigned long crc32_table[256]; // Lookup table array

};


#endif // __CRCFILE__
