qt_version=$(cat $PWD/script/qt.version)

echo "Installing Qt Installer Framework..."

./python/bin/aqt install-tool --outputdir ./Qt mac desktop tools_ifw qt.tools.ifw.47 || exit 1

echo "Copying binaries..."

rm -rf ./installer/packages/mironchik.igor.markdown/data/bin

rm -rf ./installer/packages/mironchik.igor.markdown/data/lib

rm -rf ./installer/packages/mironchik.igor.markdown/data/plugins

rm -rf ./installer/packages/mironchik.igor.markdown/data/libexec

rm -rf ./installer/packages/mironchik.igor.markdown/data/resources

rm -rf ./installer/packages/mironchik.igor.markdown/data/translations

mkdir ./installer/packages/mironchik.igor.markdown/data/bin || exit 1

mkdir ./installer/packages/mironchik.igor.markdown/data/lib || exit 1

mkdir ./installer/packages/mironchik.igor.markdown/data/plugins || exit 1

mkdir ./installer/packages/mironchik.igor.markdown/data/libexec || exit 1

mkdir ./installer/packages/mironchik.igor.markdown/data/resources || exit 1

mkdir ./installer/packages/mironchik.igor.markdown/data/translations || exit 1

cp ./build-markdown-tools/bin/md-editor ./installer/packages/mironchik.igor.markdown/data/bin/md-editor || exit 1

cp ./build-markdown-tools/bin/md-launcher ./installer/packages/mironchik.igor.markdown/data/bin/md-launcher || exit 1

cp $PWD/../KDE/lib/libKF6SyntaxHighlighting.6.3.0.dylib ./installer/packages/mironchik.igor.markdown/data/lib/libKF6SyntaxHighlighting.6.3.0.dylib || exit 1

cp $PWD/../KDE/lib/libKF6SyntaxHighlighting.6.dylib ./installer/packages/mironchik.igor.markdown/data/lib/libKF6SyntaxHighlighting.6.dylib || exit 1

cp $PWD/../KDE/lib/libKF6SyntaxHighlighting.dylib ./installer/packages/mironchik.igor.markdown/data/lib/libKF6SyntaxHighlighting.dylib || exit 1

cp $PWD/../KDE/lib/libKF6SonnetCore.6.9.0.dylib ./installer/packages/mironchik.igor.markdown/data/lib/libKF6SonnetCore.6.9.0.dylib || exit 1

cp $PWD/../KDE/lib/libKF6SonnetCore.6.dylib ./installer/packages/mironchik.igor.markdown/data/lib/libKF6SonnetCore.6.dylib || exit 1

cp $PWD/../KDE/lib/libKF6SonnetCore.dylib ./installer/packages/mironchik.igor.markdown/data/lib/libKF6SonnetCore.dylib || exit 1

cp $PWD/../KDE/lib/libKF6SonnetUi.6.9.0.dylib ./installer/packages/mironchik.igor.markdown/data/lib/libKF6SonnetUi.6.9.0.dylib || exit 1

cp $PWD/../KDE/lib/libKF6SonnetUi.6.dylib ./installer/packages/mironchik.igor.markdown/data/lib/libKF6SonnetUi.6.dylib || exit 1

cp $PWD/../KDE/lib/libKF6SonnetUi.dylib ./installer/packages/mironchik.igor.markdown/data/lib/libKF6SonnetUi.dylib || exit 1

cp -r ./build-markdown-tools/bin/md-pdf-gui ./installer/packages/mironchik.igor.markdown/data/bin/md-pdf-gui || exit 1

cp -r ./build-markdown-tools/lib ./installer/packages/mironchik.igor.markdown/data || exit 1

rm -f ./installer/packages/mironchik.igor.markdown/data/lib/*.a || exit 1

cp -r ./Qt/$qt_version/macos/lib ./installer/packages/mironchik.igor.markdown/data || exit 1

cp -r ./Qt/$qt_version/macos/plugins ./installer/packages/mironchik.igor.markdown/data || exit 1

cp -r ./Qt/$qt_version/macos/translations ./installer/packages/mironchik.igor.markdown/data || exit 1

cp -r ./3rdparty/resvg/target/release/libresvg.dylib ./installer/packages/mironchik.igor.markdown/data/lib/libresvg.dylib || exit 1

rm -rf ./installer/packages/mironchik.igor.markdown/data/lib/cmake || exit 1

rm -rf ./installer/packages/mironchik.igor.markdown/data/lib/metatypes || exit 1

rm -rf ./installer/packages/mironchik.igor.markdown/data/lib/pkgconfig || exit 1

rm -rf ./installer/packages/mironchik.igor.markdown/data/lib/*.prl || exit 1

echo "Creating installer..."

./Qt/Tools/QtInstallerFramework/4.7/bin/binarycreator -c ./installer/config/config.xml -p ./installer/packages Markdown_MacOS_x64.Installer || exit 1
