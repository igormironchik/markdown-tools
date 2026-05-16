set /P qt_version=<%CD%\script\qt.version
set /P qt_arch=<%CD%\script\qt.arch.win

set "current_dir=%CD%"

set "cwd=%current_dir:\=/%"

echo "Building markdown-tools..."

cmake -DCMAKE_BUILD_TYPE=Release -S . -B "%cwd%/../builds/build-markdown-tools" -DBUILD_MDPDF_TESTS=OFF -DCMAKE_MODULE_PATH="%cwd%/../builds/conan" -DCMAKE_PREFIX_PATH="%cwd%/../Qt/%qt_version%/%qt_arch%;%cwd%/../KDE;%cwd%/../builds/conan" -DECM_DIR="%cwd%/../KDE/share/ECM/cmake" -DOPENSSL_ROOT_DIR="%cwd%/../Qt/Tools/OpenSSLv3/Win_x64" -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ..\builds\build-markdown-tools --config Release -j 3

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)
