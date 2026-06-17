if ! command -v arm-openeuler-linux-gnueabihf-gcc  &> /dev/null; then
  echo "Please install a toolchain for cross-compiling."
  exit 1
fi

set -ex

dir=build-arm-linux-openeuler
mkdir -p $dir
cd $dir

cmake \
  -DCMAKE_INSTALL_PREFIX=../DTLN_AEC_euler \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_MARCH=arm-euler \
  -DXNNPACK_ENABLE_ARM_DOTPROD=OFF \
  -DXNNPACK_ENABLE_ARM_I8MM=OFF \
  -DCMAKE_TOOLCHAIN_FILE=../toolchains/arm-linux-openeuler.toolchain.cmake \
  ..

make VERBOSE=1 -j32
make install/strip