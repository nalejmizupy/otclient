# Changelog

## 02-04-2026
- Added FreeType-backed TrueType font loading, bundled Verdana assets, so themed widgets can request `Family-Size` fonts without duplicating layout logic.

## 05-12-2023
### Breaking API Changes
- `UIWidget` property `qr-code` & `qr-code-border` replaced with `UIQrCode` properties `code` & `code-border`
- `image-source-base64` replaced with `image-source: base64:/path/to/image`
- `#include "shadermanager.h"` moved to `#include <framework/graphics/shadermanager.h>`
