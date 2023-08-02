# HBBr
Hbb Renderer.
这是一个完全基于Vulkan API所编写的渲染器。  
(做着玩的,希望未来有用武之地)  
   
编译前提(需要额外下载准备):    
1.使用Visual studio开发，build tools是142版本的，也就是你的Visual studio必须要是2019以后的版本。   
2.需要另行下载 Vulkan SDK ,版本为 1.3.250.1 , 而且安装的时候请务必包含GLM数学库。    
3.编辑器GUI使用的是QT5.12.12,不使用QML,因为窗口嵌入有点麻烦。   
  
补充说明:  
1.渲染器会内置Imgui作为辅助UI绘制  
2.编辑器使用了assimp作为资产的导入导出  
3.使用pugixml辅助外部配置  
   
最终打包的独立渲染器:          
1.Imgui,pugixml保留                
2.不会带有QT的内容           
3.不会带有assimp 
            
目前有3个配置可选：     
1.Debug             （Editor Debug，启动项：Editor）            
2.Release           （Editor Release，启动项：Editor）              
3.ReleaseGame_GLFW  （Renderer Only with GLFW,启动项：RendererCore）             

![PreviewImage](/Preview.png) 
![PreviewImageCube](/PreviewCube.png) 
![PreviewImageCube](/PreviewFBX.png) 
