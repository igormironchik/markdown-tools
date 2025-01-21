set /P qt_version=<%CD%\script\qt.version

echo "Copying binaries..."

rmdir /S /Q installer\packages\mironchik.igor.markdown\data\bin

rmdir /S /Q installer\packages\mironchik.igor.markdown\data\lib

rmdir /S /Q installer\packages\mironchik.igor.markdown\data\plugins

rmdir /S /Q installer\packages\mironchik.igor.markdown\data\libexec

rmdir /S /Q installer\packages\mironchik.igor.markdown\data\resources

rmdir /S /Q installer\packages\mironchik.igor.markdown\data\translations

mkdir installer\packages\mironchik.igor.markdown\data\bin

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

mkdir installer\packages\mironchik.igor.markdown\data\plugins

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

mkdir installer\packages\mironchik.igor.markdown\data\resources

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

mkdir installer\packages\mironchik.igor.markdown\data\translations

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

mkdir installer\packages\mironchik.igor.markdown\data\lib

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

copy /Y Qt\%qt_version%\msvc2019_64\bin installer\packages\mironchik.igor.markdown\data\bin

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

del /Q installer\packages\mironchik.igor.markdown\data\bin\*.exe

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

del /Q installer\packages\mironchik.igor.markdown\data\bin\*.bat

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

del /Q installer\packages\mironchik.igor.markdown\data\bin\*.cmake

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

del /Q installer\packages\mironchik.igor.markdown\data\bin\*.pl

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

del /Q installer\packages\mironchik.igor.markdown\data\bin\*.py

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

del /Q installer\packages\mironchik.igor.markdown\data\bin\*.sh

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

del /Q installer\packages\mironchik.igor.markdown\data\bin\Qt6*d.dll

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

copy /Y build-markdown-tools\bin installer\packages\mironchik.igor.markdown\data\bin

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

mkdir installer\packages\mironchik.igor.markdown\data\lib\plugins

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

xcopy %CD%\..\KDE\lib\plugins installer\packages\mironchik.igor.markdown\data\lib\plugins /S /Y

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

copy %CD%\..\KDE\bin\KF6SyntaxHighlighting.dll installer\packages\mironchik.igor.markdown\data\bin\KF6SyntaxHighlighting.dll

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

copy %CD%\..\KDE\bin\KF6SonnetCore.dll installer\packages\mironchik.igor.markdown\data\bin\KF6SonnetCore.dll

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

copy %CD%\..\KDE\bin\KF6SonnetUi.dll installer\packages\mironchik.igor.markdown\data\bin\KF6SonnetUi.dll

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

xcopy build-markdown-tools\lib installer\packages\mironchik.igor.markdown\data\lib /S /Y

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

del /Q installer\packages\mironchik.igor.markdown\data\lib\*.lib

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

rmdir /S /Q installer\packages\mironchik.igor.markdown\data\lib\Release

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

del /Q installer\packages\mironchik.igor.markdown\data\bin\test.render.bat

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

copy /Y Qt\%qt_version%\msvc2019_64\bin\QtWebEngineProcess.exe installer\packages\mironchik.igor.markdown\data\bin\QtWebEngineProcess.exe

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

xcopy Qt\%qt_version%\msvc2019_64\plugins installer\packages\mironchik.igor.markdown\data\plugins /S /Y

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

xcopy Qt\%qt_version%\msvc2019_64\resources installer\packages\mironchik.igor.markdown\data\resources /S /Y

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

xcopy Qt\%qt_version%\msvc2019_64\translations installer\packages\mironchik.igor.markdown\data\translations /S /Y

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

copy /Y 3rdparty\Windows\VC_Redist\VC_redist.x64.exe installer\packages\mironchik.igor.markdown\data\bin\VC_redist.x64.exe

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

copy /Y 3rdparty\resvg\target\release\resvg.dll installer\packages\mironchik.igor.markdown\data\bin\resvg.dll

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)
