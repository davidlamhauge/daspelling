#!/usr/bin/env bash

trap 'echo "::error::Command failed"' ERR
set -eE

create_package_linux() {
  echo "::group::Set up AppImage contents"
  make install INSTALL_ROOT="${PWD}/build" VERBOSE=1
  echo "::endgroup::"

  echo "::group::Create AppImage"
  echo "listing files"
  ls -la 
  curl -fsSLO https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage
  chmod 755 linuxdeployqt-continuous-x86_64.AppImage
  LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:" \
    ./linuxdeployqt-continuous-x86_64.AppImage \
    /build/build/usr/share/applications/daspelling.desktop -appimage
  local output_name="daspelling-linux-$1-$(date +%F)"
  mv daspelling*.AppImage "$output_name.AppImage"
  echo "::set-output name=package-name::$output_name.AppImage"
  echo "::endgroup::"
}

create_package_macos() {
  echo "::group::Clean"
  make clean
  echo "listing files"
  ls -la 
  echo "::endgroup::"

  echo "Deploy Qt libraries"
  macdeployqt daspelling.app
  echo "::group::Apply macdeployqt fix"
  curl -fsSLO https://github.com/aurelien-rainone/macdeployqtfix/archive/master.zip
  bsdtar xf master.zip
  python macdeployqtfix-master/macdeployqtfix.py \
    daspelling.app/Contents/MacOS/daspelling \
    /usr/local/Cellar/qt/5.9.1/
  echo "::endgroup::"
  echo "Remove files"
  rm -rf macdeployqtfix-master master.zip
  echo "Create ZIP"
  cd ..
  pwd
  bsdtar caf "daspelling-mac-$1-$(date +%F).zip" build
  echo "::set-output name=package-name::daspelling-mac-$1-$(date +%F).zip"
}

create_package_windows() {
  local platform="${INPUT_ARCH%%_*}"
  echo "Remove files"
  find \( -name '*.pdb' -o -name '*.ilk' \) -delete
  echo "::group::Deploy Qt libraries"
  echo "listing files"
  ls -la 
  windeployqt release/daspelling.exe
  echo "::endgroup::"
  echo "Create ZIP"
  cd ..
  pwd
  "${WINDIR}\\System32\\tar" caf "daspelling-${platform}-$1-$(date +%F).zip" build
  echo "::set-output name=package-name::daspelling-${platform}-$1-$(date +%F).zip"
}

"create_package_$(echo $RUNNER_OS | tr '[A-Z]' '[a-z]')" "${GITHUB_RUN_NUMBER}"
