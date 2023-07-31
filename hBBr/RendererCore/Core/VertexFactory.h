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

		std::vector<float> GetData()
		{
			std::vector<float> result(
				3 * pos.size() +
				3 * nor.size() +
				3 * tan.size() +
				4 * col.size() +
				4 * uv01.size() +
				4 * uv23.size()
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
			result.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return result;
		}
	};

	struct ScreenTriangleVertexInput : public VertexInput
	{
		ScreenTriangleVertexInput() 
		{
			if (vertexInputData.size() <= 0)
			{
				pos =
				{
					glm::vec3(-1,1,0) ,
					glm::vec3(0,-1,0),
					glm::vec3(1,1,0)
				};
				col =
				{
					glm::vec4(1,0,0,1),
					glm::vec4(0,1,0,1),
					glm::vec4(0,0,1,1)
				};
				vertexInputData = GetData();
				vertexIndices = {0,1,2};
				vertexInputLayout = BuildLayout();
			}
		}
		std::vector<float> vertexInputData;
		std::vector<uint32_t> vertexIndices;
		VertexInputLayout vertexInputLayout;
	};
};