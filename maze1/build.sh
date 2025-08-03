#!/bin/bash
# Script to bundle Qt app and dependencies for distribution
# Usage: ./package-qt-app.sh

set -e

APP=Maze1
BUILD_DIR=build
DIST_DIR=dist
QT_PLUGINS_DIR=$(dirname $(ldd $BUILD_DIR/$APP | grep libQt6Core | awk '{print $3}'))/../plugins

# Clean and create dist dir
rm -rf $DIST_DIR
mkdir -p $DIST_DIR/platforms

# Copy executable
cp $BUILD_DIR/$APP $DIST_DIR/

# Copy Qt libraries
ldd $BUILD_DIR/$APP | grep "/libQt" | awk '{print $3}' | xargs -I '{}' cp -v '{}' $DIST_DIR/

# Copy platform plugin (libqxcb)
PLATFORM_PLUGIN=""
if [ -f $QT_PLUGINS_DIR/platforms/libqxcb.so ]; then
  PLATFORM_PLUGIN="$QT_PLUGINS_DIR/platforms/libqxcb.so"
elif [ -f /usr/lib/qt6/plugins/platforms/libqxcb.so ]; then
  PLATFORM_PLUGIN="/usr/lib/qt6/plugins/platforms/libqxcb.so"
elif [ -f /usr/lib64/qt6/plugins/platforms/libqxcb.so ]; then
  PLATFORM_PLUGIN="/usr/lib64/qt6/plugins/platforms/libqxcb.so"
else
  PLATFORM_PLUGIN=$(find /usr -name libqxcb.so 2>/dev/null | head -n 1)
fi
if [ -n "$PLATFORM_PLUGIN" ] && [ -f "$PLATFORM_PLUGIN" ]; then
  cp -v "$PLATFORM_PLUGIN" $DIST_DIR/platforms/
else
  echo "Warning: Could not find libqxcb.so. The app may not run on other systems."
fi

# Copy assets if needed
cp -r asset $DIST_DIR/

# Copy resources if needed
if [ -f $BUILD_DIR/qrc_resources.cpp ]; then
  cp $BUILD_DIR/qrc_resources.cpp $DIST_DIR/
fi

# Print run instructions
echo "\nBundle created in $DIST_DIR. To run on another system, use:"
echo "  export QT_QPA_PLATFORM_PLUGIN_PATH=\"\$(pwd)/$DIST_DIR/platforms\""
echo "  cd $DIST_DIR && ./QTqt"
