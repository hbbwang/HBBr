#include <ostream>
#include "Texture2D.h"
#include "VulkanRenderer.h"
#include "VulkanManager.h"
#include "ContentManager.h"
#include "FileSystem.h"
#include "ConsoleDebug.h"
#include "DDSTool.h"
#include "ContentManager.h"
#include "Serializable.h"
#include "RendererConfig.h"
#include "TextureCube.h"
#include "VulkanObjectManager.h"
#include  "VMABuffer.h"
std::vector<Texture2D*> Texture2D::_upload_textures;
std::unordered_map<HString, std::weak_ptr<Texture2D>> Texture2D::_system_textures;
std::vector<VkSampler>Texture2D::_samplers;
uint64_t Texture2D::_textureStreamingSize = 0;
uint64_t Texture2D::_maxTextureStreamingSize = (uint64_t)4 * (uint64_t)1024 * (uint64_t)1024 * (uint64_t)1024; //4 GB

Texture2D::~Texture2D()
{
	auto manager = VulkanManager::GetManager();
	if (_imageViewMemory != VK_NULL_HANDLE)
	{
		manager->FreeBufferMemory(_imageViewMemory);
		_textureStreamingSize = std::max((uint64_t)0, _textureStreamingSize - _textureMemorySize);
	}
	manager->DestroyImageView(_imageView);
	manager->DestroyImage(_image);
}

void Texture2D::Transition(VkCommandBuffer cmdBuffer, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevelBegin, uint32_t mipLevelCount, uint32_t baseArrayLayer, uint32_t layerCount)
{
	VulkanManager::GetManager()->Transition(cmdBuffer, _image, _imageAspectFlags, oldLayout, newLayout, mipLevelBegin, mipLevelCount, baseArrayLayer, layerCount);
	_imageLayout = newLayout;
}

void Texture2D::TransitionImmediate(VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevelBegin, uint32_t mipLevelCount, uint32_t baseArrayLayer, uint32_t layerCount)
{
	auto manager = VulkanManager::GetManager();
	VkCommandBuffer cmdbuf;
	manager->AllocateCommandBuffer(manager->GetCommandPool(), cmdbuf);
	manager->BeginCommandBuffer(cmdbuf);
	{
		Transition(cmdbuf, oldLayout, newLayout, mipLevelBegin, mipLevelCount, baseArrayLayer, layerCount);
	}
	manager->EndCommandBuffer(cmdbuf);
	manager->SubmitQueueImmediate({ cmdbuf });
	vkQueueWaitIdle(manager->GetGraphicsQueue());
	manager->FreeCommandBuffer(manager->GetCommandPool(), cmdbuf);
}

void Texture2D::Resize(uint32_t width, uint32_t height)
{
	auto manager = VulkanManager::GetManager();
	if (_image == VK_NULL_HANDLE)
		return;

	if (_imageViewMemory != VK_NULL_HANDLE)
	{
		manager->FreeBufferMemory(_imageViewMemory);
		_textureStreamingSize = std::max((uint64_t)0, _textureStreamingSize - _textureMemorySize);
	}
	manager->DestroyImageView(_imageView);
	manager->DestroyImage(_image);
	//
	manager->CreateImage(width, height, _format, _usageFlags, _image);

	_textureMemorySize = manager->CreateImageMemory(_image, _imageViewMemory);
	_textureStreamingSize += _textureMemorySize;

	if (_textureStreamingSize > _maxTextureStreamingSize)
	{
		MessageOut((HString("Max texture streaming size is ") + HString::FromSize_t(_maxTextureStreamingSize) + ", but current texture streaming size is  " + HString::FromSize_t(_textureStreamingSize)), false, false, "255,255,0");
	}

	manager->CreateImageView(_image, _format, _imageAspectFlags, _imageView);
	//
	VkCommandBuffer cmdbuf;
	manager ->AllocateCommandBuffer(manager ->GetCommandPool(), cmdbuf);
	manager->BeginCommandBuffer(cmdbuf);
	{
		Transition(cmdbuf, VK_IMAGE_LAYOUT_UNDEFINED, _imageLayout);
	}
	manager->EndCommandBuffer(cmdbuf);
	manager->SubmitQueueImmediate({ cmdbuf });
	std::vector<VkCommandBuffer> bufs = { cmdbuf };
	manager->FreeCommandBuffers(manager->GetCommandPool(), bufs);

	_imageSize = { width,height };
}

std::shared_ptr<Texture2D> Texture2D::CreateTexture2D(
	uint32_t width, uint32_t height, VkFormat format, 
	VkImageUsageFlags usageFlags, HString textureName,
	uint32_t miplevel, uint32_t layerCount, VkMemoryPropertyFlags memoryPropertyFlag)
{
	std::shared_ptr<Texture2D> newTexture = std::make_shared<Texture2D>();
	newTexture->_textureName = textureName;
	newTexture->_imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	newTexture->_imageSize = {width,height};
	VulkanManager::GetManager()->CreateImage(width, height, format, usageFlags, newTexture->_image, miplevel, layerCount);
	if (format == VK_FORMAT_R32_SFLOAT || format == VK_FORMAT_D32_SFLOAT || format == VK_FORMAT_D24_UNORM_S8_UINT)
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
	else if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D16_UNORM_S8_UINT)
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	else
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	newTexture->_textureMemorySize = VulkanManager::GetManager()->CreateImageMemory(newTexture->_image, newTexture->_imageViewMemory, memoryPropertyFlag);
	_textureStreamingSize += newTexture->_textureMemorySize;
	VulkanManager::GetManager()->CreateImageView(newTexture->_image, format, newTexture->_imageAspectFlags, newTexture->_imageView, miplevel, layerCount);
	newTexture->_format = format;
	newTexture->_usageFlags = usageFlags;
	return newTexture;
}

std::shared_ptr<Texture2D> Texture2D::LoadAsset(HGUID guid, VkImageUsageFlags usageFlags)
{
	const auto texAssets = ContentManager::Get()->GetAssets(AssetType::Texture2D);
	HString guidStr = GUIDToString(guid);
	//从内容管理器查找资产
	auto it = texAssets.find(guid);
	{
		if (it == texAssets.end())
		{
			MessageOut(HString("Can not find [" + guidStr + "] texture in content manager."), false, false, "255,255,0");
			return nullptr;
		}
	}
	auto dataPtr = std::static_pointer_cast<AssetInfo<Texture2D>>(it->second);

	//是否需要重新加载
	bool bReload = false;
	if (dataPtr->IsAssetLoad())
	{
		return dataPtr->GetData();
	}
	else if (!dataPtr->IsAssetLoad() && dataPtr->GetSharedPtr())
	{
		bReload = true;
	}

	//获取实际路径
	HString filePath = it->second->absFilePath;
	if (!FileSystem::FileExist(filePath.c_str()))
	{
		return nullptr;
	}
#if _DEBUG
	ConsoleDebug::print_endl("Import 2D(dds) texture :" + filePath, "255,255,255");
#endif
	//Load dds
	DDSLoader loader(filePath.c_str());
	auto out = loader.LoadDDSToImage();
	if (out == nullptr)
	{
		return nullptr;
	}

	if (out->isCubeMap)
	{
		ConsoleDebug::printf_endl_warning("The texture asset is a cube map.");
		return nullptr;
	}

	std::shared_ptr<Texture2D> newTexture;
	if (!bReload)
	{
		newTexture.reset(new Texture2D);
	}
	else
	{
		//重新刷新asset
		newTexture = dataPtr->GetSharedPtr();
	}

	//Create Texture2D Object.
	uint32_t w = out->data_header.width;
	uint32_t h = out->data_header.height;
	VkFormat format = out->texFormat;

	newTexture->_assetInfo = dataPtr;
	newTexture->_textureName = dataPtr->displayName;
	newTexture->_imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	newTexture->_imageSize = { w, h };
	newTexture->_imageData = *out;
	uint32_t arrayLevel = (newTexture->_imageData.isCubeMap) ? 6 : 1;
	VulkanManager::GetManager()->CreateImage(w, h, format, usageFlags, newTexture->_image, newTexture->_imageData.mipLevel, arrayLevel);
	if (format == VK_FORMAT_R32_SFLOAT || format == VK_FORMAT_D32_SFLOAT)
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
	else if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D16_UNORM_S8_UINT)
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	else
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;

	newTexture->_textureMemorySize = VulkanManager::GetManager()->CreateImageMemory(newTexture->_image, newTexture->_imageViewMemory);
	_textureStreamingSize += newTexture->_textureMemorySize;

	VulkanManager::GetManager()->CreateImageView(newTexture->_image, format, newTexture->_imageAspectFlags, newTexture->_imageView, newTexture->_imageData.mipLevel, arrayLevel);
	newTexture->_format = format;
	newTexture->_usageFlags = usageFlags;

	//标记为需要CopyBufferToImage
	newTexture->UploadToGPU();

	dataPtr->SetData(newTexture);

	return dataPtr->GetData();
}

#if IS_EDITOR
void Texture2D::SaveAsset(HString path)
{
}
#endif

void Texture2D::UploadToGPU()
{
	if (_upload_textures.capacity() <= _upload_textures.size())
	{
		if (_upload_textures.capacity() < 200)
			_upload_textures.reserve(_upload_textures.capacity() + 20);
		else
			_upload_textures.reserve(_upload_textures.capacity() * 2);
	}
	_upload_textures.push_back(this);
}

void Texture2D::GlobalInitialize()
{
	const auto& manager = VulkanManager::GetManager();

	//Create Sampler
	VkSampler sampler = VK_NULL_HANDLE;
	VkSamplerCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	info.magFilter = VK_FILTER_LINEAR;
	info.minFilter = VK_FILTER_LINEAR;
	info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	info.minLod = -100; 
	info.maxLod = VK_LOD_CLAMP_NONE;
	info.compareEnable = false;
	//info.anisotropyEnable = VK_FALSE;
	//info.maxAnisotropy = 1.0f;
	//各项异性采样
	if (manager->GetPhysicalDeviceFeatures().samplerAnisotropy == VK_TRUE)
	{
		info.anisotropyEnable = VK_TRUE;
		info.maxAnisotropy = 16.0f;
	}
	else
	{
		info.anisotropyEnable = VK_FALSE;
		info.maxAnisotropy = 1.0f;
	}

	_samplers.reserve((uint32_t)TextureSampler_Max);
	{
		manager->CreateSampler(sampler, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, 0, 16);
		_samplers.push_back(std::move(sampler));
	}
	{
		manager->CreateSampler(sampler, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, 0, 16);
		_samplers.push_back(std::move(sampler));
	}
	{
		manager->CreateSampler(sampler, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 0, 16);
		_samplers.push_back(std::move(sampler));
	}
	{
		manager->CreateSampler(sampler, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, 0, 16);
		_samplers.push_back(std::move(sampler));
	}
	{
		manager->CreateSampler(sampler, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, 0, 16);
		_samplers.push_back(std::move(sampler));
	}
	{
		manager->CreateSampler(sampler, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, 0, 16);
		_samplers.push_back(std::move(sampler));
	}
	{
		manager->CreateSampler(sampler, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 0, 16);
		_samplers.push_back(std::move(sampler));
	}
	{
		manager->CreateSampler(sampler, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, 0, 16);
		_samplers.push_back(std::move(sampler));
	}

	//Create BaseTexture
	auto uvGridTex = ContentManager::Get()->LoadAsset<Texture2D>(HGUID("69056105-8b40-a3b2-65d4-95ebf2fe28fb"));
	auto blackTex = ContentManager::Get()->LoadAsset<Texture2D>(HGUID("9eb473e5-6ef2-d6a8-9a31-8f4b26eb9f12"));
	auto normalTex = ContentManager::Get()->LoadAsset<Texture2D>(HGUID("fa3e3e63-872f-99e3-a126-6ce5b8fedfae"));
	auto whiteTex = ContentManager::Get()->LoadAsset<Texture2D>(HGUID("38c42242-404c-b372-fac8-d0441f2100f8"));
	auto testTex = ContentManager::Get()->LoadAsset<Texture2D>(HGUID("eb8ac147-e469-f5a3-c48a-5daec8880f1f"));
	auto blackCubeMapTex = ContentManager::Get()->LoadAsset<TextureCube>(HGUID("f1490ebb-c873-4baa-b4bf-e8292af28cf5"));

	if (uvGridTex)
	{
		uvGridTex->SetSystemAsset(true);
		Texture2D::AddSystemTexture("UVGrid", uvGridTex);
	}
	if (whiteTex)
	{
		whiteTex->SetSystemAsset(true);
		Texture2D::AddSystemTexture("White", whiteTex);
	}
	if (blackTex)
	{
		blackTex->SetSystemAsset(true);
		Texture2D::AddSystemTexture("Black", blackTex);
	}
	if (normalTex)
	{
		normalTex->SetSystemAsset(true);
		Texture2D::AddSystemTexture("Normal", normalTex);
	}
	if (testTex)
	{
		testTex->SetSystemAsset(true);
		Texture2D::AddSystemTexture("TestTex", testTex);
	}
	if (blackCubeMapTex)
	{
		blackCubeMapTex->SetSystemAsset(true);
		Texture2D::AddSystemTexture("CubeMapBalck", blackCubeMapTex);
	}
}

void Texture2D::GlobalUpdate()
{
}

void Texture2D::GlobalRelease()
{
	const auto& manager = VulkanManager::GetManager();
	for (auto i : _samplers)
	{
		vkDestroySampler(manager->GetDevice(), i, nullptr);
	}
	_samplers.clear();
	_system_textures.clear();
}

void Texture2D::AddSystemTexture(HString tag, std::weak_ptr<Texture2D> tex)
{
	//系统纹理是渲染器底层预设纹理，需要直接准备就绪
	tex.lock()->CopyBufferToTextureImmediate();
	_system_textures.emplace(tag, tex);
}

std::shared_ptr<Texture2D> Texture2D::GetSystemTexture(HString tag)
{
	//_system_textures[tag];
	auto it = _system_textures.find(tag);
	if (it != _system_textures.end())
	{
		return it->second.lock();
	}
	return _system_textures.begin()->second.lock();
}

bool Texture2D::CopyBufferToTexture(VkCommandBuffer cmdbuf)
{
	const auto& manager = VulkanManager::GetManager();
	{
		//上一帧已经复制完成了,可以删除Upload buffer 和Memory了
		if (IsValid())
		{
			if (_uploadBuffer != VK_NULL_HANDLE)
			{
				vkDestroyBuffer(manager->GetDevice(), _uploadBuffer, nullptr);
				_uploadBuffer = VK_NULL_HANDLE;
			}
			if (_uploadBufferMemory != VK_NULL_HANDLE)
			{
				vkFreeMemory(manager->GetDevice(), _uploadBufferMemory, nullptr);
				_uploadBufferMemory = VK_NULL_HANDLE;
			}
			return true;
		}
		else if (_imageData.imageData.size() > 0 || _imageData.imageDataF.size() > 0)
		{
			//Copy image data to upload memory
			uint64_t imageSize = _imageData.imageSize;

			//Cube 有6面
			uint32_t faceNum = 1;
			if (_imageData.isCubeMap)
			{
				faceNum = 6;
			}
			imageSize *= faceNum;
			manager->CreateBufferAndAllocateMemory(
				_textureMemorySize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				_uploadBuffer,
				_uploadBufferMemory);
			//创建Buffer储存Image data
			if (_imageData.imageData.size() > 0)
			{
				//char 只有一个字节，所以数组大小和imageSize一致
				void* data = nullptr;
				vkMapMemory(manager->GetDevice(), _uploadBufferMemory, 0, _textureMemorySize, 0, &data);
				memcpy(data, _imageData.imageData.data(), _imageData.imageData.size());
			}
			else if (_imageData.imageDataF.size() > 0)
			{
				//float 有4个字节，所以size = 数组大小 * sizeof(float)
				void* data = nullptr;
				vkMapMemory(manager->GetDevice(), _uploadBufferMemory, 0, _textureMemorySize, 0, &data);
				memcpy(data, _imageData.imageDataF.data(), (size_t)_imageData.imageSize);
			}
			vkUnmapMemory(manager->GetDevice(), _uploadBufferMemory);

			//从Buffer copy到Vkimage
			//if (_imageData->blockSize > 0)
			{
				if (_imageData.isCubeMap)
					Transition(cmdbuf, _imageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, _imageData.mipLevel, 0, 6);
				else
					Transition(cmdbuf, _imageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, _imageData.mipLevel);

				std::vector<VkBufferImageCopy>region(_imageData.mipLevel * faceNum);
				int regionIndex = 0;
				uint32_t bufferOffset = 0;
				for (uint32_t face = 0; face < faceNum; face++)
				{
					uint32_t mipWidth = _imageData.data_header.width;
					uint32_t mipHeight = _imageData.data_header.height;
					for (uint32_t i = 0; i < _imageData.mipLevel; i++)
					{
						region[regionIndex] = {};
						region[regionIndex].bufferOffset = bufferOffset;
						region[regionIndex].bufferRowLength = 0;
						region[regionIndex].bufferImageHeight = 0;
						region[regionIndex].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
						region[regionIndex].imageSubresource.mipLevel = i;
						region[regionIndex].imageSubresource.baseArrayLayer = face;
						region[regionIndex].imageSubresource.layerCount = 1;
						region[regionIndex].imageOffset = { 0,0,0 };
						region[regionIndex].imageExtent = { mipWidth ,mipHeight,1 };
						if (_imageData.blockSize > 0)
							bufferOffset += SIZE_OF_BC((int32_t)mipWidth, (int32_t)mipHeight, _imageData.blockSize);
						else
							bufferOffset += mipWidth * mipHeight;
						if (mipWidth > 1) mipWidth /= 2;
						if (mipHeight > 1) mipHeight /= 2;
						regionIndex++;
					}
				}
				vkCmdCopyBufferToImage(cmdbuf, _uploadBuffer, _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (uint32_t)region.size(), region.data());

				//复制完成后,把Layout转换到Shader read only,给shader采样使用
				if (_imageData.isCubeMap)
					Transition(cmdbuf, _imageLayout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, _imageData.mipLevel, 0, 6);
				else
					Transition(cmdbuf, _imageLayout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, _imageData.mipLevel);
			}
			//上传完成,清除图像的CPU数据
			_imageData.imageData.clear();
			_imageData.imageDataF.clear();
			return false;
		}
	}
	//上传完成,标记
	_bUploadToGPU++;
	return false;
}

void Texture2D::CopyBufferToTextureImmediate()
{
	const auto& manager = VulkanManager::GetManager();
	VkCommandBuffer buf;
	manager->AllocateCommandBuffer(manager->GetCommandPool(), buf);
	manager->BeginCommandBuffer(buf, 0);
	{
		CopyBufferToTexture(buf);
	}
	manager->EndCommandBuffer(buf);
	manager->SubmitQueueImmediate({ buf });
	manager->FreeCommandBuffer(manager->GetCommandPool(), buf);
	//
	//if (IsValid())
	{
		if (_uploadBuffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(manager->GetDevice(), _uploadBuffer, nullptr);
			_uploadBuffer = VK_NULL_HANDLE;
		}
		if (_uploadBufferMemory != VK_NULL_HANDLE)
		{
			vkFreeMemory(manager->GetDevice(), _uploadBufferMemory, nullptr);
			_uploadBufferMemory = VK_NULL_HANDLE;
		}
	}
	auto it = std::find(_upload_textures.begin(), _upload_textures.end(), this);
	if (it != _upload_textures.end())
	{
		_upload_textures.erase(it);
	}
}

void Texture2D::DestoryUploadBuffer()
{
	const auto& manager = VulkanManager::GetManager();
	if (_uploadBuffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(manager->GetDevice(), _uploadBuffer, nullptr);
		_uploadBuffer = VK_NULL_HANDLE;
	}
	if (_uploadBufferMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory(manager->GetDevice(), _uploadBufferMemory, nullptr);
		_uploadBufferMemory = VK_NULL_HANDLE;
	}
}

bool Texture2D::CopyTextureToBuffer(VkCommandBuffer cmdbuf, VkBuffer buffer, VkDeviceSize offset)
{
	VkBufferImageCopy copy = {};
	copy.bufferOffset = offset;
	copy.bufferRowLength = 0;
	copy.bufferImageHeight = 0;
	copy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	copy.imageOffset = { 0, 0, 0 };
	copy.imageExtent = _imageSize;
	copy.imageExtent.depth = 1;
	vkCmdCopyImageToBuffer(cmdbuf, _image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer, 1, &copy);
	return true;
}

void Texture2D::CopyTextureToBufferImmediate(VkBuffer buffer, VkDeviceSize offset)
{
	const auto& manager = VulkanManager::GetManager();
	VkCommandBuffer buf;
	manager->AllocateCommandBuffer(manager->GetCommandPool(), buf);
	manager->BeginCommandBuffer(buf, 0);
	{
		CopyTextureToBuffer(buf, buffer, offset);
	}
	manager->EndCommandBuffer(buf);
	manager->SubmitQueueImmediate({ buf });
	vkQueueWaitIdle(VulkanManager::GetManager()->GetGraphicsQueue());
	manager->FreeCommandBuffer(manager->GetCommandPool(), buf);
}