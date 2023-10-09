rem 关闭回显
@echo off
rem 清除屏幕
cls
::==========================================================================
::本脚本用来在Windows环境使用Cmake交叉编译Android使用的assimp-5.0.1库
::==========================================================================

title cmake build assimp-5.0.1

chcp 65001>nul

echo.
echo.
echo.--------本脚本用来用来在Windows环境使用Cmake交叉编译Android使用的库------
echo.

echo CMD Path:%cd%
echo 当前 dir:%~dp0

rem 切换到当前目录
cd /d %~dp0

:: --------------------------------------------------------------------------
rem 临时变量
rem 需要安装ninja.exe，放到cmake的目录下
set ANDROID_NDK=C:/NVPACK/android-sdk-windows/ndk/21.4.7075529
set CMAKE_PATH=C:/NVPACK/CMake
set ANDROID_VERSION=21
set SOURCE_DIR=E:/assimp-master
set NATIVE_BUILD_DIR=.
set INSTALL_DIR=%~dp0

set ANDROID_ARMV5_CFLAGS="-march=armv5te"
rem -mfpu=vfpv3-d16  -fexceptions -frtti
set ANDROID_ARMV7_CFLAGS="-march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -mfpu=neon"
rem -mfloat-abi=softfp -mfpu=neon -fexceptions -frtti
set ANDROID_ARMV8_CFLAGS="-march=armv8-a"   
set ANDROID_X86_CFLAGS="-march=i686 -mtune=intel -mssse3 -mfpmath=sse -m32"
set ANDROID_X86_64_CFLAGS="-march=x86-64 -msse4.2 -mpopcnt -m64 -mtune=intel"

:: --------------------------------------------------------------------------
echo.
echo.--------------------------开始build--------------------------------------
echo.

rem call:label表示调用用":"标识,以goto:eof结束的函数代码段

rem --make armeabi
@REM call:buildAssimp armeabi %ANDROID_ARMV5_CFLAGS%

rem --make armeabi-v7a
rem call:buildAssimp armeabi-v7a %ANDROID_ARMV7_CFLAGS%

rem --make arm64-v8a
call:buildAssimp arm64-v8a %ANDROID_ARMV8_CFLAGS%

rem --make x86
rem call:buildAssimp x86 %ANDROID_X86_CFLAGS%

rem --make x86_64
rem call:buildAssimp x86_64 %ANDROID_X86_64_CFLAGS%

set timeout=5
echo.倒计时 %timeout% 秒钟后自动退出
for /L %%a in (%timeout% -1 1) do ( ^
    set /p=%%a . <nul&ping -n 2 127.0.0.1>nul
)

echo.
echo.&pause&goto:eof

:: --------------------------------------------------------------------------
rem 定义函数 (%1:arch_abi,%2:cflags)
:buildAssimp
setlocal

    echo.[func]-----------------start make [%1]---------------------------
    echo.
    
    ::注意这里我们使用  %~1 引用参数是为了脱去变量赋值时的 边界 双引号""
    set ABI=%~1
    set CFLAGS=%~2
    set BINARY_DIR="%NATIVE_BUILD_DIR%/bin/%ABI%"
    set PLATFORM_VERSION=%ANDROID_VERSION%
    set LIB_OUTPUT_DIR="%INSTALL_DIR%/dist/%ABI%"
    
    rem 批处理中if else 需在一行，() 括号内容表示在"("开始的同一行
    if %ABI%==armeabi (
        echo.[func]----当前编译--^>armeabi
        goto pass
    )
    if %ABI%==armeabi-v7a (
        echo.[func]----当前编译--^>armeabi-v7a
        goto pass
    )
    if %ABI%==arm64-v8a (
        echo.[func]----当前编译--^>arm64-v8a
        goto pass
    )
    if %ABI%==x86 (
        echo.[func]----当前编译--^>x86
        goto pass
    )
    if %ABI%==x86_64 (
        echo.[func]----当前编译--^>x86_64
        goto pass
    ) else (
        echo.[ERROR]----不支持的ABI---^>%ABI%
        pause>nul
        exit
    )
    
    :pass
    
    echo [func]----PLATFORM_VERSION==%ANDROID_VERSION%
    echo [func]----ABI==%ABI%
    echo [func]----CFLAGS==%CFLAGS%
    echo [func]----SOURCE_DIR==%SOURCE_DIR%
    echo [func]----BINARY_DIR==%BINARY_DIR%
    echo [func]----LIB_OUTPUT_DIR==%LIB_OUTPUT_DIR%
    echo.
    echo.
    
    if not exist %BINARY_DIR% md %BINARY_DIR%
    
    cd /d %BINARY_DIR%
    
    :: -G"MinGW Makefiles"
    :: -DCMAKE_MAKE_PROGRAM=%ANDROID_NDK%/prebuilt/windows-x86_64/bin/make.exe
    
    rem 开源库有提供cmake         Android Gradle - Ninja ^ 
        cmake ^
        -DANDROID_ABI=%ABI% ^
        -DANDROID_PLATFORM=android-%PLATFORM_VERSION% ^
        -DCMAKE_INSTALL_PREFIX=%LIB_OUTPUT_DIR% ^
        -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=%LIB_OUTPUT_DIR%/build ^
        -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=%LIB_OUTPUT_DIR%/build ^
        -DCMAKE_BUILD_TYPE=Release ^
        -DANDROID_NDK=%ANDROID_NDK% ^
        -DCMAKE_TOOLCHAIN_FILE=%ANDROID_NDK%/build/cmake/android.toolchain.cmake ^
        -DCMAKE_MAKE_PROGRAM=%CMAKE_PATH%/bin/ninja.exe ^
        -G"Ninja" ^
        -DANDROID_ARM_NEON=TRUE ^
        -DANDROID_TOOLCHAIN=clang ^
        -DANDROID_STL=c++_shared ^
        -DCMAKE_C_FLAGS="%CFLAGS% -Os -Wall -pipe -fPIC" ^
        -DASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT=FALSE ^
        -DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=FALSE ^
        -DASSIMP_BUILD_OBJ_IMPORTER=FALSE ^
        -DASSIMP_BUILD_FBX_IMPORTER=TRUE ^
        -DASSIMP_BUILD_SAMPLES=OFF ^
        -DASSIMP_BUILD_TESTS=OFF ^
        -DASSIMP_NO_EXPORT=ON ^
        -DASSIMP_BUILD_ZLIB=ON ^
        -DASSIMP_BUILD_ASSIMP_TOOLS=OFF ^
        -DASSIMP_ANDROID_JNIIOSYSTEM=ON ^
        -DINJECT_DEBUG_POSTFIX=OFF ^
         %SOURCE_DIR%
        
    %CMAKE_PATH%/bin/cmake --build ./
    rem ninja -C .
    rem 需要Ninja install 可以参考《在MSYS2中的脚本》
    @REM %CMAKE_PATH%/bin/ninja clean
    @REM %CMAKE_PATH%/bin/ninja
    ::%CMAKE_PATH%/bin/ninja install
    cd ../../
            
    echo.[func]-----------------[%1] make end---------------------------
    echo.

endlocal
goto:eof