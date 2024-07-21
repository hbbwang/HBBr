#pragma once
#include <qwidget.h>
#include <qevent.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include "qpixmap.h"
#include "qapplication.h"
#include "qdesktopwidget.h"
#include "qstyle.h"
#include "qpainter.h"
#include "qstyleoption.h"
#include <qtimer.h>

#include <qfile.h>
#include <QTextStream>
class CustomTitleBar :
	public QWidget
{
	Q_OBJECT

public:
	CustomTitleBar(QWidget* parent = Q_NULLPTR);

	void SetChildWidget(QWidget* child);
	inline QWidget* GetChildWidget() const { return p_child; }

	//Set Title Height
	inline void SetTitleHeight(int newHeight)
	{
		titleHeight = newHeight;
		UpdateTitle();
	}

	//Set Title String
	inline void SetTitleText(QString newTitle)
	{
		if (p_labelTitle)
		{
			p_labelTitle->setText(newTitle);
			UpdateTitle();
		}
	}

	inline void SetTitleBottomWidth(int newWidth)
	{
		if (newWidth > 0)
		{
			titleBottonWidth = newWidth;
			UpdateTitle();
		}
	}

	inline void SetWindowBorder(int newBorderSize)
	{
		if (newBorderSize > 0)
		{
			borderSize = newBorderSize;
			UpdateTitle();
		}
	}

	void UpdateTitle();

	//inline void SetMinimumSize(QSize newSize) { setMinimumSize(newSize); }
	//inline void SetMinimumSize(int width,int height) { setMinimumSize(QSize(width,height)); }

	QToolButton* GetCloseButton()const { return p_closeButton; }
	QToolButton* GetMaxButton()const { return p_maxButton; }
	QToolButton* GetMinButton()const { return p_minButton; }
	QLabel* GetTitleLabel()const { return p_labelTitle; }

	bool bEnableCustomClose = false;

	//来用更新边框光标的
	void MouseMoveUpdate();

private:
	QWidget* titleWidget;

	class QVBoxLayout* mainLayout;
	class QHBoxLayout* titleLayout;

	int titleHeight = 30;
	int titleBottonWidth = 30;

	QWidget* p_child = NULL;			//嵌套的子窗口
	QPoint		mousePosition;			//局部鼠标位置
	QPoint		lastMouseGlobalPosition;//上一帧的全屏鼠标位置
	bool		bIsMousePressed;

	QToolButton* p_minButton;	//最小化按钮
	QToolButton* p_maxButton;	//最大化按钮
	QToolButton* p_closeButton;	//关闭按钮
	QLabel* p_labelTitle;	//标题栏名称
	bool			bIsMax = false;
	QPixmap			normalPix;		//缩小图标
	QPixmap			maxPix;			//最大化图标

	QString			bkUrl;				//标题栏背景图片路径
	QPixmap			bkImage;			//标题栏背景图片
	int				bkImageState = 0;	//状态，0为中心平铺，1为左上角平铺

	/*---------窗口边框----*/
	int		borderSize = 4;

	/*---------窗口缩放----*/
	//上
	QRect	topHit;
	bool	bResizeByTopHit = false;
	//下
	QRect	bottomHit;
	bool	bResizeByBottomHit = false;
	//左
	QRect	leftHit;
	bool	bResizeByLeftHit = false;
	//右
	QRect	rightHit;
	bool	bResizeByRightHit = false;
	//右下
	QRect	rightBottomHit;
	bool	bResizeByRbHit = false;
	//右上
	QRect	rightTopHit;
	bool	bResizeByRtHit = false;
	//左下
	QRect	leftBottomHit;
	bool	bResizeByLbHit = false;
	//左上
	QRect	leftTopHit;
	bool	bResizeByLtHit = false;
	//计时器
	QTimer* timer;


protected:
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void mouseDoubleClickEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void resizeEvent(QResizeEvent* event);
	virtual void showEvent(QShowEvent* event);
	virtual void paintEvent(QPaintEvent* event);
	virtual void focusOutEvent(QFocusEvent* event);

	//功能函数
	static bool IsInside(QRect r, QPoint p)
	{
		if (p.x() >= r.left() && p.x() <= r.right() &&
			p.y() >= r.top() && p.y() <= r.bottom())
			return true;
		else
			return false;
	}

private slots:
	void actionMin();      //最小化窗口
	void actionMax();      //最大化窗口
	void actionClose();    //关闭窗口

};