#include "HDRI2Cube.h"
#include "VulkanRenderer.h"
#include "Model.h"
#include "PassManager.h"
#include "ImageTool.h"
#include "FormMain.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

HDRI2Cube::HDRI2Cube(HString imagePath)
	:ComputePass(nullptr)
{
	_renderer = VulkanApp::GetMainForm()->renderer;

	auto imageData = ImageTool::ReadHDRImage(imagePath.c_str());
	uint32_t w = imageData->data_header.width;
	uint32_t h = imageData->data_header.height;

	_cubeMapFaceSize = { w / 4, w / 4 };

	_hdriTexture = Texture2D::CreateTexture2D(
		w, h,
		VK_FORMAT_R32G32B32A32_SFLOAT,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		HString("Provisional HDRI Texture")
	); 
	_hdriTexture->_imageData = *imageData;
	_hdriTexture->CopyBufferToTextureImmediate();
	//

	for (int i = 0; i < 6; i++)
	{
		_storeTexture[i] = Texture2D::CreateTexture2D(
			_cubeMapFaceSize.width, _cubeMapFaceSize.height,
			VK_FORMAT_R32G32B32A32_SFLOAT,
			VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			HString("Cube Map Store Texture")
		);
		_storeTexture[i]->TransitionImmediate(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
	}
}

HDRI2Cube::~HDRI2Cube()
{
	const auto& manager = VulkanManager::GetManager();

	for (int i = 0; i < 6; i++)
	{
		_storeTexture[i].reset();
	}
	_hdriTexture.reset();
	manager->DestroyPipelineLayout(_pipelineLayout);
	manager->DestroyDescriptorSetLayout(_storeDescriptorSetLayout);
}

void HDRI2Cube::PassInit()
{
	const auto& manager = VulkanManager::GetManager();

	//DescriptorSet
	//manager->CreateDescripotrSetLayout(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2, _storeDescriptorSetLayout, VK_SHADER_STAGE_COMPUTE_BIT);
	manager->CreateDescripotrSetLayout(
		{ 
			VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ,
			VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ,
			VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ,
			VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ,
			VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ,
			VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
		}, _storeDescriptorSetLayout, VK_SHADER_STAGE_COMPUTE_BIT);
	manager->CreatePipelineLayout(
		{
			_storeDescriptorSetLayout
		}
	, _pipelineLayout);

	//Set Pass Name
	_passName = "HDRI to CubeMap Pass";
	store_descriptorSet.reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, _storeDescriptorSetLayout, 0, VK_SHADER_STAGE_COMPUTE_BIT));

	auto alignmentSize = VulkanManager::GetManager()->GetMinUboAlignmentSize(sizeof(HDRI2CubeUnifrom));
	_uniformBuffer.reset(new Buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, alignmentSize));


}

void HDRI2Cube::PassExecute()
{
	const auto& manager = VulkanManager::GetManager();
	{
		{
			manager->DeviceWaitIdle();
			VkCommandBuffer cmdBuf;
			manager->AllocateCommandBuffer(manager->GetCommandPool(), cmdBuf);
			manager->BeginCommandBuffer(cmdBuf);
			//Begin...
			// 
			//设置管线
			//Create GUIShader Pipeline
			auto pipelineObject = PipelineManager::GetComputePipelineMap(_pipelineIndex);
			if (pipelineObject == nullptr)
			{
				_pipelineIndex = CreatePipeline();
				pipelineObject = PipelineManager::GetComputePipelineMap(_pipelineIndex);
			}
			{
				//textures
				store_descriptorSet->NeedUpdate();
				std::vector<Texture2D* > storeTexs = {
					_storeTexture[0].get(),
					_storeTexture[1].get(),
					_storeTexture[2].get(),
					_storeTexture[3].get(),
					_storeTexture[4].get(),
					_storeTexture[5].get()
				};
				store_descriptorSet->UpdateStoreTextureDescriptorSet(storeTexs);
				store_descriptorSet->NeedUpdate();
				store_descriptorSet->UpdateTextureDescriptorSet({ _hdriTexture.get() }, { Texture2D::GetSampler(TextureSampler::TextureSampler_Nearest_Clamp) }, 6);

				//uniform
				HDRI2CubeUnifrom u = {};
				u.HDRITextureSize = glm::vec2(_hdriTexture->GetTextureSize().width, _hdriTexture->GetTextureSize().height);
				_uniformBuffer->BufferMapping(&u, 0, sizeof(HDRI2CubeUnifrom));
				auto alignmentSize = VulkanManager::GetManager()->GetMinUboAlignmentSize(sizeof(HDRI2CubeUnifrom));
				VulkanManager::GetManager()->UpdateBufferDescriptorSet(_uniformBuffer->GetBuffer(), store_descriptorSet->GetDescriptorSet(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 7, 0, alignmentSize);
			}
			VkPipeline pipeline = VK_NULL_HANDLE;
			{
				VkPipeline currentPipeline = pipelineObject->pipeline;
				if (currentPipeline != pipeline)
				{
					pipeline = currentPipeline;
					manager->CmdCmdBindPipeline(cmdBuf, pipeline, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE);
				}
				//textures
				vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelineLayout, 0, 1, &store_descriptorSet->GetDescriptorSet(), 0, 0);
				//vkCmdSetScissor(cmdBuf, 0, 1, &i.second.viewport);
				vkCmdDispatch(cmdBuf, _cubeMapFaceSize.width / 8, _cubeMapFaceSize.height / 8, 6);
			}
			//End...
			
			for (int i = 0; i < 6; i++)
			{
				_storeTexture[i]->Transition(cmdBuf, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
			}

			manager->EndCommandBuffer(cmdBuf);
			manager->SubmitQueueImmediate({ cmdBuf });
			vkQueueWaitIdle(manager->GetGraphicsQueue());
			manager->FreeCommandBuffer(manager->GetCommandPool(), cmdBuf);
		}
	}

	//导出图像
	std::shared_ptr<Buffer> buffer;
	buffer.reset(new Buffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
		_storeTexture[0]->GetTextureMemorySize() * 6
	));

	VkDeviceSize copy_offset = 0;
	for (int i = 0; i < 6; i++)
	{
		_storeTexture[i]->CopyTextureToBufferImmediate(buffer.get(), copy_offset);
		copy_offset += _storeTexture[i]->GetTextureMemorySize();
	}

	if (!stbi_write_hdr((HString("D:/sss") + ".hdr").c_str(), _cubeMapFaceSize.width, _cubeMapFaceSize.height * 6, 4, (float*)buffer->GetBufferMemory()))
	{
		ConsoleDebug::print_endl("Save output image failed", "255,255,0");
	}
	buffer->UnMapMemory();

}

PipelineIndex HDRI2Cube::CreatePipeline()
{
	VkPipeline pipeline = VK_NULL_HANDLE;
	auto csCache = Shader::_csShader["HDRI2Cube@0"];
	VkComputePipelineCreateInfoCache pipelineCreateInfo = {};
	PipelineManager::SetComputeShader(pipelineCreateInfo, csCache.get());
	PipelineManager::SetPipelineLayout(pipelineCreateInfo, _pipelineLayout);
	PipelineManager::BuildComputePipelineState(pipelineCreateInfo, pipeline);
	return PipelineManager::AddPipelineObject(csCache, pipeline, _pipelineLayout);
}
