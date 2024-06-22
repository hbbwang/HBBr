#include "MaterialEditor.h"
#include "QStyleOption.h"
#include "QPainter.h"
#include "EditorMain.h"
#include <qevent.h>
#include <qmessagebox.h>
#include <EditorCommonFunction.h>
#include <QSplitter>
#include <qlayout.h>
#include "VulkanRenderer.h"
MaterialDetailWindow::MaterialDetailWindow(std::weak_ptr<Material> mat, QWidget* parent)
	:QWidget(parent)
{
	_material = mat;
	_parent = (MaterialEditor*)parent;

	//创建Material Editor 的内容
	//分割窗口
	_splitter = new QSplitter(this);
	_splitter->setOrientation(Qt::Orientation::Horizontal);
	_left = new QWidget(_splitter);
	_right = new QWidget(_splitter);
	_splitter->addWidget(_left);
	_splitter->addWidget(_right);
	//Left
	{
		QVBoxLayout* vlayout = new QVBoxLayout(_left);
		_left->setLayout(vlayout); 
		vlayout->setSpacing(0); vlayout->setContentsMargins(0, 0, 0, 0);
		auto left_v_splitter = new QSplitter(_left);
		vlayout->addWidget(left_v_splitter);
		left_v_splitter->setOrientation(Qt::Orientation::Vertical);
		_left_up = new QWidget(left_v_splitter); left_v_splitter->addWidget(_left_up);
		_left_bottom = new QWidget(left_v_splitter); left_v_splitter->addWidget(_left_up);	
		//左边面板，分上下两块，上面是三维视图，下面是材质属性设置
		auto matWindow = VulkanApp::CreateNewWindow(512, 512, _material.lock()->_assetInfo.lock()->guid.str().c_str(), true);
		HWND hwnd = (HWND)VulkanApp::GetWindowHandle(matWindow);

	}
	//Right
	{
		//右边面板，主要是材质（Shader）的参数设置

	}
}

MaterialDetailWindow::~MaterialDetailWindow()
{

}

void MaterialDetailWindow::resizeEvent(QResizeEvent* event)
{
	_splitter->resize(width(), height());
	QWidget::resizeEvent(event);
}

void MaterialDetailWindow::closeEvent(QCloseEvent* event)
{

}

void MaterialDetailWindow::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);
	QStyleOption styleOpt;
	styleOpt.init(this);
	QPainter painter(this);
	style()->drawPrimitive(QStyle::PE_Widget, &styleOpt, &painter, this);
	QWidget::paintEvent(event);
}

#include "qdebug.h"
// 
//
MaterialEditor* MaterialEditor::_mainWindow = nullptr;

MaterialEditor::MaterialEditor(QWidget *parent)
	: QTabWidget(parent)
{
	//ui.setupUi(this);
	setObjectName("MaterialEditor");
	setWindowFlags(Qt::Window);
	setMovable(true);

	tabBar()->installEventFilter(this);
	installEventFilter(this);
	//tabBar()->setDocumentMode(true);
}

MaterialEditor::~MaterialEditor()
{

}

void MaterialEditor::closeEvent(QCloseEvent* event)
{
	for (auto& i : _allDetailWindows)
	{
		i.editor->close();
	}
	_allDetailWindows.clear();
	_mainWindow = nullptr;
}

void MaterialEditor::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);
	QStyleOption styleOpt;
	styleOpt.init(this);
	QPainter painter(this);
	style()->drawPrimitive(QStyle::PE_Widget, &styleOpt, &painter, this);
	QTabWidget::paintEvent(event);
}

MaterialDetailWindow* MaterialEditor::OpenMaterialEditor(std::weak_ptr<Material> mat, bool bTab)
{
	int width = 1024, height = 768;
	GetEditorInternationalizationInt("MaterialEditor", "WindowWidth", width);
	GetEditorInternationalizationInt("MaterialEditor", "WindowHeight", height);
	if (_mainWindow == nullptr)
	{
		MaterialEditor* main = new MaterialEditor(EditorMain::_self);
		main->resize(width, height);
		main->show();
		_mainWindow = main;
	}
	//我们不会打开相同的材质球
	for (auto& i : _mainWindow->_allDetailWindows)
	{
		if (i.editor->_material.lock()->_assetInfo.lock()->guid == mat.lock()->_assetInfo.lock()->guid)
		{
			if (bTab == i.bTab)
			{
				_mainWindow->setCurrentWidget(i.editor);
				return i.editor;
			}
			else
				break;
		}
	}
	MaterialDetailWindow* newWidget = new MaterialDetailWindow(mat, _mainWindow);
	newWidget->setObjectName("MaterialEditor_Instance");
	newWidget->setWindowTitle(mat.lock()->_assetInfo.lock()->displayName.c_str());
	newWidget->installEventFilter(_mainWindow);
	if (bTab)
	{
		_mainWindow->addTab(newWidget, mat.lock()->_assetInfo.lock()->displayName.c_str());
	}
	else
	{
		newWidget->resize(width, height);
		newWidget->setWindowFlag(Qt::Window, true);
		newWidget->show();
	}
	//_mainWindow->_allDetailWindows.append({newWidget})
	return newWidget;
}

bool MaterialEditor::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == tabBar())
	{
		if (event->type() == QEvent::MouseButtonPress)
		{
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			if (mouseEvent->button() & Qt::LeftButton)
			{
				startPos = mouseEvent->pos();
				index = currentIndex();
			}
		}
		else if (event->type() == QEvent::MouseMove)
		{
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			int distance = (mouseEvent->pos() - startPos).manhattanLength();
			if (distance >= QApplication::startDragDistance())
			{
				auto localPos = mapFromGlobal(mouseEvent->globalPos());
				if (this->tabBar()->count() > 1)
				{
					auto indexWidget = widget(index);
					if (!bInDrag && indexWidget)
					{
						if (!tabBar()->rect().contains(localPos))
						{
							bInDrag = true;
							removeTab(index);						
							currentDragWidget = OpenMaterialEditor(((MaterialDetailWindow*)indexWidget)->_material, false);
						}
					}
				}
			}

			if (currentDragWidget && bInDrag)
			{
				auto pos = mouseEvent->globalPos();
				pos.setX(pos.x() - currentDragWidget->width() / 2);
				int titleBarHeight = currentDragWidget->style()->pixelMetric(QStyle::PM_TitleBarHeight);
				pos.setY(pos.y() - titleBarHeight / 2);
				currentDragWidget->move(pos);
			}
		}
		else if (event->type() == QEvent::MouseButtonRelease)
		{
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			if (mouseEvent->button() & Qt::LeftButton)
			{
				bInDrag = false;
				currentDragWidget = nullptr;
			}
		}
	}
	else if (watched->objectName() == "MaterialEditor_Instance")
	{
		if (event->type() == QEvent::NonClientAreaMouseButtonPress)
			//if (event->type() == QEvent::MouseButtonPress)
		{
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			if (!bResetDrag && mouseEvent->button() == Qt::LeftButton)
			{
				bResetDrag = true;
			}
		}
		else if (event->type() == QEvent::NonClientAreaMouseButtonRelease)
		{
			bResetDrag = false;
		}
		else if (bResetDrag)
			//else if (event->type() == QEvent::MouseMove)
		{
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			MaterialDetailWindow* dragWidget = (MaterialDetailWindow*)watched;
			int distance = (mouseEvent->pos() - startPos).manhattanLength();
			if (distance >= QApplication::startDragDistance())
			{
				auto localPos = tabBar()->mapFromGlobal(QCursor::pos());
				if (tabBar()->rect().contains(localPos))
				{
					bResetDrag = false;
					currentDragWidget = MaterialEditor::OpenMaterialEditor(dragWidget->_material, true);
					setCurrentWidget(currentDragWidget);
					dragWidget->close();
				}
			}
		}
	}
	return false;
}

