#include "FloatSetting.h"

FloatSetting::FloatSetting( QWidget *parent, float value , float step, int precision)
	: QDoubleSpinBox(parent)
{
	//this->setObjectName("ParameterFloat");
	//
	SetStep(step);
	SetPrecision(precision);
	setValue(value);
	//
	this->setMouseTracking(true);
	//this->lineEdit()->setMouseTracking(true);
	//this->setStyleSheet("\
	//	QDoubleSpinBox{border:2px solid rgb(10,10,10);border-radius:8px;color:rgb(255,255,255);\
	//	background:qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgb(155, 50, 25), stop:0.25 rgb(155, 50, 25), stop:0.26 rgb(75, 75, 75), stop:1 rgb(75, 75, 75))\
	//	}\
	//	");
	this->lineEdit()->setAlignment(Qt::AlignLeft);
	this->setContextMenuPolicy(Qt::NoContextMenu);
	this->setFocusPolicy(Qt::ClickFocus);
	//
	setRange(-999999,999999);
	setMinimumSize(20,20);
	//setMaximumSize(80,22);

	connect(this,SIGNAL(valueChanged(double)),this,SLOT(ValueChanged(double)));

}

FloatSetting::~FloatSetting()
{
	 
}

void FloatSetting::mousePressEvent(QMouseEvent* event)
{
	QPoint mousepos = event->pos();
	int width = this->width();
	int height = this->height();
	QRect rect = QRect(QPoint(width / 2, 0), QSize(width, height));
	if (rect.contains(mousepos))
	{
		bEnableMovingSetting = true;
		StartPos = QCursor::pos();
		CurrentX = mousepos.x();
		CurrentValue = GetValue();
		AddStore = 0;
		this->clearFocus();
	}
}

void FloatSetting::mouseReleaseEvent(QMouseEvent* event)
{
	bEnableMovingSetting = false;
}

void FloatSetting::mouseMoveEvent(QMouseEvent* event)
{
	QPoint mousepos = event->pos();
	int width = this->width();
	int height = this->height();
	QRect rect = QRect(QPoint(width/2, 0), QSize(width, height));
	if (rect.contains(mousepos) && !bEnableMovingSetting )
	{
		if(cursor().shape() != Qt::SizeHorCursor)
			setCursor(Qt::SizeHorCursor);
	}
	else if(!bEnableMovingSetting)
	{
		if(cursor().shape() != Qt::ArrowCursor)
			setCursor(Qt::ArrowCursor);
	}
	if (bEnableMovingSetting )
	{
		AddStore += (float)(CurrentX - mousepos.x()) * GetStep();
		setValue(CurrentValue - AddStore);
		QCursor::setPos(StartPos);
		if (cursor().shape() != Qt::BlankCursor)
			setCursor(Qt::BlankCursor);
	}
	//
}

void FloatSetting::paintEvent(QPaintEvent* event)
{
	if (this->hasFocus() && !bSelectText)
	{
		this->selectAll();
		bSelectText = true;
	}
	else if (!this->hasFocus())
	{
		bSelectText = false;
	}
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
	QWidget::paintEvent(event);
}

void FloatSetting::wheelEvent(QWheelEvent* event)
{
}

void FloatSetting::ValueChanged(double newValue)
{
	BindValue();
}

void FloatSetting::SetStep(float _step)
{
	this->setSingleStep(_step);
}

void FloatSetting::SetPrecision(int _precision)
{
	this->setDecimals(_precision);
}