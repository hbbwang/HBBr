#pragma once
//渲染器配置
#include <memory>
#include <string>
#include "pugixml.hpp"
class XMLStream
{
public:

	/* 读取xml文档 */
	static bool LoadXML(const char* path , pugi::xml_document& doc);

	/* 读取xml节点,可以使用路径形式,例如:root/aa/bb */
	static bool LoadXMLNode(const char* nodeName, pugi::xml_node& node);

};