# software-rasterizer-fltk

[![CI](https://github.com/Wil12Fred/software-rasterizer-fltk/actions/workflows/ci.yml/badge.svg)](https://github.com/Wil12Fred/software-rasterizer-fltk/actions/workflows/ci.yml) [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A small software rasterizer written from scratch in C++: it draws a 3-D scene into an
FLTK window pixel by pixel — no OpenGL — implementing the classic pipeline stages by hand,
from Bresenham lines to clipping, scan-line fill, depth buffering and lighting.

![Running on Linux](docs/screenshot.png)

*Running on Linux. The earlier macOS captures of each stage are in `docs/`:
[Bresenham](docs/Bresenham.png), [ScanLine](docs/ScanLine.png),
[Clipping](docs/Clipping.png), [Polygon clipping](docs/ClippingDePoligono.png),
[Perspective](docs/Perspective.png), [Depth](docs/Depth.png),
[Interpolation](docs/Interpolation.png), [Illumination](docs/Ilumination.png).*

## Features

| Algorithm | Where |
|---|---|
| Bresenham line drawing | `struct Bresenham`, `renderer/geometry.h` |
| Scan-line polygon fill (active edge table) | `struct ScanLine`, `renderer/geometry.h` |
| Cohen–Sutherland line clipping | `struct CohenSutherland`, `renderer/geometry.h` |
| Sutherland–Hodgman polygon clipping | `struct SutherlandHodgeman`, `renderer/geometry.h` |
| Perspective projection, transformations | `struct Transformations`, `renderer/geometry.h`, `math/euclidean/` |
| Depth buffer, color interpolation, lighting | `renderer/geometry.h`, `renderer/triangle.cpp`, `renderer/polygon.cpp`, `renderer/renderer.cpp` |

- The scene — background, camera, ambient/point lights, nested transformations and
  objects — is read from a text file (`data/*.scn`).
- Mouse control: drag with the left button to rotate the camera, with the right button
  to move it forward and back.

## Requirements

| | Debian / Ubuntu | Arch Linux |
|---|---|---|
| compiler, make | `g++ make` | `gcc make` |
| FLTK 1.3 | `libfltk1.3-dev` | `fltk` (Arch ships FLTK 1.4; only 1.3 is tested) |
| tests (headless run) | `xvfb imagemagick` | `xorg-server-xvfb imagemagick` |

## Build

```sh
make                                   # uses fltk-config from PATH
make FLTK_CONFIG=/path/to/fltk-config  # or an FLTK installed elsewhere
```

## Run

Run from the repository root (the scene path `./data/test-cubo.scn` is relative):

```sh
./raylite
```

It prints nothing on the console; it opens a window with the cube scene (screenshot
above). The headless check in `make test` measured 5901 non-background pixels in that
window on the CI runner.

The background colour comes from the scene's `[background]` block. `data/test-cubo.scn`,
the scene `raylite.cpp` loads, sets it to white (`rgb 1.0 1.0 1.0`); `data/test.scn`, the
two-triangle scene of the early stage captures in `docs/`, sets it to black. That is why
the Linux screenshot is white and most macOS captures are black — not a platform
difference. To render `test.scn`, change the path in `raylite.cpp`.

## Tests

```sh
make unit     # geometry/math unit tests only (no FLTK needed)
make test     # unit tests + the real program under xvfb
```

- `tests/test_geometry.cpp` — Bresenham lines into all eight octants (both endpoints,
  one pixel per major-axis step, 8-connected, at most half a pixel from the ideal line);
  Cohen–Sutherland accepts an inside line unchanged, rejects an outside one and clips
  crossing lines onto the window boundary along the original line; scan-line fill of a
  rectangle and of a right triangle gives exactly the expected pixel sets; a square
  clipped by Sutherland–Hodgman stays inside the window and still fills the visible part;
  cross product, and `M · invert(M) = I` for a translate·rotate·scale transformation;
  translation applied to a point.
- `tests/smoke.sh` — starts `raylite` on a private `xvfb-run` display (never the
  caller's), checks it is still running after 4 s, and takes a screenshot that must
  contain the rendered cube (pixels other than the white background).

## Project layout

| Path | Contents |
|---|---|
| `raylite.cpp` | entry point |
| `renderer/` | engine, scene parser, camera, viewport (FLTK widget), rasterizer |
| `renderer/geometry.h` | the 2-D algorithms above |
| `math/numerical/`, `math/euclidean/` | vector/matrix templates, inversion, transformations |
| `structures/` | a small stack template |
| `data/*.scn` | scene files |
| `tests/` | unit tests and the headless run |

## Known limitations

- Only FLTK 1.3 is tested; the code uses the `register` keyword (built with
  `-Wno-register`).
- The scene file is fixed in `raylite.cpp`; there is no command-line argument.
- `Color` allocates a new `std::vector` on every `set`, and nothing frees it, so the
  renderer leaks memory on every frame.
- `renderer/parser2.h` is an unused variant of the parser, and `renderer/polygon.cpp`
  still prints debug coordinates.
- `invert()` does not check for singular matrices.

## Background

Final project for a computer graphics course at university, 2018.

## License

MIT — see [LICENSE](LICENSE).
