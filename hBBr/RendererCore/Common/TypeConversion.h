#pragma once
#include "Common.h"
typedef unsigned char   byte;
typedef short   int16;
typedef unsigned short   uint16;

#define RGB888_RED      0x00ff0000
#define RGB888_GREEN    0x0000ff00
#define RGB888_BLUE     0x000000ff

#define RGB565_RED      0xf800
#define RGB565_GREEN    0x07E0
#define RGB565_BLUE     0x001f

/*
要点提示:
1. float和unsigned long具有相同的数据结构长度
2. union据类型里的数据存放在相同的物理空间
*/
typedef union
{
	float fdata;
	unsigned long ldata;
}FloatLongType;

namespace TypeConversion
{
	static int bytesToInt(uint8_t* bytes)
	{
		int a = bytes[0] & 0xFF;
		a |= ((bytes[1] << 8) & 0xFF00);
		a |= ((bytes[2] << 16) & 0xFF0000);
		a |= ((bytes[3] << 24) & 0xFF000000);
		return a;
	}

	static unsigned int bytesToUInt(uint8_t* bytes)
	{
		unsigned int a = bytes[0] & 0xFF;
		a |= ((bytes[1] << 8) & 0xFF00);
		a |= ((bytes[2] << 16) & 0xFF0000);
		a |= ((bytes[3] << 24) & 0xFF000000);
		return a;
	}

	static int16 bytesToInt16(uint8_t* bytes)
	{
		int16 a = bytes[0] & 0xFF;
		a |= ((bytes[1] << 8) & 0xFF00);
		return a;
	}

	static uint16 bytesToUInt16(uint8_t* bytes)
	{
		uint16 a = bytes[0] & 0xFF;
		a |= ((bytes[1] << 8) & 0xFF00);
		return a;
	}

	static uint8_t* UIntToBytes(unsigned int ui)
	{
		uint8_t* src = new uint8_t[4];
		src[3] = (uint8_t)((ui >> 24) & 0xFF);
		src[2] = (uint8_t)((ui >> 16) & 0xFF);
		src[1] = (uint8_t)((ui >> 8) & 0xFF);
		src[0] = (uint8_t)(ui & 0xFF);
		return src;
	}

	static uint8_t* IntToBytes(int ui)
	{
		uint8_t* src = new uint8_t[4];
		src[3] = (uint8_t)((ui >> 24) & 0xFF);
		src[2] = (uint8_t)((ui >> 16) & 0xFF);
		src[1] = (uint8_t)((ui >> 8) & 0xFF);
		src[0] = (uint8_t)(ui & 0xFF);
		return src;
	}

	/*
	将浮点数f转化为4个字节数据存放在uint8_t[4]中
	*/
	static void Float_to_Byte(float f, unsigned char uint8_t[])
	{
		FloatLongType fl;
		fl.fdata = f;
		uint8_t[0] = (unsigned char)fl.ldata;
		uint8_t[1] = (unsigned char)(fl.ldata >> 8);
		uint8_t[2] = (unsigned char)(fl.ldata >> 16);
		uint8_t[3] = (unsigned char)(fl.ldata >> 24);
	}
	/*
	将4个字节数据byte[4]转化为浮点数存放在*f中
	*/
	static void Byte_to_Float(float* f, unsigned char byte[])
	{
		FloatLongType fl;
		fl.ldata = 0;
		fl.ldata = byte[0];
		fl.ldata = (fl.ldata << 8) | byte[1];
		fl.ldata = (fl.ldata << 8) | byte[2];
		fl.ldata = (fl.ldata << 8) | byte[3];
		*f = fl.fdata;
	}

	static uint8_t* UInt16ToBytes(uint16 ui)
	{
		uint8_t* src = new uint8_t[2];
		src[1] = (uint8_t)((ui >> 8) & 0xFF);
		src[0] = (uint8_t)(ui & 0xFF);
		return src;
	}

	static uint8_t* Int16ToBytes(int16 ui)
	{
		uint8_t* src = new uint8_t[2];
		src[1] = (uint8_t)((ui >> 8) & 0xFF);
		src[0] = (uint8_t)(ui & 0xFF);
		return src;
	}

	////////////////
	struct RGBA
	{
		uint8_t R = 0xff;
		uint8_t G = 0xff;
		uint8_t B = 0xff;
		uint8_t A = 0xff;
	};

	struct BYTE_16bit
	{
		uint8_t B1 = 0x0;
		uint8_t B2 = 0x0;
	};

	static RGBA ConversionByteToRGB555A1(uint8_t* b5g5r5a1)
	{
		uint8_t b0 = b5g5r5a1[1];
		uint8_t b1 = b5g5r5a1[0];
		RGBA out;
		out.R = b0 >>2;
		out.G = (b0 << 3 | b1 >> 5) & 0x1f;
		out.B = b1 & 0x1f;
		out.A = 0;
		return out;
	}

	static BYTE_16bit ConversionRGB555A1ToByte(RGBA rgb5)
	{
		BYTE_16bit out;
		out.B1 = (rgb5.G << 5) & 0xe0 | rgb5.B;
		out.B2 = (rgb5.R << 2) | (rgb5.G>>3);
		return out;
	}

}