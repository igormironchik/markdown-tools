set /P qt_version=<%CD%\script\qt.version
set /P qt_arch=<%CD%\script\qt.arch.win
set /P resvg_version=<%CD%\3rdparty\resvg.version

echo "Copying binaries..."

rmdir /S /Q installer\packages\mironchik.igor.markdown\data\bin

rmdir /S /Q installer\packages\mironchik.igor.markdown\data\plugins

rmdir /S /Q installer\packages\mironchik.igor.markdown\data\libexec

rmdir /S /Q installer\packages\mironchik.igor.markdown\data\resources

rmdir /S /Q installer\packages\mironchik.igor.markdown\data\translations

mkdir installer\packages\mironchik.igor.markdown\data\bin

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

copy /Y %CD%\..\builds\build-markdown-tools\bin installer\packages\mironchik.igor.markdown\data\bin

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

del /Q installer\packages\mironchik.igor.markdown\data\bin\cfgfile.generator.exe

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

mkdir installer\packages\mironchik.igor.markdown\data\bin\data

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

xcopy %CD%\..\KDE\bin\data installer\packages\mironchik.igor.markdown\data\bin\data /S /Y

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

mkdir installer\packages\mironchik.igor.markdown\data\bin\data\hunspell

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

xcopy 3rdparty\Windows\hunspell installer\packages\mironchik.igor.markdown\data\bin\data\hunspell /S /Y

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

copy /Y %CD%\..\KDE\bin\kioworker.exe installer\packages\mironchik.igor.markdown\data\bin\kioworker.exe

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

copy /Y %CD%\..\builds\conan\hunspell.dll installer\packages\mironchik.igor.markdown\data\bin\hunspell.dll

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

xcopy %CD%\..\KDE\bin\*.dll installer\packages\mironchik.igor.markdown\data\bin /S /Y

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

del /Q installer\packages\mironchik.igor.markdown\data\bin\test.render.bat

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

copy /Y 3rdparty\resvg-%resvg_version%\target\release\resvg.dll installer\packages\mironchik.igor.markdown\data\bin\resvg.dll

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

mkdir %CD%\..\Qt\%qt_version%\msvc2022_64\plugins\kf6

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

mkdir %CD%\..\Qt\%qt_version%\msvc2022_64\plugins\kf6\sonnet

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

mkdir %CD%\..\Qt\%qt_version%\msvc2022_64\plugins\kf6\kio

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

mkdir %CD%\..\Qt\%qt_version%\msvc2022_64\plugins\kf6\kio_dnd

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

mkdir %CD%\..\Qt\%qt_version%\msvc2022_64\plugins\kf6\urifilters

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

mkdir %CD%\..\Qt\%qt_version%\msvc2022_64\plugins\kiconthemes6

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

mkdir %CD%\..\Qt\%qt_version%\msvc2022_64\plugins\kiconthemes6\iconengines

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

xcopy %CD%\..\KDE\lib\plugins\kf6 %CD%\..\Qt\%qt_version%\msvc2022_64\plugins\kf6 /S /Y

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

del /Q %CD%\..\Qt\%qt_version%\msvc2022_64\plugins\kf6\sonnet\sonnet_ispellchecker.dll

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

xcopy %CD%\..\KDE\lib\plugins\kiconthemes6 %CD%\..\Qt\%qt_version%\msvc2022_64\plugins\kiconthemes6 /S /Y

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

xcopy %CD%\..\KDE\lib\plugins\styles %CD%\..\Qt\%qt_version%\msvc2022_64\plugins\styles /S /Y

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)
