# - комментарии из https://urho3d.io/documentation/HEAD/_using_library.html
#### - мои комментарии

#### Можно не использовать переменные окружения, а указать путь к скомпилированному движку в самом скрипте
set (ENV{URHO3D_HOME} "${CMAKE_SOURCE_DIR}/../engine/build_vs17")

# Set CMake minimum version
cmake_minimum_required (VERSION 3.10.2)
 
# Set project name
project (U3DBFG)
 
#### Удобно не копировать папку cmake в директорию с исходниками проекта, а просто указать путь к ней
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../engine/repo/cmake/Modules")
 
# Include UrhoCommon.cmake module after setting project name
include (UrhoCommon)
 
# Define target name
set (TARGET_NAME bfg)
 
# Define source files
define_source_files ()

#### Папки с ресурсами
define_resource_dirs (GLOB_PATTERNS ${CMAKE_SOURCE_DIR}/AppData
                                    ${CMAKE_SOURCE_DIR}/../engine/repo/bin/CoreData
                                    ${CMAKE_SOURCE_DIR}/../engine/repo/bin/Data)
 
# Setup target with resource copying
setup_main_executable ()

#### Назначаем проект приложения стартовым
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${TARGET_NAME})

#### Заставляем VS отображать дерево каталогов
source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
