echo "Installing OpenSSL..."

aqt install-tool --outputdir Qt windows desktop tools_opensslv3_x64

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)
