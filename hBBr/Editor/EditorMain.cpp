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
#include "RendererCore/Core/VulkanRenderer.h"
#include "RendererCore/Core/Asset/World.h"
EditorMain* EditorMain::_self = nullptr;

EditorMain::EditorMain(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    setFocusPolicy(Qt::ClickFocus);

    _mainRenderView = new RenderView(this);
    setCentralWidget(_mainRenderView);

    _sceneOutline = new SceneOutline(_mainRenderView->_mainRenderer->renderer, this);
    _sceneOutline_dock = new QDockWidget(this);
    _sceneOutline_dock->setWidget(_sceneOutline);
    _sceneOutline_dock->setWindowTitle("Scene Outline");
    _sceneOutline_dock->setObjectName("SceneOutline");
    addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, _sceneOutline_dock);

    _contentBrowser = new ContentBrowser(this);
    _contentBrowser_dock = new QDockWidget(this);
    _contentBrowser_dock->setWidget(_contentBrowser);
    _contentBrowser_dock->setWindowTitle("Content Browser");
    _contentBrowser_dock->setObjectName("ContentBrowser");
    addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, _contentBrowser_dock);

    _inspector = new Inspector(this);
    _inspector_dock = new QDockWidget(this);
    _inspector_dock->setWidget(_inspector);
    _inspector_dock->setWindowTitle("Inspector");
    _inspector_dock->setObjectName("Inspector");
    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, _inspector_dock);

    setObjectName("EditorMain");
    setStyleSheet(GetWidgetStyleSheetFromFile("EditorMain"));

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
        VulkanApp::GetMainForm()->renderer->GetWorld()->SaveWorld();
    });

    EditorMain::_self = this;

    //Update timer
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

