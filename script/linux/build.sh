echo "Installing aqt..."

pip install aqtinstall

mkdir Qt

echo "Installing Qt..."

aqt install-qt --outputdir ./Qt linux desktop 6.6.2 gcc_64 -m qtimageformats qtwebchannel qtwebengine qtwebview qtpositioning || exit 1

self_dir=`echo "$(cd "$(dirname ".")" && pwd)"`

echo "Building markdown-tools..."

mkdir build-markdown-tools

cmake -DCMAKE_BUILD_TYPE=Release -S . -B build-markdown-tools -DBUILD_MDPDF_TESTS=OFF -DCMAKE_PREFIX_PATH=${self_dir}/Qt/6.6.2/gcc_64:../KDE || exit 1

cmake --build build-markdown-tools --config Release || exit 1
