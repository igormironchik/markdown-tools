mkdir ..\build-extra-cmake-modules
cmake -S 3rdparty\extra-cmake-modules -B ..\build-extra-cmake-modules -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DBUILD_HTML_DOCS=OFF -DBUILD_MAN_DOCS=OFF -DCMAKE_INSTALL_PREFIX=..\ecm
cmake --build ..\build-extra-cmake-modules --config Release
cmake --install ..\build-extra-cmake-modules --prefix ..\ecm
