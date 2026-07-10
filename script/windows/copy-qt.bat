cd installer\packages\mironchik.igor.markdown\data\bin

IF %ERRORLEVEL% NEQ 0 (
	exit /B %ERRORLEVEL%
)

windeployqt.exe kf6\kio\kio_file.dll
windeployqt.exe kf6\kio\kio_ftp.dll
windeployqt.exe kf6\kio\kio_http.dll
windeployqt.exe kf6\kio\kio_remote.dll
windeployqt.exe kf6\kio\kio_trash.dll

windeployqt.exe kf6\kio_dnd\dropintonewfolder.dll

windeployqt.exe kf6\sonnet\sonnet_hunspell.dll

windeployqt.exe kf6\urifilters\fixhosturifilter.dll
windeployqt.exe kf6\urifilters\kshorturifilter.dll
windeployqt.exe kf6\urifilters\kuriikwsfilter.dll
windeployqt.exe kf6\urifilters\kurisearchfilter.dll
windeployqt.exe kf6\urifilters\localdomainurifilter.dll

windeployqt.exe kf6\ktranscript.dll

windeployqt.exe kiconthemes6\iconengines\KIconEnginePlugin.dll

windeployqt.exe styles\breeze6.dll

windeployqt.exe KF6Archive.dll
windeployqt.exe KF6Bookmarks.dll
windeployqt.exe KF6BookmarksWidgets.dll
windeployqt.exe KF6BreezeIcons.dll
windeployqt.exe KF6Codecs.dll
windeployqt.exe KF6ColorScheme.dll
windeployqt.exe KF6Completion.dll
windeployqt.exe KF6ConfigCore.dll
windeployqt.exe KF6ConfigGui.dll
windeployqt.exe KF6ConfigQml.dll
windeployqt.exe KF6ConfigWidgets.dll
windeployqt.exe KF6CoreAddons.dll
windeployqt.exe KF6Crash.dll
windeployqt.exe KF6GuiAddons.dll
windeployqt.exe KF6I18n.dll
windeployqt.exe KF6I18nLocaleData.dll
windeployqt.exe KF6I18nQml.dll
windeployqt.exe KF6IconThemes.dll
windeployqt.exe KF6IconWidgets.dll
windeployqt.exe KF6ItemViews.dll
windeployqt.exe KF6JobWidgets.dll
windeployqt.exe KF6KCMUtils.dll
windeployqt.exe KF6KCMUtilsCore.dll
windeployqt.exe KF6KCMUtilsQuick.dll
windeployqt.exe KF6KIOCore.dll
windeployqt.exe KF6KIOFileWidgets.dll
windeployqt.exe KF6KIOGui.dll
windeployqt.exe KF6KIOWidgets.dll
windeployqt.exe KF6Notifications.dll
windeployqt.exe KF6Service.dll
windeployqt.exe KF6Solid.dll
windeployqt.exe KF6SonnetCore.dll
windeployqt.exe KF6SonnetUi.dll
windeployqt.exe KF6SyntaxHighlighting.dll
windeployqt.exe KF6WidgetsAddons.dll
windeployqt.exe KF6WindowSystem.dll
windeployqt.exe KF6XmlGui.dll
windeployqt.exe Kirigami.dll
windeployqt.exe KirigamiControls.dll
windeployqt.exe KirigamiDelegates.dll
windeployqt.exe KirigamiDialogs.dll
windeployqt.exe KirigamiForms.dll
windeployqt.exe KirigamiFormsPrivateCards.dll
windeployqt.exe KirigamiFormsPrivateFlat.dll
windeployqt.exe KirigamiFormsPrivateTemplates.dll
windeployqt.exe KirigamiLayouts.dll
windeployqt.exe KirigamiLayoutsPrivate.dll
windeployqt.exe KirigamiPlatform.dll
windeployqt.exe KirigamiPolyfill.dll
windeployqt.exe KirigamiPrimitives.dll
windeployqt.exe KirigamiPrivate.dll
windeployqt.exe KirigamiTemplates.dll
windeployqt.exe kuriikwsfiltereng_private.dll
windeployqt.exe kioworker.exe
windeployqt.exe md-editor.exe
windeployqt.exe md-launcher.exe
windeployqt.exe md-pdf-gui.exe

cd ..\..\..\..\..