echo "Installing aqt..."

pip install aqtinstall --upgrade

mkdir Qt

qt_version=$(cat $PWD/script/qt.version)

echo "Installing Qt..."

export AQT_CONCURRENCY=1

aqt install-qt --outputdir ./Qt linux desktop $qt_version linux_gcc_64 -m qtimageformats qtwebchannel qtwebview qtpositioning qtwebengine || exit 1
