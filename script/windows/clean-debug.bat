set /P qt_version=<%CD%\script\qt.version
rmdir /S /Q installer\packages\mironchik.igor.markdown\data\bin
rmdir /S /Q %CD%\..\Qt\%qt_version%\msvc2022_64\plugins\kf6
rmdir /S /Q %CD%\..\Qt\%qt_version%\msvc2022_64\plugins\kiconthemes6
del /Q %CD%\..\Qt\%qt_version%\msvc2022_64\plugins\styles\breeze6.dll
