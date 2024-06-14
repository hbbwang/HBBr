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

# 关于第三方引用，在此处说明一下：
## 1.assimp        
这个库不用说，用来作为模型文件导入工具实在是太方便了，强烈推荐。
https://github.com/assimp/assimp
## 2.crossguid     
guid库，简单易用，用作唯一识别。
https://github.com/graeme-hill/crossguid
## 3.Imgui         
渲染器GUI库，著名程度不用多说，大公司做编辑器也会用。
https://github.com/ocornut/imgui
## 4.lodepng       
png库，使用简单便捷，不过后面改用英伟达的nvtt了，基本不怎么用到。
https://github.com/lvandeve/lodepng
## 5.nlohmann      
主要是用它的nlohmann/json库，这个json库主要实现序列化数据储存的功能，推荐。
https://github.com/nlohmann/json
## 6.nvtt          
Nvidia Texture Tools，主要是为了实现图像导入后转为dds的功能，用法也简单。
https://developer.nvidia.com/gpu-accelerated-texture-compression
## 7.pugixml       
也是大家常用的xml库，也非常好用，用来存一些数据。不过大概率后面都会换成json了，个人觉得便捷程度不如json。
https://github.com/zeux/pugixml
## 8.sdl3          
sdl跨平台窗口库，版本是2，官方定义的3，不过里面部分代码已经不适用2了...不用glfw的原因是它不支持安卓平台。
https://github.com/libsdl-org/SDL
## 9.shaderc       
shader编译库，其实vulkan sdk里也自带了这个，不用另外下的。
https://github.com/google/shaderc
## 10.stb          
stb库，主要用到里面stb_truetype，用来实现字体转纹理，给UI用的。
https://github.com/nothings/stb/?login=from_csdn
## 11.vld          
用来查内存泄漏的
https://github.com/KindDragon/vld
## 12.vulkan      
把vulkan sdk里的头文件搬过来了，已经忘记为什么要这么干了...好像没啥必要,需要注意的是IOS和安卓的sdk是要另外下的。
https://vulkan.lunarg.com/
