#!/bin/bash

# Check for dependencies
if ! pkg-config --exists gtk+-3.0; then
    echo "Error: gtk+-3.0 is not installed. Please install libgtk-3-dev (Ubuntu/Debian) or gtk3 (Arch/Fedora)."
    exit 1
fi

# Build the project
echo "Building Calendarrr..."
if ! g++ src/main.cpp -o calendarrr $(pkg-config --cflags --libs gtk+-3.0); then
    echo "Error: Failed to build Calendarrr. Make sure you have g++ installed."
    exit 1
fi

# Move the binary to /usr/local/bin
sudo cp calendarrr /usr/local/bin/calendarrr

# Clean up the local binary
rm calendarrr

# Create the applications directory if it doesn't exist
mkdir -p ~/.local/share/applications/

# Install the desktop entry
cp calendarrr.desktop ~/.local/share/applications/calendarrr.desktop

# Move the icons to the appropriate directory
mkdir -p ~/.local/share/icons/hicolor/scalable/apps/
cp docs/assets/calendarrr-outlined.svg ~/.local/share/icons/hicolor/scalable/apps/calendarrr-outlined.svg
cp docs/assets/calendarrr.svg ~/.local/share/icons/hicolor/scalable/apps/calendarrr.svg

# Update the icon cache
gtk-update-icon-cache -f -t ~/.local/share/icons/hicolor > /dev/null 2>&1 || true

echo "Setup complete! You can now run Calendarrr from your application menu."

