# TrueType Fonts

The OTClient UI now ships with FreeType-powered font rendering plus a lighter, more flexible inventory presentation.

## TrueType font loading

- FreeType is linked into `otclient_core`, and `BitmapFont::loadTrueType` / `FontManager::getFont` automatically attempt to resolve font names that follow the `Family-Size` (or `Family-Sizepx`) convention. This keeps legacy font names compatible while allowing Verdana and other `.ttf` files placed under `data/fonts/` to render crisp glyphs at arbitrary sizes.
- When `g_fonts:getFont("Verdana-10")` is invoked, the loader probes candidate paths such as `/data/fonts/Verdana.ttf`, `/fonts/Verdana-10.ttf`, etc. Logs are emitted for each candidate so missing fonts can be detected easily.
- Glyphs are rasterized into a soft texture atlas and exposed through the existing `BitmapFont` API, so widgets that were previously using `cipsoftFont` can now reference fonts like `Verdana Bold-10` without additional UI changes.

## Adding your own fonts or styles

1. Drop new `.ttf` files inside `data/fonts/` (for example: `data/fonts/MyFont.ttf`).
2. Use the naming convention `FontFamily-Size` inside OTML or Lua styles (e.g. `font: Verdana-8px` or `font: MyFont-12`).
3. Ensure `data/fonts/` is accessible via the resource search path; the default build already exposes the repository root so the bundled Verdana files are automatically found.
