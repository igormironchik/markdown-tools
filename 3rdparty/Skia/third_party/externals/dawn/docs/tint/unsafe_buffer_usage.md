# Remaining -Wunsafe-buffer-usage Suppressions in Tint

This document summarizes and explains the remaining
`-Wunsafe-buffer-usage` and `-Wunsafe-buffer-usage-in-container`
suppressions in the Tint codebase.

## Categorization of Suppressions

### 1. Localization via `tint::Copy`
The `tint::Copy` utility in `src/tint/utils/memory/copy.h`
encapsulates `memcpy` calls.
- **Rationale**: `memcpy` is intrinsically unsafe but necessary for
  performance-critical byte copying. By using `tint::Copy`, we
  localize the suppressions to a single file and provide a safer API
  that accepts `std::span`.
- **Affected Files**: `src/tint/utils/memory/copy.h`

### 2. Two-Parameter `std::span` Constructors
Clang's `-Wunsafe-buffer-usage-in-container` warning is triggered by
`std::span(ptr, size)`.
- **Rationale**: This is often unavoidable when interfacing with older
  APIs (like `std::string_view::data()`) or when reinterpreting byte
  buffers (e.g., `std::span<std::byte>` to `std::span<uint8_t>`). We
  prefer this over raw pointer arithmetic as it still provides a
  bound-checked view for subsequent operations.
- **Affected Files**:
  - `src/tint/utils/bytes/buffer_reader.h`
  - `src/tint/utils/text/unicode.cc`

### 3. Tight Loops and Performance-Critical Indexing
In some performance-critical code (like UTF-8 decoding), we use
`UNSAFE_BUFFER_USAGE_IN_CONTAINER` to allow standard indexing after
manual bounds checks.
- **Rationale**: While `span::at()` or other methods might be safer,
  standard indexing `[]` is preferred in certain Tint utilities for
  consistency and to avoid the overhead of redundant checks where we
  have already validated the size.
- **Affected Files**:
  - `src/tint/utils/text/unicode.cc`
  - `src/tint/utils/containers/vector.h`

### 4. External API Interfacing
Code that interfaces with C-style external APIs (like
`std::from_chars` or terminal IO) requires raw pointers.
- **Rationale**: `std::from_chars` requires a pointer to
  one-past-the-end of the buffer. This pattern is inherently flagged
  but necessary to use the standard library feature.
- **Affected Files**:
  - `src/tint/lang/wgsl/reader/parser/lexer.cc` (for `line_end()`)
  - `src/tint/utils/system/terminal_posix.cc`

### 5. Custom Containers
Some foundational Tint containers (`Vector`, `HashMap`,
`BlockAllocator`) still contain suppressions.
- **Rationale**: These are complex, low-level memory management
  utilities where unsafe buffer usage is part of the implementation of
  the container itself. Refactoring these requires deep architectural
  changes beyond the scope of this effort.
- **Affected Files**:
  - `src/tint/utils/containers/vector.h`
  - `src/tint/utils/containers/hashmap_base.h`
  - `src/tint/utils/memory/block_allocator.h`
  - `src/tint/utils/memory/bump_allocator.h`
