#pragma once
#include <memory>
#include "VulkanManager.h"

struct EditorImage
{
	VkImage image; 
	VkImageView imageView;
	VmaAllocation imageAllocation;
	VmaAllocationInfo imageAllocationInfo = {};
	VkDescriptorSet descriptorSet;

	//������Ҫ�м��ߴ�������
	VkImage stageImage;
	VmaAllocation stageImageAllocation;
	VmaAllocationInfo stageImageAllocationInfo = {};

	//���������ⲿͼ�����ݵģ��ȴ����ݴ��䵽GPU֮��Ϳ���ж����
	std::shared_ptr<ImageData> imageData;
	std::shared_ptr<VMABuffer> vmaBuffer;
};
 
class EditorResource
{
public:
	inline static EditorResource* Get()
	{
		if (!_ptr)
		{
			_ptr.reset(new EditorResource());
		}
		return _ptr.get();
	}

	void Init();
	void Release();

	//Imgui images
	EditorImage* LoadTexture(uint32_t w, uint32_t h, HString path, VkCommandBuffer cmdBuf);

	EditorImage* _icon_eyeOpen;
	EditorImage* _icon_eyeClose;
	EditorImage* _icon_levelIcon;
	EditorImage* _icon_objectIcon;
	EditorImage* _icon_search;

	std::vector<EditorImage> _all_images;

private:
	static std::unique_ptr<EditorResource> _ptr;
	VkDescriptorSetLayout _img_descriptorSetLayout;
};

