echo "Building markdown-tools..."

mkdir build-markdown-tools

qt_version=$(cat $PWD/script/qt.version)

build_type=$1

if [ -z "$1" ] then
    build_type="Release"
fi

build_opts=""

if [ $build_type -eq "Debug" ] then
    build_opts="-DENABLE_COVERAGE=ON"
fi

cmake $build_opts -DCMAKE_BUILD_TYPE=$build_type -S . -B build-markdown-tools -DBUILD_MDPDF_TESTS=OFF -DCMAKE_PREFIX_PATH=$PWD/Qt/$qt_version/gcc_64 -DECM_DIR=$PWD/../KDE/share/ECM/cmake -DKF6SyntaxHighlighting_DIR=$PWD/../KDE/lib/x86_64-linux-gnu/cmake/KF6SyntaxHighlighting || exit 1

cmake --build build-markdown-tools --config Release || exit 1
