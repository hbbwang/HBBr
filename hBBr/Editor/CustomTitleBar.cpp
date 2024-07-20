#include "CustomTitleBar.h"
#include "qmessagebox.h"
#include "qlayout.h"
CustomTitleBar::CustomTitleBar(QWidget* parent) :QWidget(parent)
{
	mainLayout = new QVBoxLayout(this);
	this->setLayout(mainLayout);
	titleWidget = new QWidget(this);
	mainLayout->addWidget(titleWidget);
	titleLayout = new QHBoxLayout(titleWidget);
	titleWidget->setLayout(titleLayout);

	mainLayout->setContentsMargins(borderSize, borderSize, borderSize, borderSize);
	mainLayout->setSpacing(0);
	titleLayout->setContentsMargins(0, 0, 0, 0);
	titleLayout->setSpacing(0);

	p_labelTitle = new QLabel(this);//标题
	p_minButton = new QToolButton(this);//最小化按钮
	p_maxButton = new QToolButton(this);//最大化按钮
	p_closeButton = new QToolButton(this); //关闭按钮

	titleLayout->addWidget(p_labelTitle);
	titleLayout->addSpacerItem(new QSpacerItem(99999, 1, QSizePolicy::Policy::Preferred));//加一个展位拉伸
	titleLayout->addWidget(p_minButton);
	titleLayout->addWidget(p_maxButton);
	titleLayout->addWidget(p_closeButton);

	//SetTitleBackgroundImage();

	QPixmap  closePix = style()->standardPixmap(QStyle::SP_TitleBarCloseButton);
	p_closeButton->setIcon(closePix);
	maxPix = style()->standardPixmap(QStyle::SP_TitleBarMaxButton);
	p_maxButton->setIcon(maxPix);
	QPixmap minPix = style()->standardPixmap(QStyle::SP_TitleBarMinButton);
	p_minButton->setIcon(minPix);
	normalPix = style()->standardPixmap(QStyle::SP_TitleBarNormalButton);

	setObjectName("CustomTitleBar");
	titleWidget->setObjectName("CustomTitleBar_Title");
	p_minButton->setObjectName("CustomTitleBar_TitleMinButton");
	p_maxButton->setObjectName("CustomTitleBar_TitleMaxButton");
	p_closeButton->setObjectName("CustomTitleBar_TitleCloseButton");
	p_labelTitle->setObjectName("CustomTitleBar_TitleText");

	/******************信号与槽连接**************/
	//最小化按钮
	connect(p_minButton, SIGNAL(clicked(bool)), SLOT(actionMin()));
	//最大化按钮
	connect(p_maxButton, SIGNAL(clicked(bool)), SLOT(actionMax()));
	//关闭按钮  
	connect(p_closeButton, SIGNAL(clicked(bool)), SLOT(actionClose()));
	p_labelTitle->setText("MainWindow");
	//setAutoFillBackground(true);

	//this->setAttribute(Qt::WA_TranslucentBackground);//设置窗口背景透明
	this->setWindowFlags(Qt::FramelessWindowHint); //去掉标题栏

	////创建定时器用于每帧更新
	//timer = new QTimer();
	//timer->setInterval(200);
	//QObject::connect(timer, SIGNAL(timeout()), this, SLOT(customUpdate()));
	//timer->start();
	UpdateTitle();

	SetTitleHeight(titleHeight);
}

void CustomTitleBar::SetChildWidget(QWidget* child)
{
	if (child && p_child == NULL)
	{
		p_child = child;
		p_child->setParent(this);
		p_child->show();
		mainLayout->addWidget(p_child);
	}
}

void CustomTitleBar::UpdateTitle()
{
	//更新高度
	titleWidget->setMinimumHeight(titleHeight);
	titleWidget->setMaximumHeight(titleHeight);
	//p_labelTitle->setMaximumHeight(titleHeight);
	//p_minButton->setMaximumHeight(titleHeight);
	//p_maxButton->setMaximumHeight(titleHeight);
	//p_closeButton->setMaximumHeight(titleHeight);
	
	//更新缩放点位置
	topHit = QRect(borderSize, 0, this->width() - borderSize * 2, borderSize);
	bottomHit = QRect(borderSize, this->height() - borderSize, this->width() - borderSize * 2, borderSize);
	leftHit = QRect(0, borderSize, borderSize, this->height() - borderSize * 2);
	rightHit = QRect(this->width() - borderSize, borderSize, borderSize, this->height() - borderSize * 2);
	rightBottomHit = QRect(this->width() - borderSize, this->height() - borderSize, borderSize, borderSize);
	rightTopHit = QRect(this->width() - borderSize, 0, borderSize, borderSize);
	leftBottomHit = QRect(0, this->height() - borderSize, borderSize, borderSize);
	leftTopHit = QRect(0, 0, borderSize, borderSize);
}

void CustomTitleBar::MouseMoveUpdate()
{
	//最好判断下是否允许切换光标，不然每帧总在切换的话会导致缩放卡顿
	QPoint CurrentPos = mapFromGlobal(QCursor::pos());
	if (IsInside(rightBottomHit, CurrentPos) || bResizeByRbHit)
	{
		if (cursor() != Qt::SizeFDiagCursor)
			this->setCursor(Qt::SizeFDiagCursor);
	}
	else if (IsInside(rightTopHit, CurrentPos) || bResizeByRtHit)
	{
		if (cursor() != Qt::SizeBDiagCursor)
			this->setCursor(Qt::SizeBDiagCursor);
	}
	else if (IsInside(leftBottomHit, CurrentPos) || bResizeByLbHit)
	{
		if (cursor() != Qt::SizeBDiagCursor)
			this->setCursor(Qt::SizeBDiagCursor);
	}
	else if (IsInside(leftTopHit, CurrentPos) || bResizeByLtHit)
	{
		if (cursor() != Qt::SizeFDiagCursor)
			this->setCursor(Qt::SizeFDiagCursor);
	}
	else if (IsInside(topHit, CurrentPos) || bResizeByTopHit)
	{
		if (cursor() != Qt::SizeVerCursor)
			this->setCursor(Qt::SizeVerCursor);
	}
	else if (IsInside(bottomHit, CurrentPos) || bResizeByBottomHit)
	{
		if (cursor() != Qt::SizeVerCursor)
			this->setCursor(Qt::SizeVerCursor);
	}
	else if (IsInside(leftHit, CurrentPos) || bResizeByLeftHit)
	{
		if (cursor() != Qt::SizeHorCursor)
			this->setCursor(Qt::SizeHorCursor);
	}
	else if (IsInside(rightHit, CurrentPos) || bResizeByRightHit)
	{
		if (cursor() != Qt::SizeHorCursor)
			this->setCursor(Qt::SizeHorCursor);
	}
	else
	{
		if (cursor() != Qt::ArrowCursor)
			this->setCursor(Qt::ArrowCursor);
	}
}

void CustomTitleBar::mousePressEvent(QMouseEvent* event)
{
	mousePosition = event->pos();  //当鼠标单击窗体准备拖动时，初始化鼠标在窗体中的相对位置
	if (IsInside(rightBottomHit, mousePosition))
	{
		bResizeByRbHit = true;
		this->setCursor(Qt::SizeFDiagCursor);
		QCursor::setPos(this->geometry().bottomRight());
		lastMouseGlobalPosition = QCursor::pos();
	}
	else if (IsInside(leftBottomHit, mousePosition))
	{
		bResizeByLbHit = true;
		this->setCursor(Qt::SizeBDiagCursor);
		QCursor::setPos(this->geometry().bottomLeft());
		lastMouseGlobalPosition = QCursor::pos();
	}
	else if (IsInside(rightTopHit, mousePosition))
	{
		bResizeByRtHit = true;
		this->setCursor(Qt::SizeBDiagCursor);
		QCursor::setPos(this->geometry().topRight());
		lastMouseGlobalPosition = QCursor::pos();
	}
	else if (IsInside(leftTopHit, mousePosition))
	{
		bResizeByLtHit = true;
		this->setCursor(Qt::SizeFDiagCursor);
		QCursor::setPos(this->geometry().topLeft());
		lastMouseGlobalPosition = QCursor::pos();
	}
	else if (IsInside(topHit, mousePosition))
	{
		bResizeByTopHit = true;
		this->setCursor(Qt::SizeVerCursor);
		QCursor::setPos(QCursor::pos().x(), this->geometry().top());
		lastMouseGlobalPosition = QCursor::pos();
	}
	else if (IsInside(bottomHit, mousePosition))
	{
		bResizeByBottomHit = true;
		this->setCursor(Qt::SizeVerCursor);
		QCursor::setPos(QCursor::pos().x(), this->geometry().bottom());
		lastMouseGlobalPosition = QCursor::pos();
	}
	else if (IsInside(leftHit, mousePosition))
	{
		bResizeByLeftHit = true;
		this->setCursor(Qt::SizeHorCursor);
		QCursor::setPos(this->geometry().left(), QCursor::pos().y());
		lastMouseGlobalPosition = QCursor::pos();
	}
	else if (IsInside(rightHit, mousePosition))
	{
		bResizeByRightHit = true;
		this->setCursor(Qt::SizeHorCursor);
		QCursor::setPos(this->geometry().right(), QCursor::pos().y());
		lastMouseGlobalPosition = QCursor::pos();
	}
	else if (mousePosition.y() <= titleHeight)
	{
		bIsMousePressed = true;
	}
	QWidget::mousePressEvent(event);
}

void CustomTitleBar::mouseReleaseEvent(QMouseEvent* event)
{
	bIsMousePressed = false;
	bResizeByRbHit = false;
	bResizeByRtHit = false;
	bResizeByLbHit = false;
	bResizeByLtHit = false;
	bResizeByTopHit = false;
	bResizeByBottomHit = false;
	bResizeByLeftHit = false;
	bResizeByRightHit = false;
	this->setCursor(Qt::ArrowCursor);
	QWidget::mouseReleaseEvent(event);
}

void CustomTitleBar::mouseDoubleClickEvent(QMouseEvent* event)
{
	QWidget::mouseDoubleClickEvent(event);
	if (mousePosition.y() < 0)
		return;
	if (mousePosition.y() > titleHeight)
		return;
	if (bIsMax)
	{
		this->showNormal();
		bIsMax = false;
		p_maxButton->setIcon(maxPix);
		bIsMousePressed = false;
	}
	else
	{
		this->showMaximized();
		bIsMax = true;
		p_maxButton->setIcon(normalPix);
		bIsMousePressed = false;
	}
}

void CustomTitleBar::mouseMoveEvent(QMouseEvent* event)
{
	QWidget::mouseMoveEvent(event);
	if (bIsMousePressed )
	{
		bResizeByRbHit = false;
		bResizeByRtHit = false;
		bResizeByLbHit = false;
		bResizeByLtHit = false;
		bResizeByTopHit = false;
		bResizeByBottomHit = false;
		bResizeByLeftHit = false;
		bResizeByRightHit = false;
		QPoint movePot = event->globalPos() - mousePosition;
		move(movePot);

		if (bIsMax)
		{
			this->showMaximized();
			bIsMax = true;
			p_maxButton->setIcon(normalPix);
		}
	}
	if (bResizeByRbHit)
	{
		if (width() <= minimumSize().width() || height() <= minimumSize().height())
			lastMouseGlobalPosition = QPoint(this->geometry().x() + width(), this->geometry().y() + height());//保持定点缩放
		QPoint newPoint = event->globalPos() - lastMouseGlobalPosition;
		this->setGeometry(this->geometry().left(), this->geometry().top(), this->geometry().width() + newPoint.x(), this->geometry().height() + newPoint.y());
		lastMouseGlobalPosition = event->globalPos();
	}
	else if (bResizeByRtHit)
	{
		if (width() <= minimumSize().width() || height() <= minimumSize().height())
			lastMouseGlobalPosition = QPoint(this->geometry().x() + width(), this->geometry().y());
		QPoint newPoint = event->globalPos() - lastMouseGlobalPosition;
		if (this->geometry().height() - newPoint.y() < minimumHeight())
			newPoint.setY(0);
		this->setGeometry(this->geometry().left(), this->geometry().top() + newPoint.y(), this->geometry().width() + newPoint.x(), this->geometry().height() - newPoint.y());
		lastMouseGlobalPosition = event->globalPos();
	}
	else if (bResizeByLbHit)
	{
		if (width() <= minimumSize().width() || height() <= minimumSize().height())
			lastMouseGlobalPosition = QPoint(this->geometry().x(), this->geometry().y() + height());
		QPoint newPoint = event->globalPos() - lastMouseGlobalPosition;
		if (this->geometry().width() - newPoint.x() < minimumWidth())
			newPoint.setX(0);
		this->setGeometry(this->geometry().left() + newPoint.x(), this->geometry().top(), this->geometry().width() - newPoint.x(), this->geometry().height() + newPoint.y());
		lastMouseGlobalPosition = event->globalPos();
	}
	else if (bResizeByLtHit)
	{
		if (width() <= minimumSize().width() || height() <= minimumSize().height())
			lastMouseGlobalPosition = QPoint(this->geometry().x(), this->geometry().y());
		QPoint newPoint = event->globalPos() - lastMouseGlobalPosition;
		if (this->geometry().width() - newPoint.x() < minimumWidth())
			newPoint.setX(0);
		if (this->geometry().height() - newPoint.y() < minimumHeight())
			newPoint.setY(0);
		this->setGeometry(this->geometry().left() + newPoint.x(), this->geometry().top() + newPoint.y(), this->geometry().width() - newPoint.x(), this->geometry().height() - newPoint.y());
		lastMouseGlobalPosition = event->globalPos();
	}
	else if (bResizeByTopHit)
	{
		if (width() <= minimumSize().width() || height() <= minimumSize().height())
			lastMouseGlobalPosition = QPoint(this->geometry().x(), this->geometry().y());
		QPoint newPoint = event->globalPos() - lastMouseGlobalPosition;
		if (this->geometry().height() - newPoint.y() < minimumHeight())
			newPoint.setY(0);
		this->setGeometry(this->geometry().left(), this->geometry().top() + newPoint.y(), this->geometry().width(), this->geometry().height() - newPoint.y());
		lastMouseGlobalPosition = event->globalPos();
	}
	else if (bResizeByBottomHit)
	{
		if (width() <= minimumSize().width() || height() <= minimumSize().height())
			lastMouseGlobalPosition = QPoint(this->geometry().x(), this->geometry().y() + height());
		QPoint newPoint = event->globalPos() - lastMouseGlobalPosition;
		this->setGeometry(this->geometry().left(), this->geometry().top(), this->geometry().width(), this->geometry().height() + newPoint.y());
		lastMouseGlobalPosition = event->globalPos();
	}
	else if (bResizeByLeftHit)
	{
		if (width() <= minimumSize().width() || height() <= minimumSize().height())
			lastMouseGlobalPosition = QPoint(this->geometry().x(), this->geometry().y());
		QPoint newPoint = event->globalPos() - lastMouseGlobalPosition;
		if (this->geometry().width() - newPoint.x() < minimumWidth())
			newPoint.setX(0);
		this->setGeometry(this->geometry().left() + newPoint.x(), this->geometry().top(), this->geometry().width() - newPoint.x(), this->geometry().height());
		lastMouseGlobalPosition = event->globalPos();
	}
	else if (bResizeByRightHit)
	{
		if (width() <= minimumSize().width() || height() <= minimumSize().height())
			lastMouseGlobalPosition = QPoint(this->geometry().x() + width(), this->geometry().y());
		QPoint newPoint = event->globalPos() - lastMouseGlobalPosition;
		this->setGeometry(this->geometry().left(), this->geometry().top(), this->geometry().width() + newPoint.x(), this->geometry().height());
		lastMouseGlobalPosition = event->globalPos();
	}

}

void CustomTitleBar::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);
	UpdateTitle();
}

void CustomTitleBar::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
}

void CustomTitleBar::paintEvent(QPaintEvent* event)
{
	QPainter p(this);
	QStyleOption opt;
	opt.init(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);//使用QSS
	QWidget::paintEvent(event);

}


void CustomTitleBar::focusOutEvent(QFocusEvent* event)
{
	bIsMousePressed = false;
	bResizeByRbHit = false;
	bResizeByRtHit = false;
	bResizeByLbHit = false;
	bResizeByLtHit = false;
	bResizeByTopHit = false;
	bResizeByBottomHit = false;
	bResizeByLeftHit = false;
	bResizeByRightHit = false;
	QWidget::focusOutEvent(event);
}

void CustomTitleBar::actionMin()
{
	showMinimized();
}

void CustomTitleBar::actionMax()
{
	if (bIsMax)
	{
		this->showNormal();
		bIsMax = false;
		p_maxButton->setIcon(maxPix);
	}
	else
	{
		this->showMaximized();
		bIsMax = true;
		p_maxButton->setIcon(normalPix);
	}
}

void CustomTitleBar::actionClose()
{
	if (p_child)
	{
		p_child->close();
	}
	if(!bEnableCustomClose)
		close();
}
