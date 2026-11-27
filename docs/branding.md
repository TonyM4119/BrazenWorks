# Branding

PureCAD is based on [MitCad](https://github.com/mitcad/Mitcad) (MIT
licence). The product's name and identity live in one file,
[`branding.json`](../branding.json):

| Key | Use |
|---|---|
| `displayName` | The name people read: window title, About, `--version`, every interface text, the installer, the program file's Properties |
| `storedName` | The name the settings and data folders are keyed on (apart from a MitCad installation's); the install folder and the file type's registry names |
| `version` | The product's own version (`<major>.<minor>.<patch>`), shown to people: title, title bar, About, `--version`, the program file's Properties, the installer. MitCad's version (`project()` in `CMakeLists.txt`, `MITCAD_VERSION`) stays as it is: designs, settings and installations record it, and About says which MitCad release this is based on |
| `slug` | Lower-case name: the program file on Windows (`<slug>.exe`), the installer file, the desktop file |
| `iconDir` | The icon folder: `icon.svg`, `icon-small.svg`, `png/icon-<size>.png`, `icon.ico` |
| `organizationDomain` | macOS settings domain |
| `repositoryUrl` | This project's repository; Send Feedback and the start page's Report an Issue open its `/issues` page |
| `upstreamName`, `upstreamUrl` | MitCad: the name its code and texts use |
| `upstreamUpdateChecks` | `false`: MitCad's update check stays off (it would offer MitCad's own releases) |
| `upstreamCommunityLinks` | `false`: MitCad's Boost development (Ko-fi) and Discord buttons, their Preferences check boxes and Help's Discord entry stay hidden |

CMake reads it ([`cmake/Branding.cmake`](../cmake/Branding.cmake)) and
generates, in `build/<preset>/generated/brand/`:

- `brand.hpp` (namespace `brand`), on every target's include path;
- `brand.rc` on Windows: the program file's icon and version information
  (Explorer, the taskbar, Task Manager, the file's Properties);
- `brand.nsh` for the installer: the names the installer scripts use.

Change a value and rebuild. The build target keeps the upstream's name
(`mitcad`), so `$<TARGET_FILE:mitcad>` and the upstream's scripts follow
the program file's new name.

## Icon

Edit `icon.svg` (64 px and up) and `icon-small.svg` (48 px and under) in
`iconDir`, then run `tools/make-brand-icons.sh` (rsvg-convert and
ImageMagick) and commit the PNG and ICO files it writes. The current icon
is a placeholder: an isometric cube with a chamfered corner.

## Interface texts

The interface texts keep MitCad's wording: `BrandTranslator`
([`app/framework/Brand.hpp`](../app/framework/Brand.hpp)), asked before
every other translator, swaps `upstreamName` for `displayName` in every
language. This keeps PureCAD's changes small, so MitCad's updates
merge cleanly.

## Not branded (yet)

- Internal identifiers: `MITCAD_*` environment variables, crate and
  namespace names, the `mitcad-cli`, `mitcad-render` and other helper
  programs.
- The `.mitcad` design file extension, kept so files stay compatible with
  MitCad. The installer registers it for PureCAD.
- MitCad's own updater (off, see `upstreamUpdateChecks`): it still looks
  for `mitcad.exe`.
- macOS and Linux packages: the app bundle, the AppImage and its desktop
  file still use MitCad's name and icon; the program file is renamed on
  Windows only.
