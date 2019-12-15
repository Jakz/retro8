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
| __Input__ | | | |
| `btn([i,] [p])` | ✔ | | 1 player only |
| `btnp([i,] [p])` | ✔ | | not working as intended, 1 player only |
| __Math__ | | | |
| | | | `atan2` only one missing |
| __Tables__ | | | |
| | | | all functions implemented, not tested |
| __Map__ | | | |
| `map(cel_x, cel_y, sx, sy, cel_w, cel_h, [layer])` | ✔ |  | |
| `mget(x, y)` | ✔ | | |
| `mset(x, y, v)` | ✔ | | |
| __Cartridge__ | | | |
| `carddata` | ✔ | | just noop for now |
| `dget` | ✔ | | |
| `dset` | ✔ | | |
