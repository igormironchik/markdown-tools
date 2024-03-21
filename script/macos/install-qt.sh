echo "Installing aqt..."

pip3 install aqtinstall

mkdir Qt

echo "Installing Qt..."

aqt install-qt --outputdir ./Qt mac desktop 6.6.2 -m qtimageformats qtwebchannel qtwebengine qtwebview qtpositioning || exit 1
