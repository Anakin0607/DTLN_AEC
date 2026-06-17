if command -v aarch64-none-linux-gnu-gcc  &> /dev/null; then
  ln -svf $(which aarch64-none-linux-gnu-gcc) ./aarch64-linux-gnu-gcc
  ln -svf $(which aarch64-none-linux-gnu-g++) ./aarch64-linux-gnu-g++
  export PATH=$PWD:$PATH
fi

if ! command -v aarch64-linux-gnu-gcc  &> /dev/null; then
  echo "Please install a toolchain for cross-compiling."
  echo "You can use"
  echo "sudo apt install crossbuild-essential-arm64"
  echo "to install the toolchain"
  exit 1
fi

set -ex

dir=build-aarch64-linux-gnu
mkdir -p $dir
cd $dir

cmake \
  -DCMAKE_INSTALL_PREFIX=../ \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=ON \
  -DBUILD_MARCH=aarch64 \
  -DCMAKE_TOOLCHAIN_FILE=../toolchains/aarch64-linux-gnu.toolchain.cmake \
  ..

make VERBOSE=1 -j32
make install/strip
rm ../aarch64-linux-gnu-*