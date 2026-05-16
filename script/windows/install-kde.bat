set /P qt_version=<"%CD%\script\qt.version"

set /P qt_arch=<"%CD%\script\qt.arch.win"

set "current_dir=%CD%"

set "cwd=%current_dir:\=/%"

set "PATH=%PATH%;%cwd%/../Qt/%qt_version%/%qt_arch%/bin"

set "PKG_CONFIG_PATH=%cwd%/../builds/conan"

conan install . -of ../builds/conan -s build_type=Release --build=missing --deployer=runtime_deploy -c tools.cmake.cmaketoolchain:generator="NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

call "..\builds\conan\conanbuild.bat"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

@echo on

call "..\builds\conan\conanrun.bat"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

@echo on

for /f "tokens=*" %%i in ('pkg-config.exe --variable=prefix libgettext') do (set gettext_prefix=%%i & goto :next)

:next

set "INCLUDE=%INCLUDE%;%gettext_prefix%/include"

set "LIB=%LIB%;%gettext_prefix%/lib"

cmake -S 3rdparty/KDE/syntax-highlighting -B ../builds/build-syntax-highlighting -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=%cwd%/../KDE -DECM_DIR=%cwd%/../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH=%cwd%/../Qt/%qt_version%/%qt_arch% -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../builds/build-syntax-highlighting --config Release -j 3

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../builds/build-syntax-highlighting --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake -S 3rdparty/KDE/sonnet -B ../builds/build-sonnet -DCMAKE_BUILD_TYPE=Release -DCMAKE_MODULE_PATH=%cwd%/../builds/conan -DCMAKE_INSTALL_PREFIX=../KDE -DBUILD_TESTING=OFF -DECM_DIR=../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH="%cwd%/../Qt/%qt_version%/%qt_arch%;%cwd%/../KDE;%cwd%/../builds/conan" -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../builds/build-sonnet --config Release -j 3

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../builds/build-sonnet --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)
