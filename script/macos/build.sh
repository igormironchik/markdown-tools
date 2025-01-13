echo "Building markdown-tools..."

qt_version=$(cat $PWD/script/qt.version)

mkdir build-markdown-tools

cmake -DCMAKE_BUILD_TYPE=Release -S . -B build-markdown-tools -DBUILD_MDPDF_TESTS=OFF -DCMAKE_PREFIX_PATH="$PWD/Qt/$qt_version/macos;`brew --prefix`" -DECM_DIR=$PWD/../KDE/share/ECM/cmake -DKF6SyntaxHighlighting_DIR=$PWD/../KDE/lib/cmake/KF6SyntaxHighlighting -DKF6Sonnet_DIR=$PWD/../KDE/lib/cmake/KF6Sonnet -DFontconfig_INCLUDE_DIR=`brew --prefix fontconfig`/include -DOPENSSL_ROOT_DIR=`brew --prefix openssl@3` || exit 1

cmake --build build-markdown-tools --config Release || exit 1
