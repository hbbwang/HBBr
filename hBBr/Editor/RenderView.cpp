#include "RenderView.h"
#include "qstylepainter.h"
#include "QStyleOption.h"
#include "qevent.h"
#include <qwindow.h>
#include <qmessagebox.h>
#include "FormMain.h"
#include "GLFWInclude.h"
#include "ConsoleDebug.h"
#include "Component/GameObject.h"
#include "Component/ModelComponent.h"
#include "Asset/World.h"
#include "VulkanRenderer.h"
#ifdef _WIN32
#pragma comment(lib , "RendererCore.lib")
#endif

HWND hwnd;

RenderView::RenderView(QWidget* parent)
	: QWidget(parent)
{
	//若是使用用户自定义绘制，则须要设置WA_PaintOnScreen
	setAttribute(Qt::WA_PaintOnScreen, true);
	//不须要默认的Qt背景
	setAttribute(Qt::WA_NoSystemBackground, true);
	//重绘时，绘制全部像素
	setAttribute(Qt::WA_OpaquePaintEvent, true);

	//setMouseTracking(true);
	setFocusPolicy(Qt::ClickFocus);

	setObjectName("RenderView");

	if (_mainRendererWidget == nullptr)
	{
		//Enable custom loop
		_mainRenderer = VulkanApp::InitVulkanManager(false, true, (void*)this->winId());

		hwnd = (HWND)VulkanApp::GetWindowHandle(_mainRenderer);

		//auto mainRendererWindow = QWindow::fromWinId((WId)hwnd);
		//_mainRendererWidget = QWidget::createWindowContainer(mainRendererWindow, this);
		//_mainRendererWidget->setFocusPolicy(Qt::ClickFocus);
		//_mainRendererWidget->setObjectName("RenderView");
		auto dropFunc = [](VulkanForm *from, HString file) {
			//QMessageBox::information(0, from->name.c_str(), file.c_str(),0);
			ConsoleDebug::printf_endl(" [%s]Drop File : %s", from->name.c_str(), file.c_str());
			auto mainForm = VulkanApp::GetMainForm();
			if (from->renderer == mainForm->renderer && from->renderer->GetWorld())
			{
				auto assetInfo = ContentManager::Get()->GetAssetInfo(file);
				if (!assetInfo.expired())
				{
					GameObject* newObject = from->renderer->GetWorld()->SpawnGameObject(assetInfo.lock()->displayName);
					if (newObject)
					{
						auto modelComp = newObject->AddComponent<ModelComponent>();
						modelComp->SetModelByAssetPath(assetInfo.lock()->assetFilePath);
						newObject->SetObjectName(assetInfo.lock()->displayName);
					}
				}
			}
		};
		VulkanApp::AddDropCallback(dropFunc);
		
	}
}

RenderView::~RenderView()
{

}

void RenderView::Update()
{
	VulkanApp::UpdateForm();
}

void RenderView::showEvent(QShowEvent* event)
{

}

void RenderView::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);
	if (_mainRendererWidget != nullptr)
	{
		_mainRendererWidget->setGeometry(0, 0, width(), height());
	}
	//_sleep(1);
}

bool RenderView::event(QEvent* event)
{
	return QWidget::event(event);
}


void RenderView::closeEvent(QCloseEvent* event)
{
	VulkanApp::DeInitVulkanManager();
}

void RenderView::paintEvent(QPaintEvent* event)
{
	QStylePainter painter(this);
	QStyleOption opt;
	opt.initFrom(this);
	opt.rect = rect();
	painter.drawPrimitive(QStyle::PE_Widget, opt);
	QWidget::paintEvent(event);
}

void RenderView::keyPressEvent(QKeyEvent* event)
{

}

void RenderView::keyReleaseEvent(QKeyEvent* event)
{

}

void RenderView::focusInEvent(QFocusEvent* event)
{
	//if (GetFocus() != hwnd || VulkanApp::GetFocusForm() != _mainRenderer)
	{
		//SetFocus(nullptr);
		VulkanApp::SetFocusForm(_mainRenderer);
		SetFocus(hwnd);
	}
}

void RenderView::focusOutEvent(QFocusEvent* event)
{

}

void RenderView::mousePressEvent(QMouseEvent* event)
{
	VulkanApp::SetFocusForm(_mainRenderer);
	SetFocus(hwnd);
}

void RenderView::mouseReleaseEvent(QMouseEvent* event)
{

}