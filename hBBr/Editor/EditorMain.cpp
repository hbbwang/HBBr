#include "EditorMain.h"
#include "RenderView.h"
#include "SceneOutline.h"
#include "ContentBrowser.h"
#include "FormMain.h"
#include "EditorCommonFunction.h"
#include <QDesktopServices>
#include <QUrl>
#include <qdir.h>
#include <qfileinfo.h>
EditorMain* EditorMain::_self = NULL;

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
    EditorMain::_self = this;
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

void EditorMain::closeEvent(QCloseEvent* event)
{
    if(_sceneOutline)
        _sceneOutline->close();
    if(_mainRenderView)
        _mainRenderView->close();
}

void EditorMain::resizeEvent(QResizeEvent* event)
{
}

