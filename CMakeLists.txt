cmake_minimum_required(VERSION 3.25.0)

set(PROJECT_NAME "fanimeserverwithd2d")

project(${PROJECT_NAME} VERSION 1.0)

# Boost
set(Boost_ROOT "C:/Users/SonnyCalcr/scoop/apps/boost/1.87.0")
set (Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS locale json)
include_directories(${Boost_INCLUDE_DIRS})

# fmt, spdlog and sqlite3
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_definitions(
    /D_UNICODE=1
    /DUNICODE=1
)

# add_definitions(
#     /DFANY_DEBUG=1
# )

include_directories("./src/")

set(MY_EXECUTABLE_NAME "fanimeserverwithd2d")

set(SRC_FILES
    "./src/defines/defines.h"
    "./src/ipc/event_listener.cpp"
    "./src/ipc/event_listener.h"
    "./src/ipc/ipc.cpp"
    "./src/ipc/ipc.h"
    "./src/log/fanylog.cpp"
    "./src/log/fanylog.h"
    "./src/utils/window_utils.cpp"
    "./src/utils/window_utils.h"
    "./src/utils/common_utils.cpp"
    "./src/utils/common_utils.h"
    "./src/window/candidate_window.cpp"
    "./src/window/candidate_window.h"
    "./src/main.cpp"
)

set(SRC_FILES_IME_ENGINE
    "./src/ime_engine/shuangpin/dictionary.cpp"
    "./src/ime_engine/shuangpin/pinyin_utils.cpp"
)

set(SRC_FILES_GOOGLEIME
    "./src/ime_engine/googlepinyinime-rev/src/share/dictbuilder.cpp"
    "./src/ime_engine/googlepinyinime-rev/src/share/dictlist.cpp"
    "./src/ime_engine/googlepinyinime-rev/src/share/dicttrie.cpp"
    "./src/ime_engine/googlepinyinime-rev/src/share/lpicache.cpp"
    "./src/ime_engine/googlepinyinime-rev/src/share/matrixsearch.cpp"
    "./src/ime_engine/googlepinyinime-rev/src/share/mystdlib.cpp"
    "./src/ime_engine/googlepinyinime-rev/src/share/ngram.cpp"
    "./src/ime_engine/googlepinyinime-rev/src/share/pinyinime.cpp"
    "./src/ime_engine/googlepinyinime-rev/src/share/searchutility.cpp"
    "./src/ime_engine/googlepinyinime-rev/src/share/spellingtable.cpp"
    "./src/ime_engine/googlepinyinime-rev/src/share/spellingtrie.cpp"
    "./src/ime_engine/googlepinyinime-rev/src/share/splparser.cpp"
    "./src/ime_engine/googlepinyinime-rev/src/share/sync.cpp"
    "./src/ime_engine/googlepinyinime-rev/src/share/userdict.cpp"
    "./src/ime_engine/googlepinyinime-rev/src/share/utf16char.cpp"
    "./src/ime_engine/googlepinyinime-rev/src/share/utf16reader.cpp"
)

add_executable(${MY_EXECUTABLE_NAME} WIN32
    ${SRC_FILES}
    ${SRC_FILES_IME_ENGINE}
    ${SRC_FILES_GOOGLEIME}
)

target_link_libraries(${PROJECT_NAME} PRIVATE unofficial::sqlite3::sqlite3 fmt::fmt spdlog::spdlog Boost::locale Boost::json)