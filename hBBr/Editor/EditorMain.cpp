#include "EditorMain.h"
#include "RenderView.h"
#include "SceneOutline.h"
#include "FormMain.h"
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
    addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, _sceneOutline_dock);

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
    _sceneOutline->close();
    _mainRenderView->close();

}

void EditorMain::resizeEvent(QResizeEvent* event)
{
}
