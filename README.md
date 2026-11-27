# PureCAD

PureCAD is a free, open-source parametric CAD/CAM application for makers
and shops, with a Fusion 360-style workflow. It is built on
[MitCad](https://github.com/mitcad/Mitcad) and keeps everything MitCad
does, then adds its own look, workflow and extras. Its license is the
[MIT license](LICENSE).

- **Parametric modeling with a history.** Draw sketches with constraints
  and dimensions; features go on a timeline; parameters hold expressions.
  Change a dimension and the model rebuilds.
- **A familiar workflow.** A tabbed toolbar, command panels with a live
  preview and drag handles, the Browser, the Timeline and a command
  search, moving towards Fusion 360's layout.
- **Many CAD formats.** Opens FreeCAD, `.f3d`, SolidCraft and `.ipt` files
  with their history; imports and exports STEP, IGES, BRep, STL, OBJ, DXF
  and 3MF; exports drawings (PDF, SVG, DXF, DWG). The full table:
  [File formats](docs/user-guide.md#file-formats).
- **Making things.** Sheet metal with flat patterns, CAM tooling, and
  sending bodies straight to a slicer (Bambu Studio, OrcaSlicer,
  PrusaSlicer, UltiMaker Cura).

## What PureCAD adds

- **Its own identity:** the PureCAD name, version (0.1.0), icon, program
  file (`purecad.exe`) and installer, all set in one file,
  [`branding.json`](branding.json) ([docs/branding.md](docs/branding.md)).
- **Smoother rendering and motion:** a Smoothness setting for rounder
  curves and an Anti-aliasing setting (Preferences › Display); camera
  moves that ease in and out, including into and out of sketches; fading
  hovers and command panels.
- **A SolidCraft-style start page** with New Design in inches first.
- In progress, one step at a time: a Fusion 360-style theme and toolbar,
  a customizable right-click menu, and SolidCraft's extra tools.

MitCad's own update checks, Ko-fi and Discord links are switched off:
PureCAD is a separate project and offers its own builds.

## Building on Windows

Requirements: Windows 10 or 11 with Visual Studio 2022 or newer and its
C++ desktop tools. Clone the repository to a short path, such as
`C:\dev\src\PureCAD`, then in PowerShell run as administrator:

```powershell
$env:MITCAD_RENDER_CUDA = "0"   # no CUDA renderer build (optional)
powershell -ExecutionPolicy Bypass -File tools\dev-env\setup-windows.ps1 -Repo C:\dev\src\PureCAD
```

It installs Git, CMake, Ninja, Rust, Python, NSIS, vcpkg and Qt 6.12, and
writes a local `CMakeUserPresets.json`. Then, in a normal PowerShell:

```powershell
tools\dev-env\msvc.cmd cmake --preset dev
tools\dev-env\msvc.cmd cmake --build --preset dev
.\build\dev\purecad.exe
```

The first build takes a while (vcpkg builds OCCT and the libraries);
later builds are quick. Run `purecad.exe` with Qt's `bin` folder on
`PATH` (`C:\dev\Qt\6.12.0\msvc2022_64\bin`). It needs OpenGL 2.1 or newer.

Linux and macOS build as MitCad does: see
[MitCad's README](docs/mitcad-readme.md#building) and
[docs/development.md](docs/development.md). (The program file is renamed
on Windows only so far.)

## Staying up to date with MitCad

PureCAD is a thin fork: its changes are kept small and marked `Fork:` in
the code, so MitCad's releases merge in cleanly:

```sh
git remote add upstream https://github.com/mitcad/Mitcad.git   # once
git fetch upstream main
git merge upstream/main
```

That is why MitCad's internal names stay in the source (file and crate
names, `MITCAD_*` settings, its changelog and design notes): renaming them
would make every update conflict. Everything the user sees says PureCAD.

## Credits

PureCAD is built on [MitCad](https://github.com/mitcad/Mitcad) by the
MitCad contributors, and borrows ideas from
[SolidCraft](https://github.com/tangency-labs/solidcraft). MitCad's
original README, with its full build, test and repository notes, is kept
in [docs/mitcad-readme.md](docs/mitcad-readme.md).

## License

MIT ([LICENSE](LICENSE)), keeping MitCad's copyright notice as the license
requires. Qt and OCCT are LGPL and linked dynamically. License policy:
[docs/development.md](docs/development.md#licence-policy).
