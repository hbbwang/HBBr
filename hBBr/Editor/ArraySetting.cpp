#include "ArraySetting.h"
#include "VectorSetting.h"

ArraySetting::ArraySetting(QString name ,QWidget *parent, QString type)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.Name->setText(name);
	this->setMouseTracking(true);
	ui.ArrayNum->setStyleSheet("border:2px solid rgb(10,10,10);border-radius:8px;background-color:rgb(75,75,75);color:rgb(255,255,255);}");
	ui.listWidget->setObjectName("property_list_widget");
	ui.listWidget->setStyleSheet("#property_list_widget{border:2px solid rgb(10,10,10);border-radius:8px;background-color:transparent;color:rgb(255,255,255);}}");
	ui.deleteButton->setStyleSheet("QPushButton{ \
                                     border: 1px solid;\
                                     background-color:rgb(75,75,75);\
                                     color:rgb(220,220,220);\
                                     border-radius: 8px;\
                                 }\
                                 QPushButton:pressed{\
									 border: 1px solid;\
                                     background-color:rgb(35,55,55);\
									 color:rgb(255,255,255);\
                                     border-radius:8px;\
									 font:75 16pt;\
                                 }");
	ui.deleteButton->setMaximumHeight(30);
	ui.ArrayNum->setAlignment(Qt::AlignRight);
	this->setContextMenuPolicy(Qt::NoContextMenu);

	QFont font("Microsoft YaHei", 11, 50);
	this->setFont(font);
	//
	//
	if (type.compare("float", Qt::CaseInsensitive)==0)
	{
		typeID = 0;
	}
	else if(type.compare("float3", Qt::CaseInsensitive) == 0|| type.compare("vector3", Qt::CaseInsensitive) == 0)
	{
		typeID = 1;
	}
	else if (type.compare("float4", Qt::CaseInsensitive) == 0 || type.compare("vector4", Qt::CaseInsensitive) == 0)
	{
		typeID = 2;
	}
	
	connect(ui.ArrayNum,SIGNAL(valueChanged(int)),this,SLOT(valueChange(int)));
	connect(ui.deleteButton, SIGNAL(clicked(bool)), this, SLOT(deleteItem()));
}

ArraySetting::~ArraySetting()
{

}

void ArraySetting::valueChange(int num)
{
	if (num > lastArrayCount)
	{
		for (int i = ui.ArrayNum->value(); i > ui.listWidget->count();)
		{
			AddNewVaule();
		}
	}
	else if(num < lastArrayCount)
	{
		RemoveValue(lastArrayCount-1);
	}
}

void ArraySetting::AddNewVaule()
{
	VectorSetting* newValue=NULL;
	switch (typeID)
	{
	case 0 :
		newValue = new VectorSetting("0", this, 1);
		break;
	case 1:
		newValue = new VectorSetting("0", this, 3);
		break;
	case 2:
		newValue = new VectorSetting("0", this, 4);
		break;
	}
	arraylist.append(newValue);
	QListWidgetItem* item = new QListWidgetItem();
	arrayitem.append(item);
	ui.listWidget->addItem(item);
	ui.listWidget->setItemWidget(item, newValue);
	newValue->SetName("#" + QString::number(arraylist.size() - 1));
	lastArrayCount++;
}

void ArraySetting::RemoveValue(int index)
{
	if (arrayitem.size() >= index + 1)
	{
		ui.listWidget->removeItemWidget(arrayitem[index]);
		delete arrayitem[index];
		arrayitem.removeAt(index);
		arraylist.removeAt(index);
		lastArrayCount--;
	}
}

void ArraySetting::paintEvent(QPaintEvent* event)
{
	if (ui.listWidget->count() <= 0 && ui.ArrayNum->value() <= 0)
	{
		ui.listWidget->hide();
	}
	else
	{
		ui.listWidget->show();
	}
}

void ArraySetting::deleteItem()
{
	if (ui.listWidget->count() > 0 && ui.listWidget->currentRow()>=0 && ui.listWidget->selectedItems().size()>0)
	{
		RemoveValue(ui.listWidget->currentRow());
		ui.ArrayNum->setValue(lastArrayCount);
		for (int i = 0 ;i< arraylist.size();i++)
		{
			arraylist[i]->SetName("#"+QString::number(i));
		}
	}
}
