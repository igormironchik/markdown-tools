[![License: GPL](https://img.shields.io/badge/license-GPLv3-blue
)](https://opensource.org/license/GPL-3.0)

* [About](#about)
* [License](#license)
* [Installers](#installers)
* [Building](#building)
  * [On what platforms better use `Conan` to build `markdown-tools`?](#on-what-platforms-better-use-conan-to-build-markdown-tools)
* [Known issues](#known-issues)

# About

This is a set of applications to work with `Markdown`, including editor, `HTML`
preview viewer, converter to `PDF`.


| ![](doc/editor.png) | ![](doc/converter.png) | ![](doc/viewer.png) |
| --- | --- | --- |

# License

```
/*
    SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/
```

# Installers

You can get installers for `Windows` and `Linux` at
[actions page](https://github.com/igormironchik/markdown-tools/actions). Just check
for artifact at corresponding `deploy_linux` or `deploy_win` latest workflow run.
But for `Linux`, espcially for `KDE`, I'd recommend to use your own build with system
`Qt`, in this case you won't have issues with platform integration in `KDE`.

# Building

To build these applications first of all install dependencies:

 * `openssl`
 * `freetype`
 * `libjpeg`
 * `libpng`
 * `zlib`
 * `libxml2`
 * `fontconfig`
 * `extra-cmake-modules`
 * `kf6-syntax-highlighting`
 * `Qt >= 6.7`

With these dependencies installed just open `CMakeLists.txt` in `QtCreator`
(or use `CMake` from command line) and run build.

You can use `Conan` to install some dependencies.

`Fontconfig` should be installed in system, as using `Fontconfig` from `Conan` leads to ugly UI.

One more thing - don't use `jom`.

On `KDE` I suggest to use system's `Qt` libraries, as in this case applications will use
`KDE`'s Platform Plugin, that makes better integration with system.

## On what platforms better use `Conan` to build `markdown-tools`?

I don't suggest to use `Conan` on MacOS, there will be conflicts with `brew` and system libraries,
as I suggest to use `Fontconfig` from system. There will be problems. Just use `brew` on MacOS to
install dependencies.

On Windows `Conan` is a fresh wind to build this project.

On Linux you can use what you want, that is why this is my favorite OS.

# Known issues

* Don't use `HTML` attribute `class` in `HTML` tags, it can lead to wrongly rendered content.
`GitHub` do a magick in this case, it just deletes `class` attribute, but this editor places
`HTML` tags as they present.

* Strange behaviour of font combo box in fonts dialog on check in/out check box to constraint
fonts to monospaced due to [QTBUG-112145](https://bugreports.qt.io/browse/QTBUG-112145)

* I do not render HTML tags in PDF.

* Some LaTeX Math expressions can be possibly wrongly rendered in PDF. I use very good
library `JKQtPlotter` to render LaTeX Math, and not everything is
implemented there (possibly). But most common math things are done.

* I don't support languages that don't separate words
with spaces in converter to `PDF`.

* `LaTeX` math expressions can be rendered a little differently in PDF on different platforms
due to [QTBUG-104790](https://bugreports.qt.io/browse/QTBUG-104790), that is why I don't
have auto tests for them.
