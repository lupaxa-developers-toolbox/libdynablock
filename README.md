<p align="center">
  <a href="https://github.com/lupaxa-developers-toolbox">
    <img src="https://raw.githubusercontent.com/the-lupaxa-project/brand-assets/master/logos/organisations/developers-toolbox/readme-logo.png" alt="Developers Toolbox" />
  </a>
</p>

<h1 align="center">Libdynablock</h1>

C library for a pool of zeroed allocations.

Each pool owns the blocks it hands out. A block is one allocation: a private
header, then a payload of the exact requested size. The payload is aligned
for any object and starts out zero. Release frees one block. Destroy frees
every block still in the pool. The pool type is opaque. The public header is
`include/dynablock.h`. It uses C linkage, so a C++ translation unit can
include it.

## Requirements

- GNU make.
- gcc, unless `CC` is set. The C skill uses gcc for syntax checks.
- cppcheck, for `make c-check`.

## Build

`make lib` writes the static archive `build/libdynablock.a`. The archive is
built as C11 with `-Wall -Wextra -Wpedantic -Werror`. The compiler is gcc
unless `CC` is set. `make clean` removes `build/`.

```bash
make lib
cc -std=c11 -Iinclude -o demo demo.c build/libdynablock.a
```

`make lib CC=clang` selects the compiler. `make lib CFLAGS=-g` adds flags to
that C11 build. `make` with no target prints the makefile help.

`make update` refreshes the makefile skills. `make lib` and `make test` do
not need that checkout. `make c-check` and `make version` do.

## Quick Start

Create a pool, take a 16-byte block, write a string, and destroy the pool.
Destroy frees the block.

```c
#include "dynablock.h"

#include <string.h>

int main(void)
{
    dynablock *pool = NULL;
    char *text = NULL;

    if (dynablock_create(&pool) != DYNABLOCK_OK) {
        return 1;
    }

    if (dynablock_get(pool, 16, (void **)&text) != DYNABLOCK_OK) {
        dynablock_destroy(&pool);
        return 1;
    }

    memcpy(text, "hello", 6);
    dynablock_destroy(&pool);
    return 0;
}
```

## Ownership

The pool owns every payload from `dynablock_get` until `dynablock_release`
or `dynablock_destroy`. The caller does not `free` that pointer.

`dynablock_release` frees one payload that belongs to that pool. A pointer
from another pool, or a pointer the pool does not own, returns
`DYNABLOCK_INVAL` and frees nothing. The pointer is invalid after a
successful release.

`dynablock_destroy` frees every block still in the pool, frees the pool, and
sets the caller's pointer to NULL. Pointers into that pool are then invalid.

> **Note:** Callers serialise use of a given pool.

## Return Codes

Every function that can fail returns an `int`.

| Code              | Value | Meaning                                                                              |
| ----------------- | ----- | ------------------------------------------------------------------------------------ |
| `DYNABLOCK_OK`    | `0`   | The call succeeded.                                                                  |
| `DYNABLOCK_NOMEM` | `-1`  | Allocation failed. The pool is unchanged. Create sets `*pool` to NULL.               |
| `DYNABLOCK_INVAL` | `-2`  | The pool or out-pointer is missing, the size is 0, or the block is not in that pool. |

`dynablock_destroy(NULL)` returns `DYNABLOCK_INVAL`. `dynablock_destroy` on a
pointer that is already NULL returns `DYNABLOCK_OK` and leaves it NULL.

A failed `dynablock_get` leaves `*block` unchanged.

## Functions

```c
typedef struct dynablock dynablock;

int dynablock_create(dynablock **pool);
int dynablock_destroy(dynablock **pool);
int dynablock_get(dynablock *pool, size_t bytes, void **block);
int dynablock_release(dynablock *pool, void *block);
size_t dynablock_count(const dynablock *pool);
```

| Name                | Purpose                                                         |
| ------------------- | --------------------------------------------------------------- |
| `dynablock_create`  | Allocate an empty pool.                                         |
| `dynablock_destroy` | Free every block still owned, free the pool, and clear `*pool`. |
| `dynablock_get`     | Allocate a zeroed payload of exactly `bytes` bytes.             |
| `dynablock_release` | Free one payload that belongs to this pool.                     |
| `dynablock_count`   | Return the number of blocks still owned. A NULL pool returns 0. |

`bytes` is the number of bytes the caller can use. The payload address is
aligned for any object. A request of 0 returns `DYNABLOCK_INVAL`.

Two pools do not share blocks. Release a block on the pool that allocated it.

## Empty Pool

A new pool has a count of 0. Destroying it returns `DYNABLOCK_OK` and sets
the caller's pointer to NULL. Destroying that NULL pointer again returns
`DYNABLOCK_OK`.

`dynablock_count(NULL)` returns 0.

## Version

The version text is the macro `DYNABLOCK_VERSION_STRING` in
`include/dynablock.h`. It is a dotted semantic version. `.bumpversion.toml`
is the version source, and a bump updates that macro.

After `make init`, `make version` prints the version from
`.bumpversion.toml`.

## Development

```bash
make update
make lib
make check
make clean
```

`make c-check` syntax-checks every `.c` and `.h` file, then runs cppcheck.
`make check` runs `c-check` and then the behaviour tests. `make test` runs
only the behaviour tests.

<a href="https://github.com/the-lupaxa-project">
    <img src="https://raw.githubusercontent.com/the-lupaxa-project/brand-assets/master/logos/components/footer-for-child-orgs.svg" alt="The Lupaxa Project Footer" width="100%" />
</a>
