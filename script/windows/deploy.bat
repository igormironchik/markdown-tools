echo "Installing Qt Installer Framework..."

aqt install-tool --outputdir Qt windows desktop tools_ifw qt.tools.ifw.47

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

echo "Creating installer..."

Qt\Tools\QtInstallerFramework\4.7\bin\binarycreator.exe -c installer\config\config.xml -p installer\packages Markdown_Windows_x64.Installer.exe

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)
