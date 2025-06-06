echo "Installing aqt..."

pip install aqtinstall --upgrade

mkdir Qt

echo "Installing Qt..."

set /P qt_version=<%CD%\script\qt.version

aqt install-qt --outputdir Qt windows desktop %qt_version% win64_msvc2022_64 -m qtimageformats qtwebchannel qtwebview qtpositioning qtwebengine

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)
