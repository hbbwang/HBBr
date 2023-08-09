#pragma once
//渲染器配置
//使用的xml作为配置文件，文件格式必须要有一个root节点，
//并且子节点都应该在root里
#include <memory>
#include <string>
#include "pugixml.hpp"
#include "HString.h"
class XMLStream
{
public:

	/* 读取xml文档 */
	static bool LoadXML(const wchar_t* path, pugi::xml_document& doc);

	/* 读取xml节点,必须使用完整的路径形式,例如:root/aa/bb */
	static bool LoadXMLNode(pugi::xml_document& doc, const wchar_t* nodeName, pugi::xml_node& node);

	/* 读取xml节点的String属性 */
	static bool LoadXMLAttributeString(pugi::xml_node& node, const wchar_t* attributeName, HString& attri);

	/* 读取xml节点的Int32属性 */
	static bool LoadXMLAttributeInt(pugi::xml_node& node, const wchar_t* attributeName, int& attri);

	/* 读取xml节点的Bool属性 */
	static bool LoadXMLAttributeBool(pugi::xml_node& node, const wchar_t* attributeName, bool& attri);

	/* 读取xml节点的Float属性 */
	static bool LoadXMLAttributeFloat(pugi::xml_node& node, const wchar_t* attributeName, float& attri);

};