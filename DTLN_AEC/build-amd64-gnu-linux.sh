set -ex

dir=build
mkdir -p $dir
cd $dir

export CXXFLAGS="-Wno-stringop-overflow -Wno-array-bounds"
export CFLAGS="-Wno-stringop-overflow -Wno-array-bounds"

cmake \
  -DCMAKE_INSTALL_PREFIX=../ \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=ON \
  -DBUILD_MARCH=amd64 \
  ..

make VERBOSE=1 -j8
make install/strip