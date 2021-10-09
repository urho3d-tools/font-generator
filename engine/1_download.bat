:: Меняем кодировку консоли на UTF-8
chcp 65001

:: Путь к git.exe
set "PATH=c:\Program Files\Git\bin"

:: Качаем репозиторий в папку repo
git clone https://github.com/Urho3D/Urho3D repo

:: Переходим в папку со скачанными исходниками
cd repo

:: Возвращаем состояние репозитория к определённой версии (9 октября 2021)
git reset --hard 2911d262316824e4d3c82e71ba2a7c0440e3c832

:: Ждём нажатие клавиши ENTER перед закрытием консоли
pause
