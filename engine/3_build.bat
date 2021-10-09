:: Меняем кодировку консоли на UTF-8
chcp 65001

:: Путь к cmake.exe
set "PATH=c:\Programs\CMake\bin\"

:: Компилируем
cmake --build build_vs17 --config Release
cmake --build build_vs17 --config Debug

:: Ждём нажатие клавиши ENTER перед закрытием консоли
pause
