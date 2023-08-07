#include "EditorMain.h"
#include "RenderView.h"

EditorMain::EditorMain(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    setFocusPolicy(Qt::ClickFocus);


    _mainRenderView = new RenderView(this);
    setCentralWidget(_mainRenderView);
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
    _mainRenderView->close();
}

void EditorMain::resizeEvent(QResizeEvent* event)
{
}
