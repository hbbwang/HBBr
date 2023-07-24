#pragma once
#include "glm/glm.hpp"
#include "Pipeline.h"
namespace VertexFactory
{
	struct VertexInputBase
	{
		glm::vec3 pos;
		glm::vec3 nor;
		glm::vec3 tan;
		glm::vec4 col;
		glm::vec4 uv01;
		glm::vec4 uv23;
		static VertexInputLayout BuildLayout()
		{
			VertexInputLayout result = {};
			result.inputLayouts = {
				VK_FORMAT_R32G32B32_SFLOAT,
				VK_FORMAT_R32G32B32_SFLOAT,
				VK_FORMAT_R32G32B32_SFLOAT,
				VK_FORMAT_R32G32B32A32_SFLOAT,
				VK_FORMAT_R32G32B32A32_SFLOAT,
				VK_FORMAT_R32G32B32A32_SFLOAT
			};
			result.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			result.inputSize = sizeof(VertexInputBase);
			return result;
		}
	};

};