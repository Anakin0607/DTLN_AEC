if ! command -v arm-none-linux-gnueabihf-gcc  &> /dev/null; then
  echo "Please install a toolchain for cross-compiling."
  echo "You can use"
  echo "sudo apt install crossbuild-essential-armhf"
  echo "to install the toolchain"
  exit 1
fi

set -ex

dir=build-arm-linux-gnueabihf
mkdir -p $dir
cd $dir

cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=ON \
  -DBUILD_MARCH=arm \
  -DXNNPACK_ENABLE_ARM_DOTPROD=OFF \
  -DXNNPACK_ENABLE_ARM_I8MM=OFF \
  -DCMAKE_TOOLCHAIN_FILE=../toolchains/arm-linux-gnueabihf.toolchain.cmake \
  ..

make VERBOSE=1 -j32
make install/strip