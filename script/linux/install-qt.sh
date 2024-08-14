echo "Installing aqt..."

pip install aqtinstall

mkdir Qt

echo "Installing Qt..."

aqt install-qt --outputdir ./Qt linux desktop 6.7.2 linux_gcc_64 -m qtimageformats qtwebchannel qtwebengine qtwebview qtpositioning || exit 1
