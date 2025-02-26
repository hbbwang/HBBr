﻿#include "HDRI2Cube.h"
#include "VulkanRenderer.h"
#include "Model.h"
#include "PassManager.h"
#include "ImageTool.h"
#include "FormMain.h"
#include "NvidiaTextureTools.h"
#include "VMABuffer.h"

HDRI2Cube::HDRI2Cube()
	:ComputePass(nullptr)
{
}

HDRI2Cube::~HDRI2Cube()
{
	auto* manager = VulkanManager::GetManager();
	ReleasePass();
	_hdriTexture.reset();
	manager->DestroyPipelineLayout(_pipelineLayout);
	manager->DestroyDescriptorSetLayout(_storeDescriptorSetLayout);
}

bool HDRI2Cube::PassExecute(std::string hdrImagePath, std::string ddsOutputPath, bool bGenerateMips, int cubeMapSize)
{
	auto* manager = VulkanManager::GetManager();

	//Init
	_hdrImagePath = hdrImagePath;
	_renderer = VulkanApp::GetMainForm()->swapchain->GetRenderers().begin()->second;

	//Load hdr image
	auto imageData = ImageTool::LoadImage32Bit(hdrImagePath.c_str());
	uint32_t w = imageData->data_header.width;
	uint32_t h = imageData->data_header.height;

	//Load HDRI image
	_hdriTexture = Texture2D::CreateTexture2D(
		w, h,
		VK_FORMAT_R32G32B32A32_SFLOAT,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		std::string("Provisional HDRI Texture")
	);
	_hdriTexture->_imageData = *imageData;
	_hdriTexture->CopyBufferToTextureImmediate();

	//Create DescriptorSetLayout
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

	//Create PipelineLayout
	manager->CreatePipelineLayout(
		{
			_storeDescriptorSetLayout
		}
	, _pipelineLayout);

	//Create DescriptorSet
	store_descriptorSet.reset(new DescriptorSet(_renderer));
	store_descriptorSet->CreateBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT);
	store_descriptorSet->BuildDescriptorSetLayout();

	//Create uniform buffer
	_uniformBuffer.reset(new VMABuffer(sizeof(HDRI2CubeUnifrom), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, false, true, "HDRI2CubeUnifrom"));

	//Set cube map output size
	if (cubeMapSize < 1)
	{
		cubeMapSize = _hdriTexture->_imageData.data_header.height;
	}

	//Pass Execute
	{
		//Create Store Texture
		for (int i = 0; i < 6; i++)
		{
			_storeTexture[i] = Texture2D::CreateTexture2D(
				cubeMapSize, cubeMapSize,
				VK_FORMAT_R32G32B32A32_SFLOAT,
				VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
				std::string("Cube Map Store Texture")
			);
		}

		{
			VkCommandBuffer cmdBuf;
			manager->AllocateCommandBuffer(manager->GetCommandPool(), cmdBuf);
			manager->BeginCommandBuffer(cmdBuf);

			//Transition
			for (int i = 0; i < 6; i++)
			{
				_storeTexture[i]->Transition(cmdBuf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
			}

			//Begin...
			// 
			//设置管线
			//Create GUIShader Pipeline
			VkPipeline pipeline = VK_NULL_HANDLE;
			auto csCache = Shader::_csShader["HDRI2Cube@0"];
			VkComputePipelineCreateInfoCache pipelineCreateInfo = {};
			PipelineManager::SetComputeShader(pipelineCreateInfo, csCache.get());
			PipelineManager::SetPipelineLayout(pipelineCreateInfo, _pipelineLayout);
			PipelineManager::BuildComputePipelineState(pipelineCreateInfo, pipeline);
			PipelineIndex index = PipelineIndex::GetPipelineIndex(csCache);
			std::unique_ptr<PipelineObject> newPSO = std::make_unique<PipelineObject>();
			newPSO->pipeline = pipeline;
			newPSO->layout = _pipelineLayout;
			newPSO->pipelineType = PipelineType::Compute;
			{
				//textures
				store_descriptorSet->RefreshDescriptorSet(0);
				std::vector<Texture2D* > storeTexs = {
					_storeTexture[0].get(),
					_storeTexture[1].get(),
					_storeTexture[2].get(),
					_storeTexture[3].get(),
					_storeTexture[4].get(),
					_storeTexture[5].get()
				};
				store_descriptorSet->UpdateStoreTextureDescriptorSet(storeTexs, 0);
				store_descriptorSet->UpdateTextureDescriptorSet({ _hdriTexture }, { Texture2D::GetSampler(TextureSampler::TextureSampler_Linear_Clamp) }, 6);

				//uniform
				HDRI2CubeUnifrom u = {};
				u.CubeMapSize = (float)cubeMapSize;
				_uniformBuffer->Mapping(&u, 0, sizeof(HDRI2CubeUnifrom));
				auto alignmentSize = manager->GetMinUboAlignmentSize(sizeof(HDRI2CubeUnifrom));
				manager->UpdateBufferDescriptorSet(_uniformBuffer->GetBuffer(), store_descriptorSet->GetDescriptorSet(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 7, 0, alignmentSize);
			}
			{
				manager->CmdCmdBindPipeline(cmdBuf, pipeline, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE);
				//textures
				vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelineLayout, 0, 1, &store_descriptorSet->GetDescriptorSet(), 0, 0);
				//vkCmdSetScissor(cmdBuf, 0, 1, &i.second.viewport);
				vkCmdDispatch(cmdBuf, cubeMapSize / 8, cubeMapSize / 8, 1);
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

	std::shared_ptr<VMABuffer> copyTextureToBuffer;
	copyTextureToBuffer.reset(new VMABuffer(
		_storeTexture[0]->GetTextureMemorySize() * 6,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY,false,true,"HDRI2Cube-Texture2Buffer"));
	//Copy store texture to buffer
	{
		VkDeviceSize copy_offset = 0;
		for (int i = 0; i < 6; i++)
		{
			_storeTexture[i]->CopyTextureToBufferImmediate(copyTextureToBuffer->GetBuffer(), copy_offset);
			copy_offset += _storeTexture[i]->GetTextureMemorySize();
		}
	}

#if IS_EDITOR

	std::shared_ptr<VMABuffer> copBufferToCPU;
	copBufferToCPU.reset(new VMABuffer(
		_storeTexture[0]->GetTextureMemorySize() * 6,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_GPU_TO_CPU, false, true, "HDRI2Cube-Buffer2File"));


	//Copy GPUBuffer to CPUBuffer
	manager->CmdBufferCopyToBuffer(nullptr, copyTextureToBuffer->GetBuffer(), copBufferToCPU->GetBuffer(), _storeTexture[0]->GetTextureMemorySize() * 6);
	void* textureData = copBufferToCPU->BeginMapping();
	std::string hdrCubeMapCachePath = FileSystem::Append(FileSystem::GetFilePath(_hdrImagePath), FileSystem::GetBaseName(_hdrImagePath) + "_HDRCubeMapCache.hdr");
	//Export cube map cache
	{
		//生成临时HDR图
		//if (!stbi_write_hdr(hdrCubeMapCachePath.c_str(), cubeMapSize, cubeMapSize * 6, 4, (float*)cubeBuffer->GetBufferMemory()))
		if (!ImageTool::SaveImage32Bit(hdrCubeMapCachePath.c_str(), cubeMapSize, cubeMapSize * 6, 4, (float*)textureData))
		{
			ConsoleDebug::print_endl("Save output image failed", "255,255,0");
			return false;
		}
	}
	copBufferToCPU->EndMapping();


	//NVTT spawn cube map dds
	{
		using namespace nvtt;
		//创建上下文
		Context context;
		//启动Cuda加速
		context.enableCudaAcceleration(true);
		//纹理加载
		Surface image;
		bool hasAlpha = false;
		if (!image.load(hdrCubeMapCachePath.c_str(), &hasAlpha))
		{
			ConsoleDebug::print("Compression Image Cube Failed.Image load failed.", "255,255,0");
			//删除缓存图。
			FileSystem::FileRemove(hdrCubeMapCachePath.c_str());
			return false;
		}

		//cube map 需要把HDR图拆出6份，必须保证不能小于6
		if (image.height() < 6)
		{
			ConsoleDebug::print("Compression Image Cube Failed.Because HDR image height small than 6.", "255,255,0");
			//删除缓存图。
			FileSystem::FileRemove(hdrCubeMapCachePath.c_str());
			return false;
		}

		//转线性颜色(HDR图像一般就是非sRGB所以可以不做这个操作)
		image.toLinear(1.0);

		//导出设置
		OutputOptions output;
		FileSystem::CreateDir(FileSystem::GetFilePath(ddsOutputPath).c_str());
		output.setFileName(ddsOutputPath.c_str());

		//设置图像HDR格式
		CompressionOptions options;
		options.setFormat(Format_BC6U);
		options.setQuality(Quality_Normal);

		//把HDR纹理转换成Cube
		CubeSurface cubeImage;

		//因为是长条图，使用竖状排版拆分
		cubeImage.fold(image, CubeLayout_Column);

		//计算mipLevel
		int mipmaps = 1;
		if (bGenerateMips)
			mipmaps = cubeImage.countMipmaps();

		//设置DDS文件头
		context.outputHeader(cubeImage, mipmaps, options, output);
		//导出
		for (int f = 0; f < 6; f++)
		{
			for (int i = 0; i < mipmaps; i++)
			{
				context.compress(cubeImage.face(f), i, 1, options, output);
				if (cubeImage.face(f).canMakeNextMipmap())
					cubeImage.face(f).buildNextMipmap(MipmapFilter_Triangle);
			}
		}

		//删除缓存图。
		FileSystem::FileRemove(hdrCubeMapCachePath.c_str());
	}

#endif

	ReleasePass();
	return  true;
}

void HDRI2Cube::ReleasePass()
{
	for (int i = 0; i < 6; i++)
	{
		_storeTexture[i].reset();
	}
	_uniformBuffer.reset();
}

