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

	//���ø��±߿����
	void MouseMoveUpdate();

private:
	QWidget* titleWidget;

	class QVBoxLayout* mainLayout;
	class QHBoxLayout* titleLayout;

	int titleHeight = 30;
	int titleBottonWidth = 30;

	QWidget* p_child = NULL;			//Ƕ�׵��Ӵ���
	QPoint		mousePosition;			//�ֲ����λ��
	QPoint		lastMouseGlobalPosition;//��һ֡��ȫ�����λ��
	bool		bIsMousePressed;

	QToolButton* p_minButton;	//��С����ť
	QToolButton* p_maxButton;	//��󻯰�ť
	QToolButton* p_closeButton;	//�رհ�ť
	QLabel* p_labelTitle;	//����������
	bool			bIsMax = false;
	QPixmap			normalPix;		//��Сͼ��
	QPixmap			maxPix;			//���ͼ��

	QString			bkUrl;				//����������ͼƬ·��
	QPixmap			bkImage;			//����������ͼƬ
	int				bkImageState = 0;	//״̬��0Ϊ����ƽ�̣�1Ϊ���Ͻ�ƽ��

	/*---------���ڱ߿�----*/
	int		borderSize = 4;

	/*---------��������----*/
	//��
	QRect	topHit;
	bool	bResizeByTopHit = false;
	//��
	QRect	bottomHit;
	bool	bResizeByBottomHit = false;
	//��
	QRect	leftHit;
	bool	bResizeByLeftHit = false;
	//��
	QRect	rightHit;
	bool	bResizeByRightHit = false;
	//����
	QRect	rightBottomHit;
	bool	bResizeByRbHit = false;
	//����
	QRect	rightTopHit;
	bool	bResizeByRtHit = false;
	//����
	QRect	leftBottomHit;
	bool	bResizeByLbHit = false;
	//����
	QRect	leftTopHit;
	bool	bResizeByLtHit = false;
	//��ʱ��
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

	//���ܺ���
	static bool IsInside(QRect r, QPoint p)
	{
		if (p.x() >= r.left() && p.x() <= r.right() &&
			p.y() >= r.top() && p.y() <= r.bottom())
			return true;
		else
			return false;
	}

private slots:
	void actionMin();      //��С������
	void actionMax();      //��󻯴���
	void actionClose();    //�رմ���

};