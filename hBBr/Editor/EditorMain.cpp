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

    if (VulkanManager::GetManager() == NULL)
    {
        VulkanManager::InitManager(true);
        Shaderc::ShaderCompiler::CompileAllShaders(FileSystem::GetShaderIncludeAbsPath().c_str());
        Shader::LoadShaderCache(FileSystem::GetShaderCacheAbsPath().c_str());
    }

    //Main render view
    _mainRenderView = new RenderView(this);
    setCentralWidget(_mainRenderView);

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
