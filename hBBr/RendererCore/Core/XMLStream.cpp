#include "XMLStream.h"


bool XMLStream::LoadXML(const char* path, pugi::xml_document& doc)
{
	auto result = doc.load_file(path);
	if (result.status != pugi::status_ok)
	{
		return false;
	}
	return true;
}

bool XMLStream::LoadXMLNode(const char* nodeName, pugi::xml_node& node)
{
	std::string nodeP = nodeName;
	
	return false;
}
