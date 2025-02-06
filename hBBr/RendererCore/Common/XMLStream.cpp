#include "XMLStream.h"
#include "FileSystem.h"

bool XMLStream::LoadXML(const char* path, pugi::xml_document& doc)
{
	auto result = doc.load_file(path, pugi::parse_default, pugi::encoding_utf8);
	if (result.status != pugi::status_ok)
	{
		std::string errorMsg = "Load XML Failed:";
		errorMsg += path;
		errorMsg += "\n\t";
		using namespace pugi;
		switch (result.status)
		{
		case status_file_not_found:
			errorMsg +="Xml loading fatal error! File was not found during load_file()";
			break;
		case status_io_error:
			errorMsg += "Xml loading fatal error! Error reading from file/stream";
			break;
		case status_out_of_memory:
			errorMsg += "Xml loading fatal error!  Could not allocate memory";
			break;
		case status_internal_error:
			errorMsg += "Xml loading fatal error! Internal error occurred";
			break;
		case status_unrecognized_tag:
			errorMsg += "Xml loading fatal error! Parser could not determine tag type";
			break;
		case status_bad_pi:
			errorMsg += "Xml loading fatal error! Parsing error occurred while parsing document declaration/processing instruction";
			break;
		case status_bad_comment:
			errorMsg += "Xml loading fatal error! Parsing error occurred while parsing comment";
			break;
		case status_bad_cdata:
			errorMsg += "Xml loading fatal error! Parsing error occurred while parsing CDATA section";
			break;
		case status_bad_doctype:
			errorMsg += "Xml loading fatal error! Parsing error occurred while parsing document type declaration";
			break;
		case status_bad_pcdata:
			errorMsg += "Xml loading fatal error! Parsing error occurred while parsing PCDATA section";
			break;
		case status_bad_start_element:
			errorMsg += "Xml loading fatal error! Parsing error occurred while parsing start element tag";
			break;
		case status_bad_attribute:
			errorMsg += "Xml loading fatal error! Parsing error occurred while parsing element attribute";
			break;
		case status_bad_end_element:
			errorMsg += "Xml loading fatal error! Parsing error occurred while parsing end element tag";
			break;
		case status_end_element_mismatch:
			errorMsg += "Xml loading fatal error! There was a mismatch of start-end tags (closing tag had incorrect name, some tag was not closed or there was an excessive closing tag)";
			break;
		case status_append_invalid_root:
			errorMsg += "Xml loading fatal error! Unable to append nodes since root type is not node_element or node_document (exclusive to xml_node::append_buffer)";
			break;
		case status_no_document_element:
			errorMsg += "Xml loading fatal error! Parsing resulted in a document without element nodes";
			break;
		default:
			break;
		}
		MessageOut(errorMsg, false, false);
		return false;
	}
	return true;
}

bool XMLStream::SaveXML(std::string path, pugi::xml_document& doc)
{
	return CreateXMLDocument(path,doc);
}

bool XMLStream::LoadXMLNode(pugi::xml_document& doc, const char* nodeName, pugi::xml_node& node)
{
	std::string nodeStr = nodeName;
	auto nodeArray = StringTool::split(nodeStr, "/");
	if (!doc.empty() && doc)
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
			node = node.child(nodeArray[i].c_str());
		}
	}
	else
		return false;
	if (node.empty() || !node)
		return false;
	else
		return true;
}

bool XMLStream::LoadXMLAttributeString(pugi::xml_node& node, const char* attributeName, std::string& attri)
{
	if (!node || node.empty())
		return false;
	auto attr = node.attribute(attributeName);
	if (attr && !attr.empty())
	{
		attri = node.attribute(attributeName).as_string();
		return true;
	}
	return false;
}

bool XMLStream::LoadXMLAttributeInt(pugi::xml_node& node, const char* attributeName, int& attri)
{
	if (!node || node.empty())
		return false;
	auto attr = node.attribute(attributeName);
	if (attr && !attr.empty())
	{
		attri = node.attribute(attributeName).as_int();
		return true;
	}
	return false;
}

bool XMLStream::LoadXMLAttributeUInt(pugi::xml_node& node, const char* attributeName, uint32_t& attri)
{
	if (!node|| node.empty())
		return false;
	auto attr = node.attribute(attributeName);
	if (attr&& !attr.empty())
	{
		attri = node.attribute(attributeName).as_uint();
		return true;
	}
	return false;
}

bool XMLStream::LoadXMLAttributeUint64(pugi::xml_node& node, const char* attributeName, uint64_t& attri)
{
	if (!node || node.empty())
		return false;
	auto attr = node.attribute(attributeName);
	if (attr && !attr.empty())
	{
		attri = node.attribute(attributeName).as_ullong();
		return true;
	}
	return false;
}

bool XMLStream::LoadXMLAttributeInt64(pugi::xml_node& node, const char* attributeName, int64_t& attri)
{
	if (!node|| node.empty())
		return false;
	auto attr = node.attribute(attributeName);
	if (attr&& !attr.empty())
	{
		attri = node.attribute(attributeName).as_llong();
		return true;
	}
	return false;
}

bool XMLStream::LoadXMLAttributeBool(pugi::xml_node& node, const char* attributeName, bool& attri)
{
	if (!node || node.empty())
		return false;
	auto attr = node.attribute(attributeName);
	if (attr && !attr.empty())
	{
		attri = node.attribute(attributeName).as_bool();
		return true;
	}
	return false;
}

bool XMLStream::LoadXMLAttributeFloat(pugi::xml_node& node, const char* attributeName, float& attri)
{
	if (!node || node.empty())
		return false;
	auto attr = node.attribute(attributeName);
	if (attr && !attr.empty())
	{
		attri = node.attribute(attributeName).as_float();
		return true;
	}
	return false;
}

bool XMLStream::FindNodeByPath(pugi::xml_node& node, std::string path, pugi::xml_node& result, bool bCreateEmpty)
{
	path = StringTool::ClearSpace(path);
	if (path[0] == '/')
	{
		path = StringTool::Right(path, 1);
	}
	std::vector<std::string>paths = StringTool::split(path, "/");
	result = pugi::xml_node();

	pugi::xml_node current = node;
	bool bFound = true;
	for (int  i = 0 ; i < paths.size() ;i++)
	{
		auto next = current.child(paths[i].c_str());
		if (!next)
		{
			bFound = false;
			if (bCreateEmpty)
			{
				next = current.append_child(paths[i].c_str());
			}
			else
				break;
		}
		current = next;
		if (i >= paths.size() - 1)
		{
			result = current;
		}
	}
	if (bFound)
		return true;
	else
		return false;
}

pugi::xml_node XMLStream::FindNodeByPath(pugi::xml_node& node, std::string path)
{
	pugi::xpath_query xQuery(path.c_str());
	if (xQuery)
	{
		auto xNodeSet = node.select_node(xQuery);
		if (xNodeSet)
		{
			return xNodeSet.node();
		}
	}
	return pugi::xml_node();
}

bool XMLStream::CreateXMLDocument(std::string path, pugi::xml_document& doc)
{
	pugi::xml_node decl = doc.prepend_child(pugi::node_declaration);
	decl.append_attribute("version") = "1.0";
	decl.append_attribute("encoding") = "UTF-8";
	auto dirPath = FileSystem::GetFilePath(path);
	if (FileSystem::IsDir(dirPath))
	{
		return doc.save_file(path.c_str());
	}
	return false;
}
