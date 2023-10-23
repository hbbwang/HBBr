#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include "PassBase.h"
#include "HGuid.h"

struct GUITexture
{
	VkImageView ImageView;
	VkDescriptorSet DescriptorSet;
};

struct GUIBuffer
{
	VkBuffer vb;
	VkDeviceMemory vbm;
	VkBuffer ib;
	VkDeviceMemory ibm;
	VkBuffer ub;
	VkDeviceMemory ubm;
};

struct GUIUniformBuffer
{
	glm::mat4 Projection;
};

class GUIPass :public GraphicsPass
{
public:
	GUIPass(VulkanRenderer* renderer) :GraphicsPass(renderer) {}
	virtual ~GUIPass();
	virtual void PassInit()override;
	virtual void PassUpdate()override;
	virtual void PassReset()override;
private:
	void create_texture_descriptor_sets();
	void nk_device_upload_atlas(VkQueue graphics_queue,const void* image, int width,int height);
	void nk_sdl_font_stash_begin(struct nk_font_atlas** atlas);
	void nk_sdl_font_stash_end(void);

	std::shared_ptr<class DescriptorSet> _descriptorSet;
	uint32_t texture_descriptor_sets_len = 0 ;
	GUIBuffer _buffer;
	std::unordered_map<HString, VkPipeline> _guiPipelines;
	VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
	VkDescriptorSetLayout _texDescriptorSetLayout = VK_NULL_HANDLE;
	VkPipeline _pipeline = VK_NULL_HANDLE;
	std::vector<GUITexture> _guiTextures;
	VkImage _fontImage = VK_NULL_HANDLE;
	VkImageView _fontImageView = VK_NULL_HANDLE;
	VkDeviceMemory _fontMemory = VK_NULL_HANDLE;
	bool bSetFont = false;
};
