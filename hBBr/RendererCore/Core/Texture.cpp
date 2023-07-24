#include "Texture.h"
#include "VulkanRenderer.h"
#include "VulkanManager.h"

Texture::~Texture()
{
	VulkanRenderer::GetManager()->DestroyImageMemory(_imageViewMemory);
	VulkanRenderer::GetManager()->DestroyImageView(_imageView);
	VulkanRenderer::GetManager()->DestroyImage(_image);
}

void Texture::Transition(VkCommandBuffer cmdBuffer, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevelBegin, uint32_t mipLevelCount)
{
	VulkanRenderer::GetManager()->Transition(cmdBuffer, _image, _imageAspectFlags, oldLayout, newLayout, mipLevelBegin, mipLevelCount);
	_imageLayout = newLayout;
}

std::shared_ptr<Texture> Texture::CreateTexture2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usageFlags, HString textureName, bool noMemory)
{
	std::shared_ptr<Texture> newTexture = std::make_shared<Texture>();
	newTexture->_bNoMemory = noMemory;
	newTexture->_textureName = textureName;
	newTexture->_imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VulkanRenderer::GetManager()->CreateImage(width,height,format , usageFlags, newTexture->_image);
	if (format == VK_FORMAT_R32_SFLOAT || format == VK_FORMAT_D32_SFLOAT)
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
	else if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D16_UNORM_S8_UINT)
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	else
		newTexture->_imageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	if (!newTexture->_bNoMemory)
	{
		VulkanRenderer::GetManager()->CreateImageMemory(newTexture->_image, newTexture->_imageViewMemory);
	}
	VulkanRenderer::GetManager()->CreateImageView(newTexture->_image, format, newTexture->_imageAspectFlags, newTexture->_imageView);
	newTexture->_format = format;
	newTexture->_usageFlags = usageFlags;
	return std::move(newTexture);
}

FrameBufferTexture::~FrameBufferTexture()
{
	_frameBufferTextures.clear();
}

std::shared_ptr<FrameBufferTexture> FrameBufferTexture::CreateFrameBuffer(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usageFlags)
{
	std::shared_ptr<FrameBufferTexture> newTexture = std::make_shared<FrameBufferTexture>();
	for (int i = 0; i < VulkanRenderer::GetManager()->GetSwapchainBufferCount(); i++)
	{
		newTexture->_frameBufferTextures.push_back(Texture::CreateTexture2D(width,height,format,usageFlags));
	}
	return std::move(newTexture);
}

std::shared_ptr<Texture> FrameBufferTexture::GetBuffer()
{
	return _frameBufferTextures[VulkanRenderer::GetCurrentFrameIndex()];
}
