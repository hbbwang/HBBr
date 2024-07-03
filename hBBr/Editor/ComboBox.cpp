#include "ComboBox.h"
#include "qmessagebox.h"
#include "qabstractitemview.h"
ComboBox::ComboBox(QString name, QWidget *parent)
	: PropertyClass(parent)
{
	ui.setupUi(this);
	ui.Name->setText(name);
	ui.Name->setObjectName("PropertyName");
	Init();
}

ComboBox::ComboBox(QWidget* parent)
	: PropertyClass(parent)
{
	ui.setupUi(this);
	ui.Name->setHidden(true);
	ui.horizontalLayout->removeItem(ui.horizontalSpacer);
	Init();
}

ComboBox::~ComboBox()
{

}

void ComboBox::Init()
{

	//ui.ComboBox_0->setStyleSheet("border:2px solid rgb(10,10,10);border-radius:8px;background-color:rgb(75,75,75);color:rgb(255,255,255); height:22; outline: none;");
	QLineEdit* line = new QLineEdit(this);
	line->setReadOnly(true);
	line->setContextMenuPolicy(Qt::NoContextMenu);
	line->setFocusPolicy(Qt::ClickFocus);
	line->setEnabled(false);
	ui.ComboBox_0->setLineEdit(line);
	ui.ComboBox_0->lineEdit()->setAlignment(Qt::AlignRight);
	connect(ui.ComboBox_0, SIGNAL(currentTextChanged(const QString&)), this, SLOT(currentTextChanged(const QString&)));

}

void ComboBox::AdjustItemWidth()
{
	//qDebug() << ft.family() << ft.pointSize() << ft.pixelSize();
	QFontMetrics fm(ui.ComboBox_0->font());
	QRect rect;
	int max_len = 0;
	for (int i = 0; i < ui.ComboBox_0->count(); i++)
	{
		rect = fm.boundingRect(ui.ComboBox_0->itemText(i)); //获得字符串所占的像素大小
		if (max_len < rect.width())
		{
			max_len = rect.width();
		}
	}
	max_len *= 1.4;
	int w = qMax(max_len, ui.ComboBox_0->width());
	ui.ComboBox_0->view()->setFixedWidth(w);
}

void ComboBox::AdjustComboBixWidth()
{
	QRect rect;
	int max_len = 0;
	QFontMetrics fm(ui.ComboBox_0->font());
	rect = fm.boundingRect(ui.ComboBox_0->currentText()); 
	max_len = rect.width();
	max_len *= 1.4;
	int w = qMax(max_len, ui.ComboBox_0->width());
	ui.ComboBox_0->setFixedWidth(w);
}

void ComboBox::currentTextChanged(const QString& newText)
{
	//QMessageBox::information(0,0,0,0);
	int index = ui.ComboBox_0->findText(newText);
	_bindCurrentTextChanged(index , newText.toStdString().c_str());
}


