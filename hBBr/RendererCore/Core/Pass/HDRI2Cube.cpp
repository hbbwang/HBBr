#include "HDRI2Cube.h"
#include "VulkanRenderer.h"
#include "Model.h"
#include "PassManager.h"
#include "ImageTool.h"
#include "FormMain.h"

#define OUTPUT_FORMAT VK_FORMAT_R16G16B16A16_SFLOAT

HDRI2Cube::HDRI2Cube(HString imagePath)
	:GraphicsPass(nullptr)
{
	_renderer = VulkanApp::GetMainForm()->renderer;

	auto imageData = ImageTool::ReadHDRImage(imagePath.c_str());
	uint32_t w = imageData->data_header.width;
	uint32_t h = imageData->data_header.height;

	_hdriTexture = Texture2D::CreateTexture2D(
		w, h,
		imageData->texFormat,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		HString("Provisional HDRI Texture")
	); 

	_hdriTexture->_imageData = *imageData;
	_cubeMapFaceSize = h/2;
	_hdriTexture->CopyBufferToTextureImmediate();
}

HDRI2Cube::~HDRI2Cube()
{
	const auto& manager = VulkanManager::GetManager();
	_vertexBuffer.reset();
	manager->DestroyPipelineLayout(_pipelineLayout);
	manager->DestroyDescriptorSetLayout(_texDescriptorSetLayout);
	manager->DestroyDescriptorSetLayout(_ubDescriptorSetLayout);
	_hdriTexture.reset();
}

void HDRI2Cube::PassInit()
{
	const auto& manager = VulkanManager::GetManager();
	AddAttachment(
		VK_ATTACHMENT_LOAD_OP_CLEAR, 
		VK_ATTACHMENT_STORE_OP_STORE, 
		OUTPUT_FORMAT,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	);

	AddSubpass({}, { 0 }, -1,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

	CreateRenderPass();

	//DescriptorSet
	manager->CreateDescripotrSetLayout({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC }, _ubDescriptorSetLayout);
	manager->CreateDescripotrSetLayout({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER }, _texDescriptorSetLayout, VK_SHADER_STAGE_FRAGMENT_BIT);
	_vertexBuffer.reset(new Buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
	manager->CreatePipelineLayout(
		{
			_ubDescriptorSetLayout,
			_texDescriptorSetLayout,
		}
	, _pipelineLayout);

	//Set Pass Name
	_passName = "HDRI to CubeMap Pass";

	for (int i = 0; i < 6; i++)
	{
		ub_descriptorSet[i].reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, _ubDescriptorSetLayout, BufferSizeRange));
		tex_descriptorSet[i].reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, _texDescriptorSetLayout, 0, VK_SHADER_STAGE_FRAGMENT_BIT));
		_outputFace[i] = 
			Texture2D::CreateTexture2D(
				_cubeMapFaceSize, _cubeMapFaceSize,
				OUTPUT_FORMAT,
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
				HString("Cube Map Face") + HString::FromInt(i),1,1, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			);
	}
}

void HDRI2Cube::PassExecute()
{
	const auto& manager = VulkanManager::GetManager();
	{
		//收集顶点数据一次性使用
		std::vector<ModelPrimitive*> model_prims;
		std::vector<HDRI2CubeVertexData> vertices;
		{
			auto sphere = Model::LoadAsset(HGUID("6146e15a-0632-b197-b6f9-b12fc8a16b05"));
			Model::BuildModelPrimitives(sphere.lock().get(), model_prims, _renderer);
			auto vertexLayout = model_prims[0]->vertexInput.BuildLayout();
			const auto indicesSize = model_prims[0]->vertexInput.vertexIndices.size();
			vertices.reserve(indicesSize);
			for (int i = 0; i < indicesSize; i++)
			{
				uint32_t index = model_prims[0]->vertexInput.vertexIndices[i];
				vertices.push_back(
					{
						model_prims[0]->vertexInput.pos[index],
						model_prims[0]->vertexInput.nor[index]
					}
				);
			}
		}

		for (int f = 0; f < 6; f++)
		{
			manager->DeviceWaitIdle();
			VkCommandBuffer cmdBuf;
			manager->AllocateCommandBuffer(manager->GetCommandPool(), cmdBuf);
			manager->BeginCommandBuffer(cmdBuf);
			//Transition
			if (_outputFace[f]->GetLayout() == VK_IMAGE_LAYOUT_UNDEFINED)
			{
				_outputFace[f]->Transition(cmdBuf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			}

			manager->DestroyFrameBuffers(_framebuffers);
			_framebuffers.resize(manager->GetSwapchainBufferCount());
			for (int i = 0; i < _framebuffers.size(); i++)
			{
				VulkanManager::GetManager()->CreateFrameBuffer(_cubeMapFaceSize, _cubeMapFaceSize, _renderPass, { _outputFace[f]->GetTextureView() }, _framebuffers[i]);
			}
			VulkanManager::GetManager()->CmdSetViewport(cmdBuf, { {_cubeMapFaceSize , _cubeMapFaceSize} });
			VulkanManager::GetManager()->BeginRenderPass(cmdBuf, GetFrameBuffer(), _renderPass, { _cubeMapFaceSize , _cubeMapFaceSize }, _attachmentDescs, { 0,0,0,0 });

			//Begin...
			// 
			//设置管线
			//Create GUIShader Pipeline
			auto pipelineObject = PipelineManager::GetGraphicsPipelineMap(_pipelineIndex);
			if (pipelineObject == nullptr)
			{
				_pipelineIndex = CreatePipeline();
			}
			//收集顶点数据一次性使用
			{
				//ub
				{
					_uniformBuffer[f] = {};
					_uniformBuffer[f].Model = glm::mat4(
						1.0f, 0.0f, 0.0f, 0.0f,
						0.0f, 1.0f, 0.0f, 0.0f,
						0.0f, 0.0f, 1.0f, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f);
					glm::mat4 proj = PassManager::GetPerspectiveProjectionMatrix(90, _cubeMapFaceSize, _cubeMapFaceSize, 0.01, 10000.0f);
					glm::mat4 view;
					if (f == 0)
						glm::mat4 view = glm::lookAtLH(glm::vec3(0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
					else if (f == 1)
						glm::mat4 view = glm::lookAtLH(glm::vec3(0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
					else if (f == 2)
						glm::mat4 view = glm::lookAtLH(glm::vec3(0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0));
					else if (f == 3)
						glm::mat4 view = glm::lookAtLH(glm::vec3(0), glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0));
					else if (f == 4)
						glm::mat4 view = glm::lookAtLH(glm::vec3(0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
					else if (f == 5)
						glm::mat4 view = glm::lookAtLH(glm::vec3(0), glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));

					_uniformBuffer[f].ViewPro = proj * view;
					_uniformBuffer[f].CamPos = glm::vec3(0, 0, 0);
					_uniformBuffer[f].XDegree = 0;
					_uniformBuffer[f].ZDegree = 0;
				}
				//textures
				tex_descriptorSet[f]->UpdateTextureDescriptorSet({ _hdriTexture.get() }, { Texture2D::GetSampler(TextureSampler::TextureSampler_Linear_Wrap) });
				uint32_t ubSize = (uint32_t)manager->GetMinUboAlignmentSize(sizeof(HDRI2CubeUniformBuffer));
				ub_descriptorSet[f]->ResizeDescriptorBuffer(ubSize);
				ub_descriptorSet[f]->UpdateDescriptorSet(ubSize);
				ub_descriptorSet[f]->BufferMapping(&_uniformBuffer[f], 0, ubSize);
			}

			//vertex buffer
			_vertexBuffer->BufferMapping(vertices.data(), 0, sizeof(HDRI2CubeVertexData) * vertices.size());
			VkDeviceSize vbOffset = 0;
			VkBuffer verBuf[] = { _vertexBuffer->GetBuffer() };
			vkCmdBindVertexBuffers(cmdBuf, 0, 1, verBuf, &vbOffset);

			VkPipeline pipeline = VK_NULL_HANDLE;
			{
				//VkPipeline currentPipeline = _guiPipelines[i.second.PipelineTag];
				VkPipeline currentPipeline = PipelineManager::GetGraphicsPipelineMap(_pipelineIndex)->pipeline;
				if (currentPipeline != pipeline)
				{
					pipeline = currentPipeline;
					manager->CmdCmdBindPipeline(cmdBuf, pipeline);
				}
				//textures
				vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 1, 1, &tex_descriptorSet[f]->GetDescriptorSet(), 0, 0);
				//vkCmdSetScissor(cmdBuf, 0, 1, &i.second.viewport);
				uint32_t ubOffset = 0;
				//uniform buffers
				vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &ub_descriptorSet[f]->GetDescriptorSet(), 1, &ubOffset);
				//draw primitive
				vkCmdDraw(cmdBuf, 6, 1, 0, 0);
			}
			//End...
			VulkanManager::GetManager()->EndRenderPass(cmdBuf);


			if (_outputFace[f]->GetLayout() == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			{
				_outputFace[f]->Transition(cmdBuf, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
			}

			manager->EndCommandBuffer(cmdBuf);
			manager->SubmitQueueImmediate({ cmdBuf });
			vkQueueWaitIdle(manager->GetGraphicsQueue());
			manager->FreeCommandBuffer(manager->GetCommandPool(), cmdBuf);
		}
	}

	for (int f = 0; f < 6; f++)
	{
		std::shared_ptr<Buffer> buffer;
		buffer.reset(new Buffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT, _outputFace[f]->GetTextureMemorySize()));
		_outputFace[f]->CopyBufferToTextureImmediate(buffer.get());
		buffer->MapMemory();
		Texture2D::OutputImage((HString("D:/sss") + HString::FromInt(f) + ".dds").c_str(), _cubeMapFaceSize, _cubeMapFaceSize, nvtt::Format_BC6S, buffer->GetBufferMemory());
		buffer->UnMapMemory();
	}
}

PipelineIndex HDRI2Cube::CreatePipeline()
{
	VkPipeline pipeline = VK_NULL_HANDLE;
	auto vsCache = Shader::_vsShader["HDRI2Cube@0"];
	auto psCache = Shader::_psShader["HDRI2Cube@0"];
	VertexInputLayout vertexInputLayout = {};
	vertexInputLayout.inputLayouts.resize(1);
	vertexInputLayout.inputLayouts[0] = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputLayout.inputSize = sizeof(float) * 3;
	vertexInputLayout.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	VkGraphicsPipelineCreateInfoCache pipelineCreateInfo = {};
	PipelineManager::SetColorBlend(pipelineCreateInfo, true,
		StaticBlendState(
			1,
			CW_RGBA,
			BO_ADD, BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA,//color
			BO_ADD, BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA //alpha
		));
	Rasterizer raster = {};
	raster.cullMode = CullMode::ECM_NONE;
	PipelineManager::SetRenderRasterizer(pipelineCreateInfo, raster);
	//PipelineManager::SetRenderDepthStencil(pipelineCreateInfo);
	PipelineManager::SetVertexInput(pipelineCreateInfo, vertexInputLayout);
	PipelineManager::SetVertexShaderAndPixelShader(pipelineCreateInfo, vsCache.get(), psCache.get());
	PipelineManager::SetPipelineLayout(pipelineCreateInfo, _pipelineLayout);
	PipelineManager::BuildGraphicsPipelineState(pipelineCreateInfo, _renderPass, 0, pipeline);
	return PipelineManager::AddPipelineObject(vsCache, psCache, pipeline, _pipelineLayout);
}
