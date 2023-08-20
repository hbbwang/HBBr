#include "CustomSearchLine.h"
#include "HString.h"
#include "EditorCommonFunction.h"
#include <QPixmap>
CustomSearchLine::CustomSearchLine(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	this->setObjectName("SearchLine");
	//QPixmap image((DString::GetExePathWithoutFileName() + "Config/Theme/Icons/ICON_SEARCH.png").c_str());
	//image.scaled(4,4);
	//ui.label->setPixmap(image);

}

CustomSearchLine::~CustomSearchLine()
{

}
