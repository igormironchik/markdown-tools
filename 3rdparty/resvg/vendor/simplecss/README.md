## simplecss
![Build Status](https://github.com/RazrFalcon/simplecss/workflows/simplecss/badge.svg)
[![Crates.io](https://img.shields.io/crates/v/simplecss.svg)](https://crates.io/crates/simplecss)
[![Documentation](https://docs.rs/simplecss/badge.svg)](https://docs.rs/simplecss)
[![Rust 1.37+](https://img.shields.io/badge/rust-1.37+-orange.svg)](https://www.rust-lang.org)
![](https://img.shields.io/badge/unsafe-forbidden-brightgreen.svg)

A simple [CSS 2.1](https://www.w3.org/TR/CSS21/) parser and selector.

This is not a browser-grade CSS parser. If you need one,
use [cssparser](https://crates.io/crates/cssparser) +
[selectors](https://crates.io/crates/selectors).

Since it's very simple we will start with limitations:

### Limitations

- [At-rules](https://www.w3.org/TR/CSS21/syndata.html#at-rules) are not supported.
  They will be skipped during parsing.
- Property values are not parsed.
  In CSS like `* { width: 5px }` you will get a `width` property with a `5px` value as a string.
- CDO/CDC comments are not supported.
- Parser is case sensitive. All keywords must be lowercase.
- Unicode escape, like `\26`, is not supported.

### Features

- Selector matching support.
- The rules are sorted by specificity.
- `!import` parsing support.
- Has a high-level parsers and low-level, zero-allocation tokenizers.
- No unsafe.

### License

Licensed under either of

- Apache License, Version 2.0
  ([LICENSE-APACHE](LICENSE-APACHE) or http://www.apache.org/licenses/LICENSE-2.0)
- MIT license
  ([LICENSE-MIT](LICENSE-MIT) or http://opensource.org/licenses/MIT)

at your option.

### Contribution

Unless you explicitly state otherwise, any contribution intentionally submitted
for inclusion in the work by you, as defined in the Apache-2.0 license, shall be
dual licensed as above, without any additional terms or conditions.
