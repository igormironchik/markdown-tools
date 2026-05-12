set /P qt_version=<%CD%\script\qt.version

set /P qt_arch=<%CD%\script\qt.arch.win

cmake -S 3rdparty/KDE/extra-cmake-modules -B ../build-extra-cmake-modules -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DBUILD_HTML_DOCS=OFF -DBUILD_MAN_DOCS=OFF -DCMAKE_INSTALL_PREFIX=../KDE -DCMAKE_PREFIX_PATH=%CD%/Qt/%qt_version%/%qt_arch% -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../build-extra-cmake-modules --config Release

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../build-extra-cmake-modules --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake -S 3rdparty/KDE/syntax-highlighting -B ../build-syntax-highlighting -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=%CD%/../KDE -DECM_DIR=%CD%/../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH=%CD%/Qt/%qt_version%/%qt_arch% -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../build-syntax-highlighting --config Release

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../build-syntax-highlighting --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

conan install . -of ../build-sonnet -s build_type=Release --build=missing --deployer=runtime_deploy

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

set PKG_CONFIG_PATH=%CD%/../build-sonnet

cmake -S 3rdparty/KDE/sonnet -B ../build-sonnet -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../KDE -DBUILD_TESTING=OFF -DECM_DIR=../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH=%CD%/Qt/%qt_version%/%qt_arch% -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../build-sonnet --config Release

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../build-sonnet --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake -S 3rdparty/KDE/kwidgetsaddons -B ../build-kwidgetsaddons -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=../KDE -DECM_DIR=../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH=%CD%/Qt/%qt_version%/%qt_arch% -DBUILD_PYTHON_BINDINGS=False -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../build-kwidgetsaddons --config Release

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../build-kwidgetsaddons --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake -S 3rdparty/KDE/kconfig -B ../build-kconfig -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../KDE -DECM_DIR=../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH=%CD%/Qt/%qt_version%/%qt_arch% -DBUILD_TESTING=OFF -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../build-kconfig --config Release

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../build-kconfig --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake -S 3rdparty/KDE/kcoreaddons -B ../build-kcoreaddons -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../KDE -DECM_DIR=../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH=%CD%/Qt/%qt_version%/%qt_arch% -DBUILD_TESTING=OFF -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../build-kcoreaddons --config Release

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../build-kcoreaddons --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

conan install . -of ../build-ki18n -s build_type=Release --build=missing --deployer=runtime_deploy

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

set PATH=%PATH%;../build-ki18n

cmake -S 3rdparty/KDE/ki18n -B ../build-ki18n -DCMAKE_BUILD_TYPE=Release -DCMAKE_MODULE_PATH=../build-ki18n -DCMAKE_INSTALL_PREFIX=../KDE -DECM_DIR=../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH=%CD%/Qt/%qt_version%/%qt_arch% -DBUILD_TESTING=OFF -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../build-ki18n --config Release

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../build-ki18n --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake -S 3rdparty/KDE/kio -B ../build-kio -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../KDE -DECM_DIR=../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH=%CD%/Qt/%qt_version%/%qt_arch% -DBUILD_TESTING=OFF -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../build-kio --config Release

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../build-kio --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake -S 3rdparty/KDE/kcmutils -B ../build-kcmutils -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../KDE -DECM_DIR=../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH=%CD%/Qt/%qt_version%/%qt_arch% -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../build-kcmutils --config Release

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../build-kcmutils --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake -S 3rdparty/KDE/kdecoration -B ../build-kdecoration -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../KDE -DECM_DIR=../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH=%CD%/Qt/%qt_version%/%qt_arch% -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../build-kdecoration --config Release

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../build-kdecoration --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake -S 3rdparty/KDE/breeze-icons -B ../build-breeze-icons -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../KDE -DECM_DIR=../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH=%CD%/Qt/%qt_version%/%qt_arch% -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../build-breeze-icons --config Release

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../build-breeze-icons --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake -S 3rdparty/KDE/breeze -B ../build-breeze -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../KDE -DECM_DIR=../KDE/share/ECM/cmake -DCMAKE_PREFIX_PATH=%CD%/Qt/%qt_version%/%qt_arch% -G "NMake Makefiles"

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build ../build-breeze --config Release

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --install ../build-breeze --prefix ../KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)
