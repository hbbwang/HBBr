#include "GUIPass.h"
#include "VulkanRenderer.h"
#include "Texture.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_GLFW_VULKAN_IMPLEMENTATION
#define NK_IMPLEMENTATION

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_INDEX_BUFFER 128 * 1024

#include "nuklear.h"

nk_context ctx;
nk_font_atlas atlas;
nk_buffer cmds;
nk_draw_null_texture null;

struct GUIVertexData
{
	float position[2];
	float uv[2];
	nk_byte col[4];
};

/*
	Nuklear (Game GUI) pass
*/

GUIPass::~GUIPass()
{
	const auto& manager = VulkanManager::GetManager();
	_descriptorSet.reset();
	nk_buffer_free(&cmds);
	manager->DestroyPipeline(_pipeline);
	manager->DestroyPipelineLayout(_pipelineLayout);
	for (auto i : _guiPipelines)
	{
		manager->DestroyPipeline(i.second);
	}
	_guiPipelines.clear();
	manager->DestroyDescriptorSetLayout(_texDescriptorSetLayout);
}

NK_INTERN void nk_clipbard_paste(nk_handle usr,
	struct nk_text_edit* edit) {
	const char* text = SDL_GetClipboardText();
	if (text)
		nk_textedit_paste(edit, text, nk_strlen(text));
	(void)usr;
}

NK_INTERN void nk_clipbard_copy(nk_handle usr, const char* text,
	int len) {
	char* str = 0;
	(void)usr;
	if (!len)
		return;
	str = (char*)malloc((size_t)len + 1);
	if (!str)
		return;
	memcpy(str, text, (size_t)len);
	str[len] = '\0';
	SDL_SetClipboardText(str);
	free(str);
}

void update_texture_descriptor_set(
	struct GUITexture* texture_descriptor_set,
	VkImageView image_view) {
	texture_descriptor_set->ImageView = image_view;

	VkDescriptorImageInfo descriptor_image_info = {
		Texture::GetSampler(TextureSampler_Linear_Wrap),
		texture_descriptor_set->ImageView,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};

	VkWriteDescriptorSet descriptor_write = {
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	descriptor_write.dstSet = texture_descriptor_set->DescriptorSet;
	descriptor_write.dstBinding = 0;
	descriptor_write.dstArrayElement = 0;
	descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptor_write.descriptorCount = 1;
	descriptor_write.pImageInfo = &descriptor_image_info;

	vkUpdateDescriptorSets(VulkanManager::GetManager()->GetDevice(), 1, &descriptor_write, 0,
		VK_NULL_HANDLE);
}

void GUIPass::create_texture_descriptor_sets() {

	VkDescriptorSetLayout descriptor_set_layouts[128];
	VkDescriptorSet descriptor_sets[128];
	_guiTextures.resize(128);

	uint32_t i;
	for (i = 0; i < 128; i++) {
		descriptor_sets[i] = _guiTextures[i].DescriptorSet;
		descriptor_set_layouts[i] = _texDescriptorSetLayout;
	}

	VkDescriptorSetAllocateInfo allocate_info = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocate_info.descriptorPool = VulkanManager::GetManager()->GetDescriptorPool();
	allocate_info.descriptorSetCount = 128;
	allocate_info.pSetLayouts = descriptor_set_layouts;

	VkResult result = vkAllocateDescriptorSets(VulkanManager::GetManager()->GetDevice(),
		&allocate_info, descriptor_sets);

	for (i = 0; i < 128; i++) {
		_guiTextures[i].DescriptorSet = descriptor_sets[i];
	}
}

void GUIPass::nk_device_upload_atlas(VkQueue graphics_queue, const void* image, int width, int height) 
{
	auto manager = VulkanManager::GetManager();
	manager->CreateImage(width,height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT , _fontImage);
	auto size = manager->CreateImageMemory(_fontImage, _fontMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	struct {
		VkDeviceMemory memory;
		VkBuffer buffer;
	} staging_buffer;

	manager->CreateImageView(_fontImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, _fontImageView);

	manager->CreateBufferAndAllocateMemory(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer.buffer, staging_buffer.memory);

	uint8_t* data = 0;
	vkMapMemory(manager->GetDevice(), staging_buffer.memory, 0,
		size, 0, (void**)&data);
	memcpy(data, image, width * height * 4);
	vkUnmapMemory(manager->GetDevice(), staging_buffer.memory);


	// use the same command buffer as for render as we are regenerating the buffer
	// during render anyway
	VkCommandBufferBeginInfo begin_info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };

	VkCommandBuffer buf;
	manager->AllocateCommandBuffer(manager->GetCommandPool(), buf);
	manager->BeginCommandBuffer(buf, 0);

	{
		VkImageMemoryBarrier image_memory_barrier = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		image_memory_barrier.image = _fontImage;
		image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		image_memory_barrier.subresourceRange =VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1,},
		image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		vkCmdPipelineBarrier(buf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, VK_NULL_HANDLE, 0,
			VK_NULL_HANDLE, 1, &image_memory_barrier);

		VkBufferImageCopy buffer_copy_region = {
			0,
			0,
			0,
			{
				VK_IMAGE_ASPECT_COLOR_BIT,
				0,
				0,
				1,
			},
			{0, 0, 0},
			{
				(uint32_t)width,
				(uint32_t)height,
				1,
			},
		};

		vkCmdCopyBufferToImage(
			buf, staging_buffer.buffer, _fontImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_copy_region);

		VkImageMemoryBarrier image_shader_memory_barrier = {
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		image_shader_memory_barrier.image = _fontImage;
		image_shader_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		image_shader_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		image_shader_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		image_shader_memory_barrier.newLayout =
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		image_shader_memory_barrier.subresourceRange =VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1,},
		image_shader_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		image_shader_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,

		vkCmdPipelineBarrier(buf, VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
		VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1,
		&image_shader_memory_barrier);
	}
	manager->EndCommandBuffer(buf);
	manager->SubmitQueueImmediate({ buf });
	vkQueueWaitIdle(VulkanManager::GetManager()->GetGraphicsQueue());
	manager->FreeCommandBuffers(manager->GetCommandPool(), { buf });

	vkFreeMemory(manager->GetDevice(), staging_buffer.memory, VK_NULL_HANDLE);
	vkDestroyBuffer(manager->GetDevice(), staging_buffer.buffer,VK_NULL_HANDLE);

	// write_font_descriptor_set(adapter);
}

void GUIPass::nk_sdl_font_stash_begin(struct nk_font_atlas** atlas1)
{
	nk_font_atlas_init_default(&atlas);
	nk_font_atlas_begin(&atlas);
	*atlas1 = &atlas;
}

void GUIPass::nk_sdl_font_stash_end(void)
{
	HString fontPath = FileSystem::GetResourceAbsPath() + "Font/simhei.ttf";
	fontPath.CorrectionPath();
	struct nk_font* simhei = nk_font_atlas_add_from_file(&atlas, fontPath.c_str(), 14, 0);
	const void* image; int w, h;
	image = nk_font_atlas_bake(&atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
	nk_device_upload_atlas(VulkanManager::GetManager()->GetGraphicsQueue() ,image, w, h);
	nk_font_atlas_end(&atlas, nk_handle_ptr(_fontImageView),
		&null);
	if (simhei)
		nk_style_set_font(&ctx, &simhei->handle);
}


void GUIPass::PassInit()
{
	const auto& manager = VulkanManager::GetManager();
	AddAttachment(VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, _renderer->GetSurfaceFormat().format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	AddSubpass({}, { 0 }, -1 , 
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
	CreateRenderPass();
	//DescriptorSet
	manager->CreateDescripotrSetLayout({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER }, _texDescriptorSetLayout, VK_SHADER_STAGE_FRAGMENT_BIT);
	_descriptorSet.reset(new DescriptorSet(_renderer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, sizeof(GUIUniformBuffer)));

	{
		manager->CreateBufferAndAllocateMemory(MAX_VERTEX_BUFFER,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |VK_MEMORY_PROPERTY_HOST_COHERENT_BIT , _buffer.vb , _buffer.vbm);
		manager->CreateBufferAndAllocateMemory(MAX_INDEX_BUFFER, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _buffer.ib, _buffer.ibm);
	}

	manager->CreatePipelineLayout(
		{
			_descriptorSet->GetDescriptorSetLayout() ,
			_texDescriptorSetLayout,
		}
	, _pipelineLayout);
	_descriptorSet->UpdateDescriptorSetAll(sizeof(GUIUniformBuffer));
	//Set Pass Name
	_passName = "GUI Render Pass";
	//CraetePipeline..
	auto vsCache = Shader::_vsShader["GUIShader"];
	auto psCache = Shader::_psShader["GUIShader"];
	VertexInputLayout vertexInputLayout = {};
	vertexInputLayout.inputLayouts.resize(3);
	vertexInputLayout.inputLayouts[0] = VK_FORMAT_R32G32_SFLOAT;//Pos
	vertexInputLayout.inputLayouts[1] = VK_FORMAT_R32G32_SFLOAT;//UV
	vertexInputLayout.inputLayouts[2] = VK_FORMAT_R32G32B32A32_SFLOAT;//Color
	vertexInputLayout.inputSize = sizeof(float) * 8;
	vertexInputLayout.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	VkGraphicsPipelineCreateInfoCache pipelineCreateInfo = {};
	PipelineManager::SetColorBlend(pipelineCreateInfo, true,
		StaticBlendState(
			1,
			CW_RGBA,
			BO_ADD, BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA,//color
			BO_ADD, BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA //alpha
		));
	PipelineManager::SetRenderRasterizer(pipelineCreateInfo);
	//PipelineManager::SetRenderDepthStencil(pipelineCreateInfo);
	PipelineManager::SetVertexInput(pipelineCreateInfo, vertexInputLayout);
	PipelineManager::SetVertexShaderAndPixelShader(pipelineCreateInfo, vsCache, psCache);
	PipelineManager::SetPipelineLayout(pipelineCreateInfo, _pipelineLayout);
	PipelineManager::BuildGraphicsPipelineState(pipelineCreateInfo, _renderPass, 0, _pipeline);
	

	nk_init_default(&ctx, 0);
	ctx.clip.copy = nk_clipbard_copy;
	ctx.clip.paste = nk_clipbard_paste;
	ctx.clip.userdata = nk_handle_ptr(0);

	nk_buffer_init_default(&cmds);

	create_texture_descriptor_sets();

}

void GUIPass::PassUpdate()
{

	const auto& manager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();
	COMMAND_MAKER(cmdBuf, BasePass, _passName.c_str(), glm::vec4(0.3, 1.0, 0.1, 0.2));
	//Update FrameBuffer
	ResetFrameBuffer(_renderer->GetSurfaceSize(), {});
	SetViewport(_currentFrameBufferSize);
	BeginRenderPass({ 0,0,0,0 });

	if (!bSetFont)
	{
		bSetFont = true;
		struct nk_font_atlas* atlas1;
		nk_sdl_font_stash_begin(&atlas1);
		/*struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14, 0);*/
		/*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 16, 0);*/
		/*struct nk_font *future = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13, 0);*/
		/*struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12, 0);*/
		/*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10, 0);*/
		/*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13, 0);*/
		nk_sdl_font_stash_end();
		/* style.c */
#ifdef INCLUDE_STYLE
/* ease regression testing during Nuklear release process; not needed for anything else */
#ifdef STYLE_WHITE
		set_style(ctx, THEME_WHITE);
#elif defined(STYLE_RED)
		set_style(ctx, THEME_RED);
#elif defined(STYLE_BLUE)
		set_style(ctx, THEME_BLUE);
#elif defined(STYLE_DARK)
		set_style(ctx, THEME_DARK);
#endif
#endif
	}

	if (nk_begin(&ctx, "My Window", nk_rect(0, 0, 200, 200), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE)) {
		nk_layout_row_static(&ctx, 30, 80, 1);
		if (nk_button_label(&ctx, "Button")) {
			printf("Button pressed\n");
		}
	}
	nk_end(&ctx);

	float matrix[16] =
	{
		2.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -2.0f, 0.0f, 0.0f,
		0.0f, 0.0f, -1.0f,0.0f,
		-1.0f,1.0f, 0.0f, 1.0f 
	};
	matrix[0] /= _currentFrameBufferSize.width;
	matrix[5] /= _currentFrameBufferSize.height;
	GUIUniformBuffer projection;
	memcpy(&projection.Projection, matrix,sizeof(matrix));
	_descriptorSet->BufferMapping(&projection, 0, sizeof(projection), 0);

	uint32_t dynamic_offset[1] = { (uint32_t)0 };
	vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSet->GetDescriptorSet(), 1, dynamic_offset);
	manager->CmdCmdBindPipeline(cmdBuf, _pipeline);

	const struct nk_draw_command* cmd;
	void* vertices;
	void* elements;
	vkMapMemory(manager->GetDevice(), _buffer.vbm, 0, MAX_VERTEX_BUFFER, 0, &vertices);
	vkMapMemory(manager->GetDevice(), _buffer.ibm, 0, MAX_INDEX_BUFFER, 0, &elements);
	{
		/* fill convert configuration */
		struct nk_convert_config config;
		static const struct nk_draw_vertex_layout_element vertex_layout[] = {
			{NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(GUIVertexData, position)},
			{NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(GUIVertexData, uv)},
			{NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(GUIVertexData, col)},
			{NK_VERTEX_LAYOUT_END}
		};
		memset(&config, 0, sizeof(config));
		config.vertex_layout = vertex_layout;
		config.vertex_size = sizeof(GUIVertexData);
		config.vertex_alignment = NK_ALIGNOF(GUIVertexData);
		config.circle_segment_count = 22;
		config.curve_segment_count = 22;
		config.arc_segment_count = 22;
		config.global_alpha = 1.0f;		
		config.shape_AA = nk_anti_aliasing::NK_ANTI_ALIASING_ON;
		config.line_AA = nk_anti_aliasing::NK_ANTI_ALIASING_ON;
		config.tex_null = null;

		/* setup buffers to load vertices and elements */
		struct nk_buffer vbuf, ebuf;
		nk_buffer_init_fixed(&vbuf, vertices, (size_t)MAX_VERTEX_BUFFER);
		nk_buffer_init_fixed(&ebuf, elements, (size_t)MAX_INDEX_BUFFER);
		nk_convert(&ctx, &cmds, &vbuf, &ebuf, &config);
	}
	vkUnmapMemory(manager->GetDevice(), _buffer.vbm);
	vkUnmapMemory(manager->GetDevice(), _buffer.ibm);

	//vertex buffer
	VkDeviceSize doffset = 0;
	vkCmdBindVertexBuffers(cmdBuf, 0, 1, &_buffer.vb, &doffset);
	vkCmdBindIndexBuffer(cmdBuf, _buffer.ib, 0, VK_INDEX_TYPE_UINT16);

	VkImageView current_texture = VK_NULL_HANDLE;
	uint32_t index_offset = 0;
	nk_draw_foreach(cmd, &ctx, &cmds) {
		if (!cmd->texture.ptr) {
			continue;
		}
		if (cmd->texture.ptr && cmd->texture.ptr != current_texture) {
			int found = 0;
			uint32_t i;
			for (i = 0; i < texture_descriptor_sets_len; i++) {
				if (_guiTextures[i].ImageView ==
					cmd->texture.ptr) {
					found = 1;
					break;
				}
			}
			if (!found) {
				update_texture_descriptor_set(
					&_guiTextures[i],
					(VkImageView)cmd->texture.ptr);
				texture_descriptor_sets_len++;
			}
			vkCmdBindDescriptorSets(
				cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
				_pipelineLayout, 1, 1,
				&_guiTextures[i].DescriptorSet, 0,
				VK_NULL_HANDLE);
		}
		if (!cmd->elem_count)
			continue;
		VkRect2D scissor = {
			{(int32_t)(SDL_max(cmd->clip_rect.x , 0.f)),(int32_t)(SDL_max(cmd->clip_rect.y , 0.f)),},
			{(uint32_t)(cmd->clip_rect.w ),(uint32_t)(cmd->clip_rect.h ),},
		};
		vkCmdSetScissor(cmdBuf, 0, 1, &scissor);
		vkCmdDrawIndexed(cmdBuf, cmd->elem_count, 1, index_offset, 0, 0);
		index_offset += cmd->elem_count;
	}
	nk_clear(&ctx);
	nk_buffer_clear(&cmds);
	//End...
	EndRenderPass();
}

void GUIPass::PassReset()
{

}