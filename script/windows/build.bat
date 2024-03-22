set self_dir=%CD%

echo "Building markdown-tools..."

mkdir build-markdown-tools

conan install . -of build-markdown-tools -s build_type=Release --build=missing

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake -DCMAKE_BUILD_TYPE=Release -S . -B build-markdown-tools -DBUILD_MDPDF_TESTS=OFF -DCMAKE_PREFIX_PATH=%CD%\Qt\6.6.2\msvc2019_64 -DECM_DIR=%CD%\..\KDE\share\ECM\cmake -DKF6SyntaxHighlighting_DIR=%CD%\..\KDE\lib\cmake\KF6SyntaxHighlighting -DOPENSSL_ROOT_DIR=%CD%\Qt\Tools\OpenSSLv3\Win_x64

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

cmake --build build-markdown-tools --config Release

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)
