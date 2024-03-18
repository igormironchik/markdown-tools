echo "Installing aqt..."

pip install aqtinstall

mkdir Qt

echo "Installing Qt..."

aqt install-qt --outputdir Qt windows desktop 6.6.2 win64_msvc2019_64 -m qtimageformats qtwebchannel qtwebengine qtwebview qtpositioning

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

set self_dir=%CD%

echo "Building markdown-tools..."

mkdir build-markdown-tools

conan install . -of build-markdown-tools -s build_type=Release --build=missing

cmake -DCMAKE_BUILD_TYPE=Release -S . -B build-markdown-tools -DBUILD_MDPDF_TESTS=OFF -DCMAKE_PREFIX_PATH=%self_dir%\Qt\6.6.2\msvc2019_64;..\KDE

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build build-markdown-tools --config Release

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)
