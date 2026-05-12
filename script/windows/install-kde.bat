set /P qt_version=<"%CD%\script\qt.version"

set /P qt_arch=<"%CD%\script\qt.arch.win"

set "current_dir=%CD%"

set "cwd=%current_dir:\=/%"

echo %cwd%

set PATH=%PATH%;%cwd%/Qt/%qt_version%/%qt_arch%/bin

set PKG_CONFIG_PATH=%cwd%/../builds/conan

conan install . -of ../builds/conan -s build_type=Release --build=missing --deployer=runtime_deploy

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

cmake -S 3rdparty/KDE/extra-cmake-modules -B ../builds/build-extra-cmake-modules -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DBUILD_HTML_DOCS=OFF -DBUILD_MAN_DOCS=OFF -DCMAKE_INSTALL_PREFIX=../KDE -DCMAKE_PREFIX_PATH="%cwd%/Qt/%qt_version%/%qt_arch%;%cwd%/../KDE;%cwd%/../builds/conan" -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../builds/build-extra-cmake-modules --config Release -j 3

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../builds/build-extra-cmake-modules --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake -S 3rdparty/KDE/syntax-highlighting -B ../builds/build-syntax-highlighting -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=%cwd%/../KDE -DECM_DIR=%cwd%/../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH=%cwd%/Qt/%qt_version%/%qt_arch% -G "NMake Makefiles"

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

cmake -S 3rdparty/KDE/sonnet -B ../builds/build-sonnet -DCMAKE_BUILD_TYPE=Release -DCMAKE_MODULE_PATH=%cwd%/../builds/conan -DCMAKE_INSTALL_PREFIX=../KDE -DBUILD_TESTING=OFF -DECM_DIR=../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH="%cwd%/Qt/%qt_version%/%qt_arch%;%cwd%/../KDE;%cwd%/../builds/conan" -G "NMake Makefiles"

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

cmake -S 3rdparty/KDE/kwidgetsaddons -B ../builds/build-kwidgetsaddons -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=../KDE -DECM_DIR=../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH="%cwd%/Qt/%qt_version%/%qt_arch%;%cwd%/../KDE;%cwd%/../builds/conan" -DBUILD_PYTHON_BINDINGS=False -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../builds/build-kwidgetsaddons --config Release -j 3

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../builds/build-kwidgetsaddons --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake -S 3rdparty/KDE/kconfig -B ../builds/build-kconfig -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../KDE -DECM_DIR=../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH="%cwd%/Qt/%qt_version%/%qt_arch%;%cwd%/../KDE;%cwd%/../builds/conan" -DBUILD_TESTING=OFF -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../builds/build-kconfig --config Release -j 3

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../builds/build-kconfig --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake -S 3rdparty/KDE/kcoreaddons -B ../builds/build-kcoreaddons -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../KDE -DECM_DIR=../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH="%cwd%/Qt/%qt_version%/%qt_arch%;%cwd%/../KDE;%cwd%/../builds/conan" -DBUILD_TESTING=OFF -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../builds/build-kcoreaddons --config Release -j 3

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../builds/build-kcoreaddons --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake -S 3rdparty/KDE/ki18n -B ../builds/build-ki18n -DCMAKE_BUILD_TYPE=Release -DCMAKE_MODULE_PATH=%cwd%/../builds/conan -DCMAKE_INSTALL_PREFIX=../KDE -DECM_DIR=../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH="%cwd%/Qt/%qt_version%/%qt_arch%;%cwd%/../KDE;%cwd%/../builds/conan" -DBUILD_TESTING=OFF -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../builds/build-ki18n --config Release -j 3

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../builds/build-ki18n --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake -S 3rdparty/KDE/karchive -B ../builds/build-karchive -DCMAKE_BUILD_TYPE=Release -DOPENSSL_ROOT_DIR=%cwd%/Qt/Tools/OpenSSLv3/Win_x64 -DCMAKE_MODULE_PATH=%cwd%/../builds/conan -DCMAKE_INSTALL_PREFIX=../KDE -DECM_DIR=../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH="%cwd%/Qt/%qt_version%/%qt_arch%;%cwd%/../KDE;%cwd%/../builds/conan" -DBUILD_TESTING=OFF -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../builds/build-karchive --config Release -j 3

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../builds/build-karchive --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake -S 3rdparty/KDE/kservice -B ../builds/build-kservice -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../KDE -DECM_DIR=../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH="%cwd%/Qt/%qt_version%/%qt_arch%;%cwd%/../KDE;%cwd%/../builds/conan" -DBUILD_TESTING=OFF -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../builds/build-kservice --config Release -j 3

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../builds/build-kservice --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake -S 3rdparty/KDE/solid -B ../builds/build-solid -DCMAKE_BUILD_TYPE=Release -DCMAKE_MODULE_PATH="%cwd%/../builds/conan;%cwd%/win_cmake" -DCMAKE_INSTALL_PREFIX=../KDE -DECM_DIR=../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH="%cwd%/Qt/%qt_version%/%qt_arch%;%cwd%/../KDE;%cwd%/../builds/conan" -DBUILD_TESTING=OFF -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../builds/build-solid --config Release -j 3

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../builds/build-solid --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake -S 3rdparty/KDE/kcrash -B ../builds/build-kcrash -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../KDE -DECM_DIR=../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH="%cwd%/Qt/%qt_version%/%qt_arch%;%cwd%/../KDE;%cwd%/../builds/conan" -DBUILD_TESTING=OFF -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../builds/build-kcrash --config Release -j 3

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../builds/build-kcrash --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake -S 3rdparty/KDE/kio -B ../builds/build-kio -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../KDE -DECM_DIR=../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH="%cwd%/Qt/%qt_version%/%qt_arch%;%cwd%/../KDE;%cwd%/../builds/conan" -DBUILD_TESTING=OFF -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../builds/build-kio --config Release -j 3

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../builds/build-kio --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake -S 3rdparty/KDE/kcmutils -B ../builds/build-kcmutils -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../KDE -DECM_DIR=../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH="%cwd%/Qt/%qt_version%/%qt_arch%;%cwd%/../KDE;%cwd%/../builds/conan" -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../builds/build-kcmutils --config Release -j 3

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../builds/build-kcmutils --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake -S 3rdparty/KDE/kdecoration -B ../builds/build-kdecoration -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../KDE -DECM_DIR=../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH="%cwd%/Qt/%qt_version%/%qt_arch%;%cwd%/../KDE;%cwd%/../builds/conan" -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../builds/build-kdecoration --config Release -j 3

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../builds/build-kdecoration --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake -S 3rdparty/KDE/breeze-icons -B ../builds/build-breeze-icons -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../KDE -DECM_DIR=../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH="%cwd%/Qt/%qt_version%/%qt_arch%;%cwd%/../KDE;%cwd%/../builds/conan" -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../builds/build-breeze-icons --config Release -j 3

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../builds/build-breeze-icons --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake -S 3rdparty/KDE/breeze -B ../builds/build-breeze -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../KDE -DECM_DIR=../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH="%cwd%/Qt/%qt_version%/%qt_arch%;%cwd%/../KDE;%cwd%/../builds/conan" -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../builds/build-breeze --config Release -j 3

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../builds/build-breeze --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)
