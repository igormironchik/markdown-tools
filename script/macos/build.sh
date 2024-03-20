echo "Building markdown-tools..."

mkdir build-markdown-tools

ls $PWD/../KDE/lib

cmake -DCMAKE_BUILD_TYPE=Release -S . -B build-markdown-tools -DBUILD_MDPDF_TESTS=OFF -DCMAKE_PREFIX_PATH=$PWD/Qt/6.6.2/macos -DECM_DIR=$PWD/../KDE/share/ECM/cmake -DKF6SyntaxHighlighting_DIR=$PWD/../KDE/lib/x86_64-macos-clang/cmake/KF6SyntaxHighlighting || exit 1

cmake --build build-markdown-tools --config Release || exit 1
