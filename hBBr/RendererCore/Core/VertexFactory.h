#pragma once
#include "glm/glm.hpp"
#include "Pipeline.h"

namespace VertexFactory
{
	struct VertexInput
	{
		std::vector<glm::vec3> pos;
		std::vector<glm::vec3> nor;
		std::vector<glm::vec3> tan;
		std::vector<glm::vec4> col;
		std::vector<glm::vec4> uv01;
		std::vector<glm::vec4> uv23;
		std::vector<glm::vec4> uv45;
		std::vector<uint32_t> vertexIndices;

		std::vector<float> GetData()
		{
			std::vector<float> result(
				3 * pos.size() +
				3 * nor.size() +
				3 * tan.size() +
				4 * col.size() +
				4 * uv01.size() +
				4 * uv23.size() +
				4 * uv45.size()
			);
			uint32_t dataIndex = 0;
			for (int i = 0; i < result.size(); i++ )
			{
				if (pos.size() > dataIndex)
				{
					result[i] = pos[dataIndex].x;
					result[++i] = pos[dataIndex].y;
					result[++i] = pos[dataIndex].z;
				}
				//
				if (nor.size() > dataIndex)
				{
					result[++i] = nor[dataIndex].x;
					result[++i] = nor[dataIndex].y;
					result[++i] = nor[dataIndex].z;
				}
				//
				if (tan.size() > dataIndex)
				{
					result[++i] = tan[dataIndex].x;
					result[++i] = tan[dataIndex].y;
					result[++i] = tan[dataIndex].z;
				}
				//
				if (col.size() > dataIndex)
				{
					result[++i] = col[dataIndex].x;
					result[++i] = col[dataIndex].y;
					result[++i] = col[dataIndex].z;
					result[++i] = col[dataIndex].w;
				}
				//
				if (uv01.size() > dataIndex)
				{
					result[++i] = uv01[dataIndex].x;
					result[++i] = uv01[dataIndex].y;
					result[++i] = uv01[dataIndex].z;
					result[++i] = uv01[dataIndex].w;
				}
				//
				if (uv23.size() > dataIndex)
				{
					result[++i] = uv23[dataIndex].x;
					result[++i] = uv23[dataIndex].y;
					result[++i] = uv23[dataIndex].z;
					result[++i] = uv23[dataIndex].w;
				}
				//
				if (uv45.size() > dataIndex)
				{
					result[++i] = uv45[dataIndex].x;
					result[++i] = uv45[dataIndex].y;
					result[++i] = uv45[dataIndex].z;
					result[++i] = uv45[dataIndex].w;
				}
				dataIndex++;
			}
			return result;
		}

		std::vector<float> GetData(uint8_t inputType[7])
		{
			int mul[7]{
				inputType[0] > 0 ? 1 : 0,
				inputType[1] > 0 ? 1 : 0,
				inputType[2] > 0 ? 1 : 0,
				inputType[3] > 0 ? 1 : 0,
				inputType[4] > 0 ? 1 : 0, 
				inputType[5] > 0 ? 1 : 0,
				inputType[6] > 0 ? 1 : 0
			};

			if (nor.size() <= 0)
				nor.resize(pos.size());
			if (tan.size() <= 0)
				tan.resize(pos.size());
			if (col.size() <= 0)
				col.resize(pos.size());
			if (uv01.size() <= 0)
				uv01.resize(pos.size());
			if (uv23.size() <= 0)
				uv23.resize(pos.size());
			if (uv45.size() <= 0)
				uv45.resize(pos.size());

			std::vector<float> result(
				3 * pos.size() * mul[0] +
				3 * nor.size() * mul[1] +
				3 * tan.size() * mul[2] +
				4 * col.size() * mul[3] +
				4 * uv01.size() * mul[4] +
				4 * uv23.size() * mul[5] +
				4 * uv45.size() * mul[6]
			);
			uint32_t dataIndex = 0;
			const size_t count = result.size();
			for (size_t i = 0; i < count; i++)
			{
				if (pos.size() > dataIndex && mul[0] > 0 )
				{
					result[i] = pos[dataIndex].x;
					result[++i] = pos[dataIndex].y;
					result[++i] = pos[dataIndex].z;
				}
				//
				if (nor.size() > dataIndex && mul[1] > 0)
				{
					result[++i] = nor[dataIndex].x;
					result[++i] = nor[dataIndex].y;
					result[++i] = nor[dataIndex].z;
				}
				//
				if (tan.size() > dataIndex && mul[2] > 0)
				{
					result[++i] = tan[dataIndex].x;
					result[++i] = tan[dataIndex].y;
					result[++i] = tan[dataIndex].z;
				}
				//
				if (col.size() > dataIndex && mul[3] > 0)
				{
					result[++i] = col[dataIndex].x;
					result[++i] = col[dataIndex].y;
					result[++i] = col[dataIndex].z;
					result[++i] = col[dataIndex].w;
				}
				//
				if (uv01.size() > dataIndex && mul[4] > 0)
				{
					result[++i] = uv01[dataIndex].x;
					result[++i] = uv01[dataIndex].y;
					result[++i] = uv01[dataIndex].z;
					result[++i] = uv01[dataIndex].w;
				}
				//
				if (uv23.size() > dataIndex && mul[5] > 0)
				{
					result[++i] = uv23[dataIndex].x;
					result[++i] = uv23[dataIndex].y;
					result[++i] = uv23[dataIndex].z;
					result[++i] = uv23[dataIndex].w;
				}
				//
				if (uv45.size() > dataIndex && mul[6] > 0)
				{
					result[++i] = uv45[dataIndex].x;
					result[++i] = uv45[dataIndex].y;
					result[++i] = uv45[dataIndex].z;
					result[++i] = uv45[dataIndex].w;
				}
				dataIndex++;
			}
			return result;
		}


		VertexInputLayout BuildLayout()
		{
			VertexInputLayout result = {};
			result.inputSize = 0;
			if (pos.size() > 0)
			{
				result.inputLayouts.push_back(VK_FORMAT_R32G32B32_SFLOAT);
				result.inputSize += sizeof(glm::vec3);
			}
			if (nor.size() > 0)
			{
				result.inputLayouts.push_back(VK_FORMAT_R32G32B32_SFLOAT);
				result.inputSize += sizeof(glm::vec3);
			}
			if (tan.size() > 0)
			{
				result.inputLayouts.push_back(VK_FORMAT_R32G32B32_SFLOAT);
				result.inputSize += sizeof(glm::vec3);
			}
			if (col.size() > 0)
			{
				result.inputLayouts.push_back(VK_FORMAT_R32G32B32A32_SFLOAT);
				result.inputSize += sizeof(glm::vec4);
			}
			if (uv01.size() > 0)
			{
				result.inputLayouts.push_back(VK_FORMAT_R32G32B32A32_SFLOAT);
				result.inputSize += sizeof(glm::vec4);
			}
			if (uv23.size() > 0)
			{
				result.inputLayouts.push_back(VK_FORMAT_R32G32B32A32_SFLOAT);
				result.inputSize += sizeof(glm::vec4);
			}
			if (uv45.size() > 0)
			{
				result.inputLayouts.push_back(VK_FORMAT_R32G32B32A32_SFLOAT);
				result.inputSize += sizeof(glm::vec4);
			}
			result.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return result;
		}

		static VertexInputLayout BuildLayout(uint8_t inputType[7])
		{
			VertexInputLayout result = {};
			result.inputSize = 0;
			for (int i = 0; i < 7; i++)
			{
				if (inputType[i] == 0)
					continue;
				else if (inputType[i] == 1)
				{
					result.inputLayouts.push_back(VK_FORMAT_R32_SFLOAT);
					result.inputSize += sizeof(float);
				}
				else if (inputType[i] == 2)
				{
					result.inputLayouts.push_back(VK_FORMAT_R32G32_SFLOAT);
					result.inputSize += sizeof(glm::vec2);
				}
				else if (inputType[i] == 3)
				{
					result.inputLayouts.push_back(VK_FORMAT_R32G32B32_SFLOAT);
					result.inputSize += sizeof(glm::vec3);
				}
				else if (inputType[i] == 4)
				{
					result.inputLayouts.push_back(VK_FORMAT_R32G32B32A32_SFLOAT);
					result.inputSize += sizeof(glm::vec4);
				}
			}
			result.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return result;
		}
	};

};