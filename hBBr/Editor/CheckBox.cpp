#include "CheckBox.h"

CheckBox::CheckBox(QWidget* parent , bool bEnbale )
	: PropertyClass(parent)
{
	ui.setupUi(this);
	//ui.ComboBox_0->setStyleSheet("border:2px solid rgb(10,10,10);border-radius:8px;background-color:rgb(75,75,75);color:rgb(255,255,255);height:22;outline: none;}");
	//ui.checkBox->setGeometry(0, 0, 30, 30);
	ui.Name->setHidden(true);
	ui.horizontalSpacer->setGeometry(QRect(0, 0, 0, 0));
	ui.checkBox->resize(20,20);
	ui.checkBox->setChecked(bEnbale);
	ui.checkBox->setObjectName("PropertyCheckBox");
	connect(ui.checkBox,SIGNAL(stateChanged(int)),this,SLOT(stateChanged(int)));
}

CheckBox::CheckBox(HString name, QWidget* parent, bool bEnbale)
{
	ui.setupUi(this);
	ui.Name->setText(name.c_str());
	ui.Name->setObjectName("PropertyName");
	//ui.ComboBox_0->setStyleSheet("border:2px solid rgb(10,10,10);border-radius:8px;background-color:rgb(75,75,75);color:rgb(255,255,255);height:22;outline: none;}");
	//ui.checkBox->setGeometry(0, 0, 30, 30);
	ui.checkBox->setChecked(bEnbale);
	ui.checkBox->setObjectName("PropertyCheckBox");
	connect(ui.checkBox, SIGNAL(stateChanged(int)), this, SLOT(stateChanged(int)));
}

CheckBox::~CheckBox()
{

}

void CheckBox::SetAlignmentLeft()
{
	ui.horizontalLayout->setAlignment(Qt::AlignLeft);
	ui.horizontalLayout->addStretch(1);
}


void CheckBox::stateChanged(int state)
{
	
	if (state == Qt::Checked)
	{
		_callback(true);
		if (_boolBind != nullptr)
			*_boolBind = true;
		if (_boolArrayBind)
			_boolArrayBind->at(_boolArrayBindIndex) = true;
	}
	else if (state == Qt::Unchecked)
	{
		_callback(false);
		if (_boolBind != nullptr)
			*_boolBind = false;
		if (_boolArrayBind)
			_boolArrayBind->at(_boolArrayBindIndex) =  false;
	}		
}
