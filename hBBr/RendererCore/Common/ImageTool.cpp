#include "ImageTool.h"
#include <fstream>
#include <istream>
#include <direct.h>
#include "ConsoleDebug.h"
#include "DDSTool.h"
ImageData* ImageTool::ReadTgaImage(const char* filename)
{
	ImageData* out = NULL;
	//open TGA file
	errno_t err_src_name;
	FILE* file = NULL;
	err_src_name = fopen_s(&file, filename, "rb");//��ȡ��������ģʽ

	if (file == NULL || err_src_name != 0)
	{
		ConsoleDebug::print_endl("open TGA image file failed.", "255,255,0");
		return NULL;
	}

	//��ȡ�ļ�ͷ
	ImageHeader data_header;
	{
		//fread(&data_header, sizeof(ImageHeader), 1, file_tga);//ֱ�Ӷ�ȡ���нṹ��Ķ�������
		fread(&data_header.idLength, sizeof(uint8_t), 1, file);
		fread(&data_header.colormapType, sizeof(uint8_t), 1, file);
		fread(&data_header.imageType, sizeof(uint8_t), 1, file);
		fread(&data_header.colormapOrigin, sizeof(uint16_t), 1, file);
		fread(&data_header.colormapLength, sizeof(uint16_t), 1, file);
		fread(&data_header.colormapDepth, sizeof(uint8_t), 1, file);
		fread(&data_header.xOrigin, sizeof(uint16_t), 1, file);
		fread(&data_header.yOrigin, sizeof(uint16_t), 1, file);
		fread(&data_header.width, sizeof(uint16_t), 1, file);
		fread(&data_header.height, sizeof(uint16_t), 1, file);
		fread(&data_header.bitsPerPixel, sizeof(uint8_t), 1, file);
		fread(&data_header.imageDescriptor, sizeof(uint8_t), 1, file);
	}

	out = new ImageData;
	out->data_header = data_header;

	unsigned int pixelSize = data_header.height * data_header.width;
	unsigned int imageSize = pixelSize * (data_header.bitsPerPixel / 8);//���� �� ÿ������ռ���ֽ��� = �ܳ���

	ConsoleDebug::print_endl("Get targa image:" + HString(filename)
		+ "\n\twidth:" + HString::FromInt(data_header.width)
		+ "\n\theight:" + HString::FromInt(data_header.height)
		+ "\n\tdepth:" + HString::FromInt(data_header.bitsPerPixel)
		+ "\n\timageSize:" + HString::FromInt(imageSize)
		, "205,225,255");

	//��ȡ����
	byte* tgaData = new byte[imageSize];
	if (fread(tgaData, 1, imageSize, file) != imageSize)
	{
		ConsoleDebug::print_endl("Read TGA image color error.all pixel size is not equal the image size.", "255,255,0");
		if (out)
			delete out;
		if (tgaData)
			delete[] out;
		return NULL;
	}
	try {
		switch (data_header.bitsPerPixel)
		{
		case 8:
			if (data_header.imageType == 3)//8λ��ͨ���Ҷ�ͼ
			{
				out->texFormat = VkFormat::VK_FORMAT_R8_UNORM;
				out->imageData.resize(imageSize);
				out->imageSize = imageSize;
				if (memcpy_s(out->imageData.data(), imageSize, tgaData, imageSize) != 0)
				{
					if (out)
						delete out;
					if (tgaData)
						delete[] out;
					return NULL;
				}
			}
			break;
		case 16:
		{
			out->imageData.resize(imageSize);
			out->texFormat = VkFormat::VK_FORMAT_A1R5G5B5_UNORM_PACK16;
			out->imageSize = imageSize;
			if (memcpy_s(out->imageData.data(), imageSize, tgaData, imageSize) != 0)
			{
				if (out)
					delete out;
				if (tgaData)
					delete[] out;
				return NULL;
			}
		}
		break;
		case 24:
			//rgb888 to rgb 555
			//DX ��֧�ֵ�����R8 G8 B8 :24bit������Ҫתһ��16bit
			out->imageData.resize(pixelSize * 2);
			out->texFormat = VkFormat::VK_FORMAT_A1R5G5B5_UNORM_PACK16;
			data_header.bitsPerPixel = 16;
			out->data_header.bitsPerPixel = data_header.bitsPerPixel;

			for (unsigned int i = 0; i < pixelSize; i++)
			{
				byte R5, G5, B5;
				float rgb555i = (float)(255.0f / 31.0f);
				B5 = (byte)(tgaData[i * 3] / rgb555i) & 0xff;
				G5 = (byte)(tgaData[i * 3 + 1] / rgb555i) & 0xff;
				R5 = (byte)(tgaData[i * 3 + 2] / rgb555i) & 0xff;
				byte B1, B2;
				B1 = B5 | (G5 << 5);
				B2 = (R5 << 2) | (G5 >> 3);
				out->imageData[i * 2] = B1;
				out->imageData[i * 2 + 1] = B2;
			}
			out->imageSize = pixelSize * 2;
			break;
		case 32://32λBGRA
			out->texFormat = VkFormat::VK_FORMAT_B8G8R8A8_UNORM;
			out->imageData.resize(imageSize);
			out->imageSize = imageSize;
			if (memcpy_s(out->imageData.data(), imageSize, tgaData, imageSize) != 0)
			{
				if (out)
					delete out;
				if (tgaData)
					delete[] out;
				return NULL;
			}
			break;
		default:
			ConsoleDebug::print_endl("Read TGA image file header failed.Image depth is " + HString::FromInt(data_header.bitsPerPixel) + ".that is unuseful depth.", "255,255,0");
			if (out)
				delete out;
			if (tgaData)
				delete[] out;
			return NULL;
		}
	}
	catch (std::exception ex)
	{
		ConsoleDebug::print_endl(HString("Read Tga Image Pixel Failed: ") + ex.what(), "255,255,0");
	}

	ConsoleDebug::print_endl("Get image texture:" + HString(filename)
		+ "\n\twidth:" + HString::FromInt(data_header.width)
		+ "\n\theight:" + HString::FromInt(data_header.height)
		+ "\n\tdepth:" + HString::FromInt(data_header.bitsPerPixel)
		+ "\n\timageSize:" + HString::FromInt(out->imageSize)
		, "205,225,255");

	if (file != NULL)
		fclose(file);

	if (tgaData != NULL)
	{
		delete[] tgaData;
		tgaData = NULL;
	}
	HString cbPath = filename;
	out->filePath = cbPath;
	out->fileName = cbPath.GetBaseName();

	//SaveTgaImage("D:\\123.tga", out);

	return out;
}

ImageData* ImageTool::ReadDDSImage(const char* filename)
{
	ImageData* out = NULL;
	DDSLoader loader(filename);
	out = loader.LoadDDSToImage();
	return out;
}

bool ImageTool::SaveTgaImage(const char* filename, ImageData* tgaData)
{
	try {
		if (tgaData == NULL)
		{
			ConsoleDebug::print_endl("TGA data was null", "255,255,0");
			return false;
		}
		if (tgaData->imageData.size() <= 0)
		{
			ConsoleDebug::print_endl(HString("Save TGA image failed.The TGA data is empty!!") + filename, "255,255,0");
			return false;
		}
		FILE* file = NULL;
		if (fopen_s(&file, filename, "wb") != 0)//������+д
		{
			ConsoleDebug::print_endl(HString("open tga file failed: ") + filename, "255,255,0");
			return false;
		}
		//12 byte Header
		byte TGA_UnCompressed_HeaderRef[12] = { 0,0,2,0,0,0,0,0,0,0,0,0 };
		//6 byte image data
		byte* TGAwidth_data = TypeConversion::UIntToBytes(tgaData->data_header.width);
		byte* TGAheight_data = TypeConversion::UIntToBytes(tgaData->data_header.height);
		byte* TGAdepth_data = TypeConversion::UIntToBytes(tgaData->data_header.bitsPerPixel);
		byte TGA_Image_Data_Ref[6] = { TGAwidth_data[0],TGAwidth_data[1],TGAheight_data[0],TGAheight_data[1],TGAdepth_data[0],TGAdepth_data[1] };
		//color message
		unsigned int pixelSize = tgaData->data_header.height * tgaData->data_header.width;
		const unsigned int imageSize = pixelSize * (tgaData->data_header.bitsPerPixel / 8);
		byte* TGA_Image_Message = new byte[imageSize];
		memcpy_s(TGA_Image_Message, imageSize, tgaData->imageData.data(), imageSize);
		//
		fwrite(TGA_UnCompressed_HeaderRef, 1, 12, file);
		fwrite(TGA_Image_Data_Ref, 1, 6, file);
		fwrite(TGA_Image_Message, 1, imageSize, file);

		fclose(file);

		delete[] TGAwidth_data;
		delete[] TGAheight_data;
		delete[] TGAdepth_data;
		delete[] TGA_Image_Message;

		ConsoleDebug::print_endl(HString("Save Tga Image Successful: ") + filename, "0,255,0");
		return true;
	}
	catch (std::exception ex)
	{
		ConsoleDebug::print_endl(HString("Save Tga Image Failed: ") + ex.what(), "255,255,0");
		return false;
	}
}

bool ImageTool::SaveTgaImage(const char* filename, uint16_t w, uint16_t h, uint16_t d, void* imageData)
{
	try {
		if (imageData == NULL)
		{
			ConsoleDebug::print_endl("TGA data was null", "255,255,0");
			return false;
		}
		FILE* file = NULL;
		if (fopen_s(&file, filename, "wb") != 0)//������+д
		{
			ConsoleDebug::print_endl(HString("open tga file failed: ") + filename, "255,255,0");
			return false;
		}
		//12 byte Header
		byte TGA_UnCompressed_HeaderRef[12] = { 0,0,2,0,0,0,0,0,0,0,0,0 };
		//6 byte image data
		byte* TGAwidth_data = TypeConversion::UIntToBytes(w);
		byte* TGAheight_data = TypeConversion::UIntToBytes(h);
		byte* TGAdepth_data = TypeConversion::UIntToBytes(d);
		byte TGA_Image_Data_Ref[6] = { TGAwidth_data[0],TGAwidth_data[1],TGAheight_data[0],TGAheight_data[1],TGAdepth_data[0],TGAdepth_data[1] };
		//color message
		unsigned int pixelSize = h * w;
		const unsigned int imageSize = pixelSize * (d / 8);
		byte* TGA_Image_Message = new byte[imageSize];
		memcpy_s(TGA_Image_Message, imageSize, imageData, imageSize);
		//
		fwrite(TGA_UnCompressed_HeaderRef, 1, 12, file);
		fwrite(TGA_Image_Data_Ref, 1, 6, file);
		fwrite(TGA_Image_Message, 1, imageSize, file);

		fclose(file);

		delete[] TGAwidth_data;
		delete[] TGAheight_data;
		delete[] TGAdepth_data;
		delete[] TGA_Image_Message;

		ConsoleDebug::print_endl(HString("Save Tga Image Successful: ") + filename, "0,255,0");
		return true;
	}
	catch (std::exception ex)
	{
		ConsoleDebug::print_endl(HString("Save Tga Image Failed: ") + ex.what(), "255,255,0");
		return false;
	}
}

bool ImageTool::SavePngImage(const char* filename, uint16_t w, uint16_t h ,void* imageData)
{
	//std::vector<unsigned char>buffer;
	lodepng::encode(filename, (unsigned char*)imageData, w, h);
	return true;
}

ImageData* ImageTool::ReadPngImage(const char* filename)
{
	HString FileName = filename;
	bool ignore_checksums = false;
	std::vector<unsigned char>buffer;
	std::vector<unsigned char>imageData;
	unsigned int w = 0;
	unsigned int h = 0;
	unsigned error = 0;
	error = lodepng::load_file(buffer, filename);
	if (error) {
		std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
		HString errorStr = HString("Load png failed : ") + lodepng_error_text(error);
		printf((errorStr + "\n").c_str());
		ConsoleDebug::print_endl(HString("Load png failed.") + lodepng_error_text(error), "255,55,0");
		return NULL;
	}
	lodepng::State state;
	if (ignore_checksums) {
		state.decoder.ignore_crc = 1;
		state.decoder.zlibsettings.ignore_adler32 = 1;
	}

	error = lodepng::decode(imageData, w, h, state, buffer);
	if (error) {
		std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
		HString errorStr = HString("Load png failed : ") + lodepng_error_text(error);
		printf((errorStr+"\n").c_str());
		ConsoleDebug::print_endl(HString("Load png failed.") + lodepng_error_text(error), "255,55,0");
		return NULL;
	}

	const LodePNGColorMode& color = state.info_png.color;

	ImageData* out = new ImageData;
	out->data_header.width = w;
	out->data_header.height = h;
	out->imageData = std::move(imageData);
	out->data_header.bitsPerPixel = color.bitdepth;
	out->fileName = FileName.GetBaseName();
	out->filePath = FileName;

	int ClannelNum = 1;
	switch (color.colortype)
	{
	case LCT_RGBA:
		out->texFormat = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
		ClannelNum = 4;
		break;
	case LCT_RGB:
		out->texFormat = VkFormat::VK_FORMAT_R8G8B8_UNORM;
		ClannelNum = 3;
		break;
	case LCT_GREY:
		out->texFormat = VkFormat::VK_FORMAT_R8_UNORM;
		ClannelNum = 1;
		break;
	default:
		delete out;
		ConsoleDebug::print_endl("It is not support this png format.", "255,55,0");
		return NULL;
		break;
	}

	out->imageSize = w * h * (out->data_header.bitsPerPixel/8 * ClannelNum);
	out->data_header.bitsPerPixel *= ClannelNum;

	return out;
}

void GetHeaderLine(const char*& BufferPos, char Line[256])
{
	char* LinePtr = Line;
	uint32_t i;
	for (i = 0; i < 255; ++i)
	{
		if (*BufferPos == 0 || *BufferPos == 10 || *BufferPos == 13)
		{
			++BufferPos;
			break;
		}
		*LinePtr++ = *BufferPos++;
	}
	Line[i] = 0;
}

ImageData* ImageTool::ReadHDRImage(const char* filename)
{
	HString fileName = filename;
	if (!fileName.GetSuffix().IsSame("hdr", false))
	{
		ConsoleDebug::print_endl("open hdr image file failed.It is not a hdr file.", "255,255,0");
		return NULL;
	}
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open())
	{
		//throw std::runtime_error((HString("failed to open file : ") + filePath).c_str());
		ConsoleDebug::print_endl((HString("failed to open file : ") + filename).c_str(), "255,20,0");
	}
	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	//

	if (buffer.size() < 11)
	{
		ConsoleDebug::print_endl("open hdr image file failed.It is not a hdr file or file corruption.", "255,255,0");
		return NULL;
	}

	const char* FileDataPtr = buffer.data();
	char Line[256];
	GetHeaderLine(FileDataPtr, Line);

	if (std::strcmp(Line, "#?RADIANCE") != 0 )
	{
		ConsoleDebug::print_endl("open hdr image file failed.It is not a hdr file or file corruption.", "255,255,0");
		return NULL;
	}

	ImageData* out = new ImageData;
	const char* RGBDataStart;
	for (;;)
	{
		GetHeaderLine(FileDataPtr, Line);

		char* HeightStr = std::strstr(Line, "-Y ");
		char* WidthStr = std::strstr(Line, "+X ");

		if (HeightStr != NULL && WidthStr != NULL)
		{
			// insert a /0 after the height value
			*(WidthStr - 1) = 0;

			out->data_header.height = std::atoi(HeightStr + 3);
			out->data_header.width = std::atoi(WidthStr + 3);

			RGBDataStart = FileDataPtr;
		}
	}




	return out;
}

void ImageTool::ImageFlipY(uint32_t w, uint32_t h, uint32_t d,void *data)
{
	if (data)
	{
		uint32_t width = w;
		uint32_t height = h;
		uint32_t depth = d;
		uint32_t lineSize = width * depth;
		char* top;
		char* bottom;
		char* temp = (char*)malloc(sizeof(char) * lineSize);//һ�е�����;
		for (auto i = 0; i < height / 2 ; i++)
		{
			top = (char*)data + i * lineSize;
			bottom = (char*)data + (height - i - 1) * lineSize;
			memcpy(temp , bottom, lineSize);
			memcpy(bottom, top, lineSize);
			memcpy(top, temp, lineSize);
		}
		free(temp);
	}
}