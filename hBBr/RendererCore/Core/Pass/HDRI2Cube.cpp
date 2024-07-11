#include "HDRI2Cube.h"
#include "VulkanRenderer.h"
#include "Model.h"
#include "PassManager.h"
#include "ImageTool.h"
#include "FormMain.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

HDRI2Cube::HDRI2Cube(HString hdrImagePath, HString ddsOutputPath, bool bGenerateMips, int cubeMapSize)
	:ComputePass(nullptr)
{
	_hdrImagePath = hdrImagePath;

	_ddsOutputPath = ddsOutputPath;

	_bGenerateMips = bGenerateMips;

	_renderer = VulkanApp::GetMainForm()->renderer;

	auto imageData = ImageTool::ReadHDRImage(hdrImagePath.c_str());
	uint32_t w = imageData->data_header.width;
	uint32_t h = imageData->data_header.height;

	if (cubeMapSize >= 1)
	{
		_cubeMapFaceSize = cubeMapSize;
	}
	else
	{
		_cubeMapFaceSize = h;
	}

	//Load HDRI image
	_hdriTexture = Texture2D::CreateTexture2D(
		w, h,
		VK_FORMAT_R32G32B32A32_SFLOAT,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		HString("Provisional HDRI Texture")
	); 
	_hdriTexture->_imageData = *imageData;
	_hdriTexture->CopyBufferToTextureImmediate();

	//Create Store Texture
	for (int i = 0; i < 6; i++)
	{
		_storeTexture[i] = Texture2D::CreateTexture2D(
			_cubeMapFaceSize, _cubeMapFaceSize,
			VK_FORMAT_R32G32B32A32_SFLOAT,
			VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			HString("Cube Map Store Texture")
		);
	}

	//Init pass
	const auto& manager = VulkanManager::GetManager();

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

	store_descriptorSet.reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, _storeDescriptorSetLayout, 0, VK_SHADER_STAGE_COMPUTE_BIT));

	auto alignmentSize = manager->GetMinUboAlignmentSize(sizeof(HDRI2CubeUnifrom));

	_uniformBuffer.reset(new Buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, alignmentSize));

	//Execute
	{
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
				store_descriptorSet->UpdateTextureDescriptorSet({ _hdriTexture.get() }, { Texture2D::GetSampler(TextureSampler::TextureSampler_Linear_Clamp) }, 6);

				//uniform
				HDRI2CubeUnifrom u = {};
				u.CubeMapSize = _cubeMapFaceSize;
				_uniformBuffer->BufferMapping(&u, 0, sizeof(HDRI2CubeUnifrom));
				auto alignmentSize = manager->GetMinUboAlignmentSize(sizeof(HDRI2CubeUnifrom));
				manager->UpdateBufferDescriptorSet(_uniformBuffer->GetBuffer(), store_descriptorSet->GetDescriptorSet(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 7, 0, alignmentSize);
			}
			{
				manager->CmdCmdBindPipeline(cmdBuf, pipeline, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE);
				//textures
				vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelineLayout, 0, 1, &store_descriptorSet->GetDescriptorSet(), 0, 0);
				//vkCmdSetScissor(cmdBuf, 0, 1, &i.second.viewport);
				vkCmdDispatch(cmdBuf, _cubeMapFaceSize / 8, _cubeMapFaceSize / 8, 1);
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

	//等待ComputeShader执行完成之后，导出图像
	std::shared_ptr<Buffer> cubeBuffer;
	cubeBuffer.reset(new Buffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		_storeTexture[0]->GetTextureMemorySize() * 6
	));
	VkDeviceSize copy_offset = 0;
	for (int i = 0; i < 6; i++)
	{
		_storeTexture[i]->CopyTextureToBufferImmediate(cubeBuffer.get(), copy_offset);
		copy_offset += _storeTexture[i]->GetTextureMemorySize();
	}

#if IS_EDITOR

	HString hdrCubeMapCachePath = FileSystem::Append(_hdrImagePath.GetFilePath(), _hdrImagePath.GetBaseName() + "_HDRCubeMapCache.hdr");
	//生成临时HDR图
	if (!stbi_write_hdr(hdrCubeMapCachePath.c_str(), _cubeMapFaceSize, _cubeMapFaceSize * 6, 4, (float*)cubeBuffer->GetBufferMemory()))
	{
		ConsoleDebug::print_endl("Save output image failed", "255,255,0");
		return;
	}

	//nvtt 生成dds
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
			return;
		}

		//cube map 需要把HDR图拆出6份，必须保证不能小于6
		if (image.height() < 6)
		{
			ConsoleDebug::print("Compression Image Cube Failed.Because HDR image height small than 6.", "255,255,0");
			//删除缓存图。
			FileSystem::FileRemove(hdrCubeMapCachePath.c_str());
			return;
		}

		//转线性颜色(HDR图像一般就是非sRGB所以可以不做这个操作)
		image.toLinear(1.0);

		//导出设置
		OutputOptions output;
		output.setFileName(_ddsOutputPath.c_str());

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
		if (_bGenerateMips)
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

	cubeBuffer->UnMapMemory();
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

