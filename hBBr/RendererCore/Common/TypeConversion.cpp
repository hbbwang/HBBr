#include "TypeConversion.h"

namespace TypeConversion
{
	int bytesToInt(uint8_t* bytes)
	{
		int a = bytes[0] & 0xFF;
		a |= ((bytes[1] << 8) & 0xFF00);
		a |= ((bytes[2] << 16) & 0xFF0000);
		a |= ((bytes[3] << 24) & 0xFF000000);
		return a;
	}

	unsigned int bytesToUInt(uint8_t* bytes)
	{
		unsigned int a = bytes[0] & 0xFF;
		a |= ((bytes[1] << 8) & 0xFF00);
		a |= ((bytes[2] << 16) & 0xFF0000);
		a |= ((bytes[3] << 24) & 0xFF000000);
		return a;
	}

	int16 bytesToInt16(uint8_t* bytes)
	{
		int16 a = bytes[0] & 0xFF;
		a |= ((bytes[1] << 8) & 0xFF00);
		return a;
	}

	uint16 bytesToUInt16(uint8_t* bytes)
	{
		uint16 a = bytes[0] & 0xFF;
		a |= ((bytes[1] << 8) & 0xFF00);
		return a;
	}

	uint8_t* UIntToBytes(unsigned int ui)
	{
		uint8_t* src = new uint8_t[4];
		src[3] = (uint8_t)((ui >> 24) & 0xFF);
		src[2] = (uint8_t)((ui >> 16) & 0xFF);
		src[1] = (uint8_t)((ui >> 8) & 0xFF);
		src[0] = (uint8_t)(ui & 0xFF);
		return src;
	}

	uint8_t* IntToBytes(int ui)
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
	void Float_to_Byte(float f, unsigned char uint8_t[])
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
	void Byte_to_Float(float* f, unsigned char byte[])
	{
		FloatLongType fl;
		fl.ldata = 0;
		fl.ldata = byte[0];
		fl.ldata = (fl.ldata << 8) | byte[1];
		fl.ldata = (fl.ldata << 8) | byte[2];
		fl.ldata = (fl.ldata << 8) | byte[3];
		*f = fl.fdata;
	}

	uint8_t* UInt16ToBytes(uint16 ui)
	{
		uint8_t* src = new uint8_t[2];
		src[1] = (uint8_t)((ui >> 8) & 0xFF);
		src[0] = (uint8_t)(ui & 0xFF);
		return src;
	}

	uint8_t* Int16ToBytes(int16 ui)
	{
		uint8_t* src = new uint8_t[2];
		src[1] = (uint8_t)((ui >> 8) & 0xFF);
		src[0] = (uint8_t)(ui & 0xFF);
		return src;
	}

	RGBA ConversionByteToRGB555A1(uint8_t* b5g5r5a1)
	{
		uint8_t b0 = b5g5r5a1[1];
		uint8_t b1 = b5g5r5a1[0];
		RGBA out;
		out.R = b0 >> 2;
		out.G = (b0 << 3 | b1 >> 5) & 0x1f;
		out.B = b1 & 0x1f;
		out.A = 0;
		return out;
	}

	BYTE_16bit ConversionRGB555A1ToByte(RGBA rgb5)
	{
		BYTE_16bit out;
		out.B1 = (rgb5.G << 5);
		out.B1 = out.B1 & 0xe0;
		out.B1 = out.B1 | rgb5.B;
		//out.B1 = ((rgb5.G << 5) & 0xe0) | rgb5.B;
		out.B2 = ((rgb5.R << 2)) | (rgb5.G >> 3);
		return out;
	}
}