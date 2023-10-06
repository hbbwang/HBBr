#include "CustomSearchLine.h"
#include "HString.h"
#include "EditorCommonFunction.h"
#include <QPixmap>
#include <qabstractitemview.h>
CustomSearchLine::CustomSearchLine(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	this->setObjectName("SearchLine");
	ui.comboBox->setObjectName("SearchLine_ComboBox");
	ui.comboBox->view()->viewport()->setObjectName("setObjectName_ComboBoxViewport");
	ui.label->setMinimumSize(30, 30);
	ui.label->setMaximumSize(30, 30);
	this->setMaximumHeight(30);
	this->setMinimumHeight(30);
	//QPixmap image((FileSystem::GetProgramPath() + "Config/Theme/Icons/ICON_SEARCH.png").c_str());
	//image.scaled(4,4);
	//ui.label->setPixmap(image);

}

CustomSearchLine::~CustomSearchLine()
{

}
