#include "Texture.h"
#include "VulkanRenderer.h"

Texture::~Texture()
{
	VulkanRenderer::GetManager()->DestroyImageMemory(_imageViewMemory);
	VulkanRenderer::GetManager()->DestroyImageView(_imageView);
	VulkanRenderer::GetManager()->DestroyImage(_image);
}

std::shared_ptr<Texture> Texture::CreateTexture2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usageFlags)
{
	std::shared_ptr<Texture> newTexture = std::make_shared<Texture>();
	VulkanRenderer::GetManager()->CreateImage(width,height,format , usageFlags, newTexture->_image);
	VkImageAspectFlags aspectFlag{};
	if (format == VK_FORMAT_R32_SFLOAT || format == VK_FORMAT_D32_SFLOAT)
		aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
	else if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D16_UNORM_S8_UINT)
		aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	else
		aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
	VulkanRenderer::GetManager()->CreateImageView(newTexture->_image, format, aspectFlag, newTexture->_imageView);
	if (!newTexture->_bNoMemory)
	{
		VulkanRenderer::GetManager()->CreateImageMemory(newTexture->_image, newTexture->_imageViewMemory);
	}
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
