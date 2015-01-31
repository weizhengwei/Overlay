#pragma once

unsigned int crc32(const char* InStr, unsigned int len);
unsigned short crc16(const char* InStr, unsigned int len);
unsigned int fast_crc32( unsigned int crc, const unsigned char  *buf,  unsigned int len );

