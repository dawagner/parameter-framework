#! /bin/sh

set -e

(mkdir build_debug && cd build_debug &&
    cmake $MY_CMAKE_OPTIONS -DCMAKE_BUILD_TYPE=Debug -DCOVERAGE=${coverage} .. &&
    make -j$(nproc) &&
    (if $LINUX; then make ExperimentalTest ExperimentalMemCheck; fi))

(mkdir build && cd build &&
    cmake $MY_CMAKE_OPTIONS -DCMAKE_BUILD_TYPE=Release .. &&
    make -j$(nproc) &&
    (if $LINUX; then make test; fi) &&
    make install &&
    (if $LINUX; then cpack --verbose -G DEB && dpkg --info *.deb; fi))

(cd skeleton-subsystem &&
    cmake $MY_CMAKE_OPTIONS -DCMAKE_BUILD_TYPE=Release . &&
    make -j$(nproc) &&
    (if $LINUX; then make ExperimentalTest ExperimentalMemCheck; fi) &&
    make install)

if $LINUX; then (cd tools/clientSimulator &&
    echo "TODO - install the generated .deb instead of using \$HOME/install." \
    "This would permit to test the packaging" &&
    cmake $MY_CMAKE_OPTIONS . &&
    make &&
    make install); fi

# Check that all installed files are in a component (no "unspecified
# component" archive created)
(cd build && cpack -G TGZ -D CPACK_ARCHIVE_COMPONENT_INSTALL=ON &&
    [ ! -e "*-Unspecified.tar.gz" ])

# Keep this last
less_features="-DNETWORKING=OFF -DPYTHON_BINDINGS=OFF -DC_BINDINGS=OFF"
(mkdir build_less_features && cd build_less_features &&
    rm -rf $PREFIX/asio-1.10.6 &&
    cmake $MY_CMAKE_OPTIONS -DCMAKE_BUILD_TYPE=Debug $less_features .. &&
    make -j$(nproc) &&
    (if $LINUX; then make test; fi))

