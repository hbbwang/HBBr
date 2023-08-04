# HBBr
Hbb Renderer.
这是一个完全基于Vulkan API所编写的渲染器。  
(做着玩的,希望未来有用武之地)   
  
编译须知:    
1.下载Vulkan SDK 1.3 ,且需要下载配套的GLM数学库，建议默认C盘安装，装其他盘也行，确保环境变量里有VULKAN_SDK。        
2.编辑器窗口使用的是QT5.12.12，需要另行下载     
              
目前有3个配置可选：     
1.Debug             （Editor Debug，启动项：Editor）            
2.Release           （Editor Release，启动项：Editor）              
3.ReleaseGame_GLFW  （Renderer Only with GLFW,启动项：RendererCore）             

![PreviewImage](/Preview.png) 
![PreviewImageCube](/PreviewCube.png) 
![PreviewImageCube](/PreviewFBX.png) 
