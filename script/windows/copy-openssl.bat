copy %CD%\Qt\Tools\OpenSSLv3\Win_x64\bin\libcrypto-3-x64.dll installer\packages\mironchik.igor.markdown\data\bin\libcrypto-3-x64.dll

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

copy %CD%\Qt\Tools\OpenSSLv3\Win_x64\bin\libssl-3-x64.dll installer\packages\mironchik.igor.markdown\data\bin\libssl-3-x64.dll

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

mkdir installer\packages\mironchik.igor.markdown\data\bin\engines-3

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

mkdir installer\packages\mironchik.igor.markdown\data\bin\ossl-modules

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

copy %CD%\Qt\Tools\OpenSSLv3\Win_x64\lib\engines-3\capi.dll installer\packages\mironchik.igor.markdown\data\bin\engines-3\capi.dll

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

copy %CD%\Qt\Tools\OpenSSLv3\Win_x64\lib\engines-3\loader_attic.dll installer\packages\mironchik.igor.markdown\data\bin\engines-3\loader_attic.dll

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

copy %CD%\Qt\Tools\OpenSSLv3\Win_x64\lib\engines-3\padlock.dll installer\packages\mironchik.igor.markdown\data\bin\engines-3\padlock.dll

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

copy %CD%\Qt\Tools\OpenSSLv3\Win_x64\lib\ossl-modules\legacy.dll installer\packages\mironchik.igor.markdown\data\bin\ossl-modules\legacy.dll

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)
