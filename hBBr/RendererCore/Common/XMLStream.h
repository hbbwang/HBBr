#pragma once
//渲染器配置
//使用的xml作为配置文件，文件格式必须要有一个root节点，
//并且子节点都应该在root里
#include <memory>
#include <string>
#include "pugixml/pugixml.hpp"
#include "HString.h"
class XMLStream
{
public:

	/* 读取xml文档 */
	static bool LoadXML(const wchar_t* path, pugi::xml_document& doc);

	/* 保存xml文档UTF-8格式 */
	static bool SaveXML(HString path, pugi::xml_document& doc);

	/* 读取xml节点,必须使用完整的路径形式,例如:root/aa/bb */
	static bool LoadXMLNode(pugi::xml_document& doc, const wchar_t* nodeName, pugi::xml_node& node);

	/* 读取xml节点的String属性 */
	static bool LoadXMLAttributeString(pugi::xml_node& node, const wchar_t* attributeName, HString& attri);

	/* 读取xml节点的Int32属性 */
	static bool LoadXMLAttributeInt(pugi::xml_node& node, const wchar_t* attributeName, int& attri);

	/* 读取xml节点的uint属性 */
	static bool LoadXMLAttributeUInt(pugi::xml_node& node, const wchar_t* attributeName, uint32_t& attri);

	/* 读取xml节点的Size_t属性 */
	static bool LoadXMLAttributeUint64(pugi::xml_node& node, const wchar_t* attributeName, uint64_t& attri);

	/* 读取xml节点的Size_t属性 */
	static bool LoadXMLAttributeInt64(pugi::xml_node& node, const wchar_t* attributeName, int64_t& attri);

	/* 读取xml节点的Bool属性 */
	static bool LoadXMLAttributeBool(pugi::xml_node& node, const wchar_t* attributeName, bool& attri);

	/* 读取xml节点的Float属性 */
	static bool LoadXMLAttributeFloat(pugi::xml_node& node, const wchar_t* attributeName, float& attri);

	static pugi::xml_node GetXMLNode(pugi::xml_node& node, const wchar_t* name)
	{
		if (node)
		{
			pugi::xml_node result = node.child(name);
			if (!result)
			{
				result = node.append_child(name);
			}
			return result;
		}
		return pugi::xml_node();
	}

	template<class T>
	static void SetXMLAttribute(pugi::xml_node& node, const wchar_t* attributeName, T value)
	{
		if (node)
		{
			pugi::xml_attribute attr = node.attribute(attributeName);
			if (!attr)
			{
				attr = node.append_attribute(attributeName);
			}
			attr.set_value(value);
		}
	}

	static pugi::xml_node CreateXMLNode(pugi::xml_node& node, const wchar_t* name , bool bOverride = true)
	{
		if (node)
		{
			if (bOverride)
			{
				auto newChild = node.child(name);
				if (!newChild)
				{
					newChild = node.append_child(name);
				}
				return newChild;
			}
			else
			{
				return node.append_child(name);
			}
		}
		return pugi::xml_node();
	}

	static bool CreateXMLFile(HString path , pugi::xml_document& doc);

};