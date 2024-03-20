echo "Installing aqt..."

pip install aqtinstall

mkdir Qt

echo "Installing Qt..."

aqt install-qt --outputdir ./Qt linux desktop 6.6.2 gcc_64 -m qtimageformats qtwebchannel qtwebengine qtwebview qtpositioning || exit 1
