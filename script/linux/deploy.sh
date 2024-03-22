echo "Installing Qt Installer Framework..."

aqt install-tool --outputdir ./Qt linux desktop tools_ifw qt.tools.ifw.47 || exit 1

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

cp ./build-markdown-tools/bin/launcher ./installer/packages/mironchik.igor.markdown/data/bin/launcher || exit 1

cp ./build-markdown-tools/bin/md-pdf-gui ./installer/packages/mironchik.igor.markdown/data/bin/md-pdf-gui || exit 1

cp $PWD/../KDE/lib/x86_64-linux-gnu/libKF6SyntaxHighlighting.so.6.1.0 ./installer/packages/mironchik.igor.markdown/data/lib/libKF6SyntaxHighlighting.so.6.1.0

cp $PWD/../KDE/lib/x86_64-linux-gnu/libKF6SyntaxHighlighting.so.6 ./installer/packages/mironchik.igor.markdown/data/lib/libKF6SyntaxHighlighting.so.6

cp $PWD/../KDE/lib/x86_64-linux-gnu/libKF6SyntaxHighlighting.so ./installer/packages/mironchik.igor.markdown/data/lib/libKF6SyntaxHighlighting.so

cp -r ./build-markdown-tools/lib ./installer/packages/mironchik.igor.markdown/data || exit 1

rm -f ./installer/packages/mironchik.igor.markdown/data/lib/*.a || exit 1

cp -r ./Qt/6.6.2/gcc_64/lib ./installer/packages/mironchik.igor.markdown/data || exit 1

cp ./Qt/6.6.2/gcc_64/libexec/QtWebEngineProcess ./installer/packages/mironchik.igor.markdown/data/libexec/QtWebEngineProcess || exit 1

cp -r ./Qt/6.6.2/gcc_64/plugins ./installer/packages/mironchik.igor.markdown/data || exit 1

cp -r ./Qt/6.6.2/gcc_64/resources ./installer/packages/mironchik.igor.markdown/data || exit 1

cp -r ./Qt/6.6.2/gcc_64/translations ./installer/packages/mironchik.igor.markdown/data || exit 1

rm -rf ./installer/packages/mironchik.igor.markdown/data/lib/cmake || exit 1

rm -rf ./installer/packages/mironchik.igor.markdown/data/lib/metatypes || exit 1

rm -rf ./installer/packages/mironchik.igor.markdown/data/lib/pkgconfig || exit 1

rm -rf ./installer/packages/mironchik.igor.markdown/data/lib/*.prl || exit 1

echo "Creating installer..."

./Qt/Tools/QtInstallerFramework/4.7/bin/binarycreator -c ./installer/config/config.xml -p ./installer/packages Markdown_Linux_x64.Installer || exit 1
