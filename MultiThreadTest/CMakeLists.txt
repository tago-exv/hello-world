# CMakeList.txt : RopeContinuousMeasure の CMake プロジェクト。ソースを含めて、次を定義します:
# プロジェクト専用ロジックはこちらです。
#
set(TARGET_NAME MultiThresdTest)

# ソースをこのプロジェクトの実行可能ファイルに追加します。

add_executable (MultiThreadTest
    "MultiThreadTest.cpp")

# common path
set(PREPROCESS_BINARY_DIR   "${CMAKE_BINARY_DIR}/${TARGET_NAME}/$(Configuration)")

# include path
include_directories(${TARGET_NAME} ${COMMON_INC})

# link library path
#target_link_libraries(${TARGET_NAME} PRIVATE ExCamera ExBaslerIf ExImage ExBitmap)

#configure_current_directory(${TARGET_NAME})

# TODO: テストを追加し、必要な場合は、ターゲットをインストールします。
