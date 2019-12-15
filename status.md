# API Status
| __Graphics__ | implemented? | tests? | notes |
| --- | --- | --- | --- |
| `camera([x,] [y])` | ✔ | ✔ | |
| `circ(x, y, r, [col])` | ✔ |  | |
| `circfill(x, y, r, [col])` | ✔ | | |
| `clip([x,] [y,] [w,] [h])` | ✔ | | |
| `cls()` | ✔ | | |
| `color(col)` | ✔ | | |
| `cursor([x,] [y,] [col])` | ✔ | | |
| `fget(n, [f])` | ✔ | | |
| `fillp([pat])` |  | | |
| `fset(n, [f,] [v])` | ✔ | | |
| `line(x0, y0, x1, y1, [col])` | ✔ | | |
| `pal([c0,] [c1,] [p])` | ✔ | | |
| `palt([c,] [t])` | ✔ | | |
| `print(str, [x,] [y,] [col])` | ✔ | | |
| `pset(x, y, [c])` | ✔ | | |
| `rect(x0, y0, x1, y1, [col])` | ✔ | | |
| `rectfill(x0, y0, x1, y1, [col])` | ✔ | | |
| `spr(n, x, y, [w,] [h,] [flip_x,] [flip_y])` | ✔ | | |
| `sset(x, y, [c])` | ✔ | | |
| `sspr(sx, sy, sw, sh, dx, dy, [dw,] [dh,] [flip_x,] [flip_y])` | ✔ | | missing flip |

# Tables
All functions implemented but not thoroughly tested

# Input
Only for 1 player
| `btn([i,] [p])` | ✔ | | |
| `btnp([i,] [p])` | ✔ | | not working as intended |

# Sound
Still lacking support.

# Map
| `map(cel_x, cel_y, sx, sy, cel_w, cel_h, [layer])` | ✔ |  | |
| `mget(x, y)` | ✔ | | |
| `mset(x, y, v)` | ✔ | | |

# Math
Missing just `atan2`. No support for fixed point arithmetic, `float` is used at the moment.

# Memory
No function implemented yet but everything is already mapped to its correct address.

# Cartridge data
`cartdata`, `dget` and `dset` implemented but no persistence between sessione.