#include "EditorMain.h"
#include "RenderView.h"
#include "VulkanManager.h"
#include "ShaderCompiler.h"
#include "Shader.h"
#include "FileSystem.h"

EditorMain::EditorMain(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    setFocusPolicy(Qt::ClickFocus);
    if (VulkanManager::GetManager() == NULL)
    {
        VulkanManager::InitManager(true);
        Shaderc::ShaderCompiler::CompileAllShaders(FileSystem::GetShaderIncludeAbsPath().c_str());
        Shader::LoadShaderCache(FileSystem::GetShaderCacheAbsPath().c_str());
    }

    //Main render view
    //_mainRenderView = new RenderView(NULL); 
    //_mainRenderView->show();
    _mainRenderView = new RenderView(this); 
    _mainRenderView->show();
    setCentralWidget(_mainRenderView);
    centralWidget()->setFocusPolicy(Qt::ClickFocus);
}

EditorMain::~EditorMain()
{

}

void EditorMain::closeEvent(QCloseEvent* event)
{
    _mainRenderView->close();
    if (VulkanManager::GetManager())
    {
        Shader::DestroyAllShaderCache();
        VulkanManager::GetManager()->ReleaseManager();
    }
}

void EditorMain::focusInEvent(QFocusEvent* event)
{
}

void EditorMain::focusOutEvent(QFocusEvent* event)
{
}

void EditorMain::mousePressEvent(QMouseEvent* event)
{
}

void EditorMain::mouseReleaseEvent(QMouseEvent* event)
{
}

bool EditorMain::eventFilter(QObject* watched, QEvent* event)
{
    return QMainWindow::eventFilter(watched, event);
}
