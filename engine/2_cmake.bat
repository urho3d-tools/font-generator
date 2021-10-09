:: Меняем кодировку консоли на UTF-8
chcp 65001

:: Путь к cmake.exe
set "PATH=c:\Programs\CMake\bin\"

:: Создаём проекты для Visual Studio 2022 в папке build_vs17, используя конфиг CMakeLists.txt из папки repo
cmake repo -B build_vs17 -G "Visual Studio 17" -A Win32^
 -D URHO3D_OPENGL=1 -D URHO3D_STATIC_RUNTIME=1 -D URHO3D_SAMPLES=0 -D URHO3D_LUA=0 -D URHO3D_LUAJIT=0 -D URHO3D_ANGELSCRIPT=0^
 -D URHO3D_TOOLS=0 -D URHO3D_PHYSICS=0 -D URHO3D_NAVIGATION=0

:: Ждём нажатие клавиши ENTER перед закрытием консоли
pause
