#include "EditorMain.h"
#include "RenderView.h"
#include "SceneOutline.h"
#include "ContentBrowser.h"
#include "Inspector.h"
#include "FormMain.h"
#include "EditorCommonFunction.h"
#include <QDesktopServices>
#include <QUrl>
#include <qdir.h>
#include <QFileDialog>
#include <qfileinfo.h>
#include "LineEditDialog.h"
#include "DirtyAssetsManager.h"
#include "RendererCore/Core/VulkanRenderer.h"
#include "RendererCore/Core/Asset/World.h"
#include "QFontDatabase.h"
#include "ConsoleDebug.h"

EditorMain* EditorMain::_self = nullptr;

CustomDockWidget::CustomDockWidget(QWidget* parent) :QDockWidget(parent)
{

}

EditorMain::EditorMain(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    setFocusPolicy(Qt::ClickFocus);

    _mainRenderView = new RenderView(this);
    setCentralWidget(_mainRenderView);

    _sceneOutline = new SceneOutline(_mainRenderView->_mainRenderer->renderer, this);
    _sceneOutline_dock = new CustomDockWidget(this);
    _sceneOutline_dock->setFeatures(
        QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    _sceneOutline_dock->setWidget(_sceneOutline);
    _sceneOutline_dock->setWindowTitle("Scene Outline");
    _sceneOutline->setObjectName("SceneOutline");
    addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, _sceneOutline_dock);

    _contentBrowser = new ContentBrowser(this);
    _contentBrowser_dock = new CustomDockWidget(this);
    _contentBrowser_dock->setFeatures(
        QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    _contentBrowser_dock->setWidget(_contentBrowser);
    _contentBrowser_dock->setWindowTitle("Content Browser");
    _contentBrowser->setObjectName("ContentBrowser");
    addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, _contentBrowser_dock);

    _inspector = new Inspector(this);
    _inspector_dock = new CustomDockWidget(this);
    _inspector_dock->setFeatures(
        QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    _inspector_dock->setWidget(_inspector);
    _inspector_dock->setWindowTitle("Inspector");
    _inspector->setObjectName("Inspector");
    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, _inspector_dock);

    setObjectName("EditorMain");
    //setStyleSheet(GetWidgetStyleSheetFromFile("EditorMain"));

    connect(ui.ResetWindowStyle, &QAction::triggered, this, [this](bool bChecked) {
        setStyleSheet(GetWidgetStyleSheetFromFile("EditorMain"));
    });
    connect(ui.OpenProgramDir, &QAction::triggered, this, [this](bool bChecked) {
        //QDir dir(FileSystem::GetProgramPath().c_str());
        QFileInfo info(QDir::toNativeSeparators(FileSystem::GetProgramPath().c_str()));
        if (info.isDir())
        {
            QDesktopServices::openUrl(QUrl(info.absoluteFilePath()));
        }
    });
    connect(ui.CompileAllShader, &QAction::triggered, this, [this](bool bChecked) {
        VulkanApp::RecompileAllShader();
    });
    connect(ui.SaveWorld, &QAction::triggered, this, [this](bool bChecked) {
        VulkanApp::GetMainForm()->renderer->GetWorld()->SaveWorld();
     });
    connect(ui.SaveAsWorld, &QAction::triggered, this, [this](bool bChecked) {
        LineEditDialog* dialog = new LineEditDialog("New World Name",this);
        dialog->EnterCallBack = [dialog]() {
            if (dialog->ui.lineEdit->text().size() > 1)
            {
                VulkanApp::GetMainForm()->renderer->GetWorld()->SaveWorld(dialog->ui.lineEdit->text().toStdString().c_str());
            }
        };       
    });

    EditorMain::_self = this;

    //Update timer //编辑器缩放窗口的时候，渲染器会不停闪黑，
    //这是因为窗口缩放和渲染更新(Update)不同步导致，窗口已经缩放但是画面未及时重新绘制导致一瞬间发黑
    //哪天能找到QT5消息循环的函数重写应该就能解决这个问题了。
    _renderTimer = new QTimer(this);
    _renderTimer->setInterval(1);
    connect(_renderTimer, SIGNAL(timeout()), this, SLOT(UpdateRender()));
    _renderTimer->start();

}

EditorMain::~EditorMain()
{
}

void EditorMain::showEvent(QShowEvent* event)
{
	
}

void EditorMain::focusInEvent(QFocusEvent* event)
{
}

void EditorMain::focusOutEvent(QFocusEvent* event)
{
}

void EditorMain::UpdateRender()
{
    if (_inspector)
        _inspector->PropertyUpdate();
    if (_mainRenderView)
        _mainRenderView->Update();
}

void EditorMain::ShowDirtyAssetsManager()
{
    if (ContentManager::Get()->GetDirtyAssets().size() + World::GetDirtyWorlds().size() + Level::GetDirtyLevels().size() > 0)
    {
        auto dirtyAssetsManager = new DirtyAssetsManager(this);
        //if (ContentManager::Get()->GetDirtyAssets().size > 0)
        {
            dirtyAssetsManager->exec();
        }
    }  
}

void EditorMain::closeEvent(QCloseEvent* event)
{
    _renderTimer->stop();
    if (_inspector)
        _inspector->close();
    if(_sceneOutline)
        _sceneOutline->close();
    if(_mainRenderView)
        _mainRenderView->close();
}

void EditorMain::resizeEvent(QResizeEvent* event)
{

}
