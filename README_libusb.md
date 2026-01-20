# libusb setup for ofxMidiDevice (macOS/Homebrew)

This addon expects libusb to be available. The included `addon_config.mk` will try pkg-config first and also falls back to common Homebrew paths (`/opt/homebrew` for Apple Silicon, `/usr/local` for Intel).

## Install libusb via Homebrew
```bash
brew install libusb
```

After installation, `pkg-config --modversion libusb-1.0` should report the installed version. The addon links with `-lusb-1.0` and searches the Homebrew include/lib directories automatically.

## Bundling libusb with your app (macOS)
- Option A: static link if Homebrew ships a static archive: `pkg-config --static --libs libusb-1.0` (Homebrew typically does not ship it by default).
- Option B: bundle the dylib in your app:
  1) Copy `libusb-1.0.dylib` into `YourApp.app/Contents/Frameworks/`.
  2) Ensure the app has an rpath to `@executable_path/../Frameworks` (oF apps usually do); otherwise add: `install_name_tool -add_rpath @executable_path/../Frameworks YourApp.app/Contents/MacOS/YourApp`.
  3) Rewrite the lib path to use the bundled copy: `install_name_tool -change /opt/homebrew/lib/libusb-1.0.dylib @rpath/libusb-1.0.dylib YourApp.app/Contents/MacOS/YourApp` (adjust the source path for Intel: `/usr/local/lib/...`).

## Notes
- If you use a non-default Homebrew prefix, adjust the include/lib paths in `addon_config.mk` accordingly.
- On other platforms, add the appropriate libusb include/lib paths or `ADDON_PKG_CONFIG_LIBRARIES` entries to `addon_config.mk`.
