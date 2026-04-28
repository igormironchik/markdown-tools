echo "Installing aqt..."

./python/bin/pip3 install aqtinstall --upgrade

mkdir Qt

qt_version=$(cat $PWD/script/qt.version)

echo "Installing Qt..."

export AQT_CONCURRENCY=1

./python/bin/aqt install-qt --outputdir ./Qt mac desktop $qt_version -m qtimageformats qtwebchannel qtwebview qtpositioning qtwebengine || exit 1
