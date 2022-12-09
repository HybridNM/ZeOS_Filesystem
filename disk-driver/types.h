#ifndef __TYPES_H__
#define __TYPES_H__

typedef unsigned char		Byte;	// 8 bits
typedef unsigned short int  Word;	// 16 bits
typedef unsigned long       DWord;	// 32 bits

#define highWord(address) (Word)(((address) >> 16) & 0xFFFF)
#define lowWord(address)  (Word)((address) & 0xFFFF)
#define midByte(address)  (Byte)(((address) >> 16) & 0xFF)
#define highByte(address) (Byte)(((address) >> (16 + 8)) & 0xFF)
#define high4Bits(limit)  (Byte)(((limit) >> 16) & 0x0F)

#define NULL 0

#endif /* __TYPES_H__ */
