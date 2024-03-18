cmake -S 3rdparty/KDE/extra-cmake-modules -B ../build-extra-cmake-modules -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DBUILD_HTML_DOCS=OFF -DBUILD_MAN_DOCS=OFF -DCMAKE_INSTALL_PREFIX=../KDE
cmake --build ../build-extra-cmake-modules --config Release
cmake --install ../build-extra-cmake-modules --prefix ../KDE

cmake -S 3rdparty/KDE/ksyntaxhighlighting -B ../build-syntax-highlighting -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../KDE
cmake --build ../build-syntax-highlighting --config Release
