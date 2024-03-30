#include "DDSTool.h"
#include "ImageTool.h"
#include "Texture2D.h"
#include "ConsoleDebug.h"
#define ISBITMASK(r, g, b, a) (ddpf.dwRBitMask == r && ddpf.dwGBitMask == g && ddpf.dwBBitMask == b && ddpf.dwABitMask == a)

static DXGI_FORMAT GetDXGIFormat(const FDDSPixelFormatHeader& ddpf)
{
	if (ddpf.dwFlags & DDS_RGB)
	{
		// Note that sRGB formats are written using the "DX10" extended header.

		switch (ddpf.dwRGBBitCount)
		{
		case 32:
			if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
			{
				return DXGI_FORMAT_R8G8B8A8_UNORM;
			}

			if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
			{
				return DXGI_FORMAT_B8G8R8A8_UNORM;
			}

			if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
			{
				return DXGI_FORMAT_B8G8R8X8_UNORM;
			}

			// No DXGI format maps to ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000) aka D3DFMT_X8B8G8R8

			// Note that many common DDS reader/writers (including D3DX) swap the
			// the RED/BLUE masks for 10:10:10:2 formats. We assumme
			// below that the 'backwards' header mask is being used since it is most
			// likely written by D3DX. The more robust solution is to use the 'DX10'
			// header extension and specify the DXGI_FORMAT_R10G10B10A2_UNORM format directly

			// For 'correct' writers, this should be 0x000003ff, 0x000ffc00, 0x3ff00000 for RGB data
			if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
			{
				return DXGI_FORMAT_R10G10B10A2_UNORM;
			}

			// No DXGI format maps to ISBITMASK(0x000003ff, 0x000ffc00, 0x3ff00000, 0xc0000000) aka D3DFMT_A2R10G10B10

			if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
			{
				return DXGI_FORMAT_R16G16_UNORM;
			}

			if (ISBITMASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000))
			{
				// Only 32-bit color channel format in D3D9 was R32F.
				return DXGI_FORMAT_R32_FLOAT; // D3DX writes this out as a FourCC of 114.
			}
			break;

		case 24:
			// No 24bpp DXGI formats aka D3DFMT_R8G8B8
			break;

		case 16:
			if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000))
			{
				return DXGI_FORMAT_B5G5R5A1_UNORM;
			}
			if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0x0000))
			{
				return DXGI_FORMAT_B5G6R5_UNORM;
			}

			// No DXGI format maps to ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x0000) aka D3DFMT_X1R5G5B5.
			if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0xf000))
			{
				return DXGI_FORMAT_B4G4R4A4_UNORM;
			}

			// No DXGI format maps to ISBITMASK(0x0f00, 0x00f0, 0x000f, 0x0000) aka D3DFMT_X4R4G4B4.

			// No 3:3:2, 3:3:2:8, or paletted DXGI formats aka D3DFMT_A8R3G3B2, D3DFMT_R3G3B2, D3DFMT_P8, D3DFMT_A8P8, etc.
			break;
		}
	}
	else if (ddpf.dwFlags & DDS_LUMINANCE)
	{
		if (8 == ddpf.dwRGBBitCount)
		{
			if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x00000000))
			{
				return DXGI_FORMAT_R8_UNORM; // D3DX10/11 writes this out as DX10 extension
			}

			// No DXGI format maps to ISBITMASK(0x0f, 0x00, 0x00, 0xf0) aka D3DFMT_A4L4.
		}

		if (16 == ddpf.dwRGBBitCount)
		{
			if (ISBITMASK(0x0000ffff, 0x00000000, 0x00000000, 0x00000000))
			{
				return DXGI_FORMAT_R16_UNORM; // D3DX10/11 writes this out as DX10 extension.
			}
			if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
			{
				return DXGI_FORMAT_R8G8_UNORM; // D3DX10/11 writes this out as DX10 extension.
			}
		}
	}
	else if (ddpf.dwFlags & DDS_ALPHA)
	{
		if (8 == ddpf.dwRGBBitCount)
		{
			return DXGI_FORMAT_A8_UNORM;
		}
	}
	else if (ddpf.dwFlags & DDS_FOURCC)
	{
		if (MAKEFOURCC('D', 'X', 'T', '1') == ddpf.dwFourCC)
		{
			return DXGI_FORMAT_BC1_UNORM;
		}
		if (MAKEFOURCC('D', 'X', 'T', '3') == ddpf.dwFourCC)
		{
			return DXGI_FORMAT_BC2_UNORM;
		}
		if (MAKEFOURCC('D', 'X', 'T', '5') == ddpf.dwFourCC)
		{
			return DXGI_FORMAT_BC3_UNORM;
		}

		// While pre-mulitplied alpha isn't directly supported by the DXGI formats,
		// they are basically the same as these BC formats so they can be mapped
		if (MAKEFOURCC('D', 'X', 'T', '2') == ddpf.dwFourCC)
		{
			return DXGI_FORMAT_BC2_UNORM;
		}
		if (MAKEFOURCC('D', 'X', 'T', '4') == ddpf.dwFourCC)
		{
			return DXGI_FORMAT_BC3_UNORM;
		}

		if (MAKEFOURCC('A', 'T', 'I', '1') == ddpf.dwFourCC)
		{
			return DXGI_FORMAT_BC4_UNORM;
		}
		if (MAKEFOURCC('B', 'C', '4', 'U') == ddpf.dwFourCC)
		{
			return DXGI_FORMAT_BC4_UNORM;
		}
		if (MAKEFOURCC('B', 'C', '4', 'S') == ddpf.dwFourCC)
		{
			return DXGI_FORMAT_BC4_SNORM;
		}

		if (MAKEFOURCC('A', 'T', 'I', '2') == ddpf.dwFourCC)
		{
			return DXGI_FORMAT_BC5_UNORM;
		}
		if (MAKEFOURCC('B', 'C', '5', 'U') == ddpf.dwFourCC)
		{
			return DXGI_FORMAT_BC5_UNORM;
		}
		if (MAKEFOURCC('B', 'C', '5', 'S') == ddpf.dwFourCC)
		{
			return DXGI_FORMAT_BC5_SNORM;
		}

		// BC6H and BC7 are written using the "DX10" extended header

		if (MAKEFOURCC('R', 'G', 'B', 'G') == ddpf.dwFourCC)
		{
			return DXGI_FORMAT_R8G8_B8G8_UNORM;
		}
		if (MAKEFOURCC('G', 'R', 'G', 'B') == ddpf.dwFourCC)
		{
			return DXGI_FORMAT_G8R8_G8B8_UNORM;
		}

		// Check for D3DFORMAT enums being set here.
		switch (ddpf.dwFourCC)
		{
		case 36: // D3DFMT_A16B16G16R16
			return DXGI_FORMAT_R16G16B16A16_UNORM;

		case 110: // D3DFMT_Q16W16V16U16
			return DXGI_FORMAT_R16G16B16A16_SNORM;

		case 111: // D3DFMT_R16F
			return DXGI_FORMAT_R16_FLOAT;

		case 112: // D3DFMT_G16R16F
			return DXGI_FORMAT_R16G16_FLOAT;

		case 113: // D3DFMT_A16B16G16R16F
			return DXGI_FORMAT_R16G16B16A16_FLOAT;

		case 114: // D3DFMT_R32F
			return DXGI_FORMAT_R32_FLOAT;

		case 115: // D3DFMT_G32R32F
			return DXGI_FORMAT_R32G32_FLOAT;

		case 116: // D3DFMT_A32B32G32R32F
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		}
	}

	return DXGI_FORMAT_UNKNOWN;
}

//--------------------------------------------------------------------------------------
// Return the BPP for a particular format.
//--------------------------------------------------------------------------------------
static uint8_t BitsPerPixel(EDXGI_FORMAT fmt)
{
	switch (fmt)
	{
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
		return 128;

	case DXGI_FORMAT_R32G32B32_TYPELESS:
	case DXGI_FORMAT_R32G32B32_FLOAT:
	case DXGI_FORMAT_R32G32B32_UINT:
	case DXGI_FORMAT_R32G32B32_SINT:
		return 96;

	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SNORM:
	case DXGI_FORMAT_R16G16B16A16_SINT:
	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R32G32_UINT:
	case DXGI_FORMAT_R32G32_SINT:
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		return 64;

	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
	case DXGI_FORMAT_R10G10B10A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UINT:
	case DXGI_FORMAT_R11G11B10_FLOAT:
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
	case DXGI_FORMAT_R16G16_TYPELESS:
	case DXGI_FORMAT_R16G16_FLOAT:
	case DXGI_FORMAT_R16G16_UNORM:
	case DXGI_FORMAT_R16G16_UINT:
	case DXGI_FORMAT_R16G16_SNORM:
	case DXGI_FORMAT_R16G16_SINT:
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT:
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
	case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
	case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		return 32;

	case DXGI_FORMAT_R8G8_TYPELESS:
	case DXGI_FORMAT_R8G8_UNORM:
	case DXGI_FORMAT_R8G8_UINT:
	case DXGI_FORMAT_R8G8_SNORM:
	case DXGI_FORMAT_R8G8_SINT:
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
	case DXGI_FORMAT_R16_UINT:
	case DXGI_FORMAT_R16_SNORM:
	case DXGI_FORMAT_R16_SINT:
	case DXGI_FORMAT_B5G6R5_UNORM:
	case DXGI_FORMAT_B5G5R5A1_UNORM:
	case DXGI_FORMAT_B4G4R4A4_UNORM:
		return 16;

	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8_UINT:
	case DXGI_FORMAT_R8_SNORM:
	case DXGI_FORMAT_R8_SINT:
	case DXGI_FORMAT_A8_UNORM:
		return 8;

	case DXGI_FORMAT_R1_UNORM:
		return 1;

	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		return 4;

	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		return 8;

	default:
		return 0;
	}
}

//--------------------------------------------------------------------------------------
// Get surface information for a particular format.
//--------------------------------------------------------------------------------------
static int GetCnumBytesPerBlock(
	EDXGI_FORMAT fmt
)
{
	int bcnumBytesPerBlock = 0;
	switch (fmt)
	{
	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		bcnumBytesPerBlock = 8;
		break;

	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		bcnumBytesPerBlock = 16;
		break;

	case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
		break;
	default:
		break;
	}
	return bcnumBytesPerBlock;
}


//--------------------------------------------------------------------------------------
DDSLoader::DDSLoader(const char* filePath) : DDSHeader(0), DDS10Header(0)
{
	isCubeMap = false;
	path = filePath;
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);
	if (!file.is_open())
	{
		//throw std::runtime_error((HString("failed to open file : ") + filePath).c_str());
		ConsoleDebug::print_endl((HString("failed to open dds file : ") + filePath).c_str(), "255,20,0");
		return;
	}
	size_t fileSize = static_cast<size_t>(file.tellg());
	if (fileSize <= sizeof(FDDSFileHeader))
	{
		return;
	}
	DDSBuffer.resize(fileSize);
	file.seekg(0);
	file.read(DDSBuffer.data(), fileSize);
	file.close();
}

DDSLoader::DDSLoader(std::vector<char> buffer) : DDSHeader(0), DDS10Header(0)
{
	isCubeMap = false;
	DDSBuffer = buffer;
}

ImageData* DDSLoader::LoadDDSToImage()
{
	arraySize = 1;
	if (DDSBuffer.size() <= sizeof(FDDSFileHeader))
	{
		return nullptr;
	}

	//获取文件头
	const FDDSFileHeader* DDS = (FDDSFileHeader*)(DDSBuffer.data() + 4);
	uint32_t AlwaysRequiredFlags = DDSF_Caps | DDSF_Height | DDSF_Width | DDSF_PixelFormat;
	if (DDSBuffer.size() >= sizeof(FDDSFileHeader) + 4 &&
		DDSBuffer[0] == 'D' && DDSBuffer[1] == 'D' && DDSBuffer[2] == 'S' && DDSBuffer[3] == ' ' &&
		DDS->dwSize == sizeof(FDDSFileHeader) &&
		DDS->ddpf.dwSize == sizeof(FDDSPixelFormatHeader) &&
		(DDS->dwFlags & AlwaysRequiredFlags) == AlwaysRequiredFlags)
	{
		DDSHeader = DDS;
	}
	// Check for dx10 dds format, 并不是特指DX10,而是只支持DX10以上版本
	// 比如最新的BC6和BC7格式
	if (DDS->ddpf.dwFourCC == DDSPF_DX10)
	{
		DDS10Header = (FDDS10FileHeader*)(DDSBuffer.data() + 4 + sizeof(FDDSFileHeader));
	}
	//是否导入失败
	if (!IsValid())
	{
		ConsoleDebug::print_endl((HString("load dds file failed.")).c_str(), "255,20,0");
		return nullptr;
	}

	//是不是CubeMap
	if (DDSHeader->dwCaps2 & DDSC_CubeMap)
	{
		isCubeMap = true;
		arraySize = 6;
	}

	unsigned int  blockSize = 16;
	bool bcFormat = false;
	uint8_t bitsPerPixel = 8;

	//-----Get format 获取纹理格式
	DXGI_FORMAT dxgiTextureFormat = DXGI_FORMAT_UNKNOWN;
	if(DDS10Header)
	{
		dxgiTextureFormat = (DXGI_FORMAT)DDS10Header->format;
	}
	else
	{
		dxgiTextureFormat = GetDXGIFormat(DDSHeader->ddpf);
	}

	//如果没有获取到格式,就相当于失败了。
	if(dxgiTextureFormat == DXGI_FORMAT_UNKNOWN)
	{
		ConsoleDebug::print_endl((HString("load dds file failed.Unknow texture format.")).c_str(), "255,20,0");
		return nullptr;
	}

	ImageData* out = new ImageData;

	//获取BC block 大小,只有压缩纹理才会有
	blockSize = GetCnumBytesPerBlock(dxgiTextureFormat);
	//获取像素位数
	bitsPerPixel = BitsPerPixel(dxgiTextureFormat);
	bcFormat = blockSize != 0;

	switch(dxgiTextureFormat)
	{
		//RGBA8
		case	DXGI_FORMAT_R8G8B8A8_UNORM:
			out->texFormat = VkFormat::VK_FORMAT_R8G8B8A8_UNORM; break;
		case	DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			out->texFormat = VkFormat::VK_FORMAT_R8G8B8A8_SRGB; break;
		case	DXGI_FORMAT_R8G8B8A8_UINT :
			out->texFormat = VkFormat::VK_FORMAT_R8G8B8A8_UINT; break;
		case	DXGI_FORMAT_R8G8B8A8_SNORM :
			out->texFormat = VkFormat::VK_FORMAT_R8G8B8A8_SNORM; break;
		case	DXGI_FORMAT_R8G8B8A8_SINT :
			out->texFormat = VkFormat::VK_FORMAT_R8G8B8A8_SINT; break;
		//RGBA16
		case	DXGI_FORMAT_R16G16B16A16_FLOAT:
			out->texFormat = VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT; break;
		case	DXGI_FORMAT_R16G16B16A16_UNORM:
			out->texFormat = VkFormat::VK_FORMAT_R16G16B16A16_UNORM; break;
		case	DXGI_FORMAT_R16G16B16A16_UINT:
			out->texFormat = VkFormat::VK_FORMAT_R16G16B16A16_UINT; break;
		case	DXGI_FORMAT_R16G16B16A16_SNORM:
			out->texFormat = VkFormat::VK_FORMAT_R16G16B16A16_SNORM; break;
		case	DXGI_FORMAT_R16G16B16A16_SINT:
			out->texFormat = VkFormat::VK_FORMAT_R16G16B16A16_SINT; break;
		//RGBA32
		case	DXGI_FORMAT_R32G32B32A32_FLOAT:
			out->texFormat = VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT; break;
		case	DXGI_FORMAT_R32G32B32A32_UINT:
			out->texFormat = VkFormat::VK_FORMAT_R32G32B32A32_UINT; break;
		case	DXGI_FORMAT_R32G32B32A32_SINT:
			out->texFormat = VkFormat::VK_FORMAT_R32G32B32A32_SINT; break;
		//R
		case	DXGI_FORMAT_R8_UNORM:
			out->texFormat = VkFormat::VK_FORMAT_R8_UNORM; break;
		case	DXGI_FORMAT_R8_UINT:
			out->texFormat = VkFormat::VK_FORMAT_R8_UINT; break;
		case	DXGI_FORMAT_R8_SNORM:
			out->texFormat = VkFormat::VK_FORMAT_R8_SNORM; break;
		case	DXGI_FORMAT_R8_SINT:
			out->texFormat = VkFormat::VK_FORMAT_R8_SINT; break;
		//BC1
		case	DXGI_FORMAT_BC1_UNORM:
			out->texFormat = VkFormat::VK_FORMAT_BC1_RGB_UNORM_BLOCK; break;
		case	DXGI_FORMAT_BC1_UNORM_SRGB:
			out->texFormat = VkFormat::VK_FORMAT_BC1_RGB_SRGB_BLOCK; break;
		//BC2
		case	DXGI_FORMAT_BC2_UNORM:
			out->texFormat = VkFormat::VK_FORMAT_BC2_UNORM_BLOCK; break;
		case	DXGI_FORMAT_BC2_UNORM_SRGB:
			out->texFormat = VkFormat::VK_FORMAT_BC2_SRGB_BLOCK; break;
		//BC3
		case	DXGI_FORMAT_BC3_UNORM:
			out->texFormat = VkFormat::VK_FORMAT_BC3_UNORM_BLOCK; break;
		case	DXGI_FORMAT_BC3_UNORM_SRGB:
			out->texFormat = VkFormat::VK_FORMAT_BC3_SRGB_BLOCK; break;
		//BC4
		case	DXGI_FORMAT_BC4_UNORM:
			out->texFormat = VkFormat::VK_FORMAT_BC4_UNORM_BLOCK; break;
		case	DXGI_FORMAT_BC4_SNORM:
			out->texFormat = VkFormat::VK_FORMAT_BC4_SNORM_BLOCK; break;
		//BC5
		case	DXGI_FORMAT_BC5_UNORM:
			out->texFormat = VkFormat::VK_FORMAT_BC5_UNORM_BLOCK; break;
		case	DXGI_FORMAT_BC5_SNORM:
			out->texFormat = VkFormat::VK_FORMAT_BC5_SNORM_BLOCK; break;
		//BC6
		case	DXGI_FORMAT_BC6H_UF16:
			out->texFormat = VkFormat::VK_FORMAT_BC6H_UFLOAT_BLOCK; break;
		case	DXGI_FORMAT_BC6H_SF16:
			out->texFormat = VkFormat::VK_FORMAT_BC6H_SFLOAT_BLOCK; break;
		//BC6
		case	DXGI_FORMAT_BC7_UNORM:
			out->texFormat = VkFormat::VK_FORMAT_BC7_UNORM_BLOCK; break;
		case	DXGI_FORMAT_BC7_UNORM_SRGB:
			out->texFormat = VkFormat::VK_FORMAT_BC7_SRGB_BLOCK; break;
		default:
			break;
	}

	//-----Get data
	out->isCubeMap = isCubeMap;
	out->blockSize = blockSize;
	out->data_header.width = DDSHeader->dwWidth;
	out->data_header.height = DDSHeader->dwHeight;
	out->data_header.bitsPerPixel = bitsPerPixel;
	out->mipLevel = DDSHeader->dwMipMapCount;

	//获取图像总大小
	out->imageSize = 0;
	int32_t mipWidth = DDSHeader->dwWidth;
	int32_t mipHeight = DDSHeader->dwHeight;
	for (unsigned int i = 0; i < out->mipLevel; ++i)
	{
		if (bcFormat)
		{
			uint32_t imageSize = SIZE_OF_BC(mipWidth, mipHeight, blockSize);//BC压缩格式的大小计算公式
			out->imageSize += imageSize;
		}
		else
			out->imageSize += mipWidth * mipHeight * (bitsPerPixel /8);
		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	//-----Get image data pointer 获取图像数据
	const char* Ptr = (const char*)DDSHeader + sizeof(FDDSFileHeader);
	// jump over dx10 header if available
	if (DDS10Header)
	{
		Ptr += sizeof(FDDS10FileHeader);
	}
	if(isCubeMap)
	{
		out->imageData.resize(out->imageSize * 6);
		memcpy(out->imageData.data(), Ptr, out->imageSize * 6);
	}
	else
	{
		out->imageData.resize(out->imageSize);
		memcpy(out->imageData.data(), Ptr, out->imageSize);
	}
	return out;
}

bool DDSLoader::IsValid2DTexture() const
{
	if (IsValid() && (DDSHeader->dwCaps2 & DDSC_CubeMap) == 0 && (DDSHeader->dwCaps2 & DDSC_Volume) == 0 && (DDS10Header == nullptr || (DDS10Header->resourceType == 3 && DDS10Header->arraySize == 1)))
	{
		return true;
	}
	return false;
}