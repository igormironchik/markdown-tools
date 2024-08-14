cmake -S 3rdparty/KDE/extra-cmake-modules -B ../build-extra-cmake-modules -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DBUILD_HTML_DOCS=OFF -DBUILD_MAN_DOCS=OFF -DCMAKE_INSTALL_PREFIX=../KDE
cmake --build ../build-extra-cmake-modules --config Release
cmake --install ../build-extra-cmake-modules --prefix ../KDE

cmake -S 3rdparty/KDE/syntax-highlighting -B ../build-syntax-highlighting -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%CD%/../KDE -DECM_DIR=%CD%/../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH=%CD%/Qt/6.7.2/msvc2019_64
cmake --build ../build-syntax-highlighting --config Release
cmake --install ../build-syntax-highlighting --prefix ../KDE
