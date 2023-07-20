#include "EditorMain.h"
#include "RenderView.h"

EditorMain::EditorMain(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    //Main render view
    _mainRenderView = new RenderView(this);
    setCentralWidget(_mainRenderView);

}

EditorMain::~EditorMain()
{

}
