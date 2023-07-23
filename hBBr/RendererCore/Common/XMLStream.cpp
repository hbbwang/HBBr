#include "XMLStream.h"

bool XMLStream::LoadXML(const wchar_t* path, pugi::xml_document& doc)
{
	auto result = doc.load_file(path, pugi::parse_default, pugi::encoding_utf8);
	if (result.status != pugi::status_ok)
	{
		return false;
	}
	return true;
}

bool XMLStream::LoadXMLNode(pugi::xml_document& doc, const wchar_t* nodeName, pugi::xml_node& node)
{
	HString nodeStr = nodeName;
	auto nodeArray = nodeStr.Split("/");
	if (!doc.empty() && doc != nullptr)
	{
		for (int i = 0; i < nodeArray.size(); i++)
		{
			if (i == 0 && nodeArray[0] == "root")
			{
				node = doc.first_child();
				continue;
			}
			else if (i == 0)
				node = doc.first_child();
			node = node.child(nodeArray[i].c_wstr());
		}
	}
	else
		return false;
	if (node.empty() || node == nullptr)
		return false;
	else
		return true;
}

bool XMLStream::LoadXMLAttributeString(pugi::xml_node& node, const wchar_t* attributeName, HString& attri)
{
	if (node.empty() || node == nullptr)
		return false;
	auto attr = node.attribute(attributeName);
	if (attr != nullptr && !attr.empty())
	{
		attri = node.attribute(attributeName).as_string();
		return true;
	}
	return false;
}

bool XMLStream::LoadXMLAttributeInt(pugi::xml_node& node, const wchar_t* attributeName, int& attri)
{
	if (node.empty() || node == nullptr)
		return false;
	auto attr = node.attribute(attributeName);
	if (attr != nullptr && !attr.empty())
	{
		attri = node.attribute(attributeName).as_int();
		return true;
	}
	return false;
}

bool XMLStream::LoadXMLAttributeBool(pugi::xml_node& node, const wchar_t* attributeName, bool& attri)
{
	if (node.empty() || node == nullptr)
		return false;
	auto attr = node.attribute(attributeName);
	if (attr != nullptr && !attr.empty())
	{
		attri = node.attribute(attributeName).as_bool();
		return true;
	}
	return false;
}

bool XMLStream::LoadXMLAttributeFloat(pugi::xml_node& node, const wchar_t* attributeName, float& attri)
{
	if (node.empty() || node == nullptr)
		return false;
	auto attr = node.attribute(attributeName);
	if (attr != nullptr && !attr.empty())
	{
		attri = node.attribute(attributeName).as_float();
		return true;
	}
	return false;
}
