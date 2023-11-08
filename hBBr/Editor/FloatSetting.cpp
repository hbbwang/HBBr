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
	this->lineEdit()->setAlignment(Qt::AlignLeft);
	this->setContextMenuPolicy(Qt::NoContextMenu);
	this->setFocusPolicy(Qt::ClickFocus);
	//
	setRange(-999999999.99999,999999999.99999);
	setMinimumSize(20,20);
	//setMaximumSize(80,22);
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

void FloatSetting::SetStep(float _step)
{
	this->setSingleStep(_step);
}

void FloatSetting::SetPrecision(int _precision)
{
	this->setDecimals(_precision);
}