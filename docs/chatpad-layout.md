# Xbox 360 Chatpad layout notes

Hardware scancodes are **position-based** (Cliffle / MS protocol). Regional
chatpads only change the **keycap printing**, not the USB codes. xboxdrv maps
scancodes to Linux `KEY_*` symbols for the base layer; Shift / Green / Orange
layers are **combinations** and are not full Unicode input yet.

Reference photo: German / Western-European QWERTZ Messenger Kit chatpad
(white legends = base, orange = Orange modifier, green = Green modifier).

## Modifiers (byte 1 of the 5-byte report)

| Bit | Mask | Key (photo) | Role |
|-----|------|-------------|------|
| 0 | `0x01` | **shift** (bottom-left of letter block; also printed CAPS in orange) | Momentary Shift for the base layer. **CAPS / sticky Shift is Orange+Shift**, not Shift alone. |
| 1 | `0x02` | **Green square** (bottom-left, lights green) | Selects green legends on keycaps. |
| 2 | `0x04` | **Orange circle** (bottom-right, lights orange) | Selects orange legends; with Shift → CAPS lock behaviour on console. |
| 3 | `0x08` | **People** (two-person icon) | People / messenger key (LED lock-style in xboxdrv today). |

xboxdrv emits Linux key codes useful on the desktop (layers are still
placeholder until compose/XKB):

- Shift → `KEY_LEFTSHIFT`
- Green → `KEY_LEFTALT`
- Orange → `KEY_LEFTCTRL`
- People → `KEY_LEFTMETA`

**Modifier / LED policy** (aligned with the Xbox 360 console manual):

- **Shift / Green / Orange / People** are **one-shot sticky**: a tap arms
  the mode and lights the LED; the next non-modifier key is sent with that
  modifier held, then the sticky mode and LED clear. Holding the key keeps
  the modifier active for as long as it is down.
- **Orange + Shift** (either order) toggles **CAPS lock**. While CAPS is on,
  Shift stays effectively down and the Shift LED stays lit until the same
  combo is pressed again.
- People is treated like the other modifiers on PC (no Messenger
  notification LED). True orange/green legend characters still need compose
  tables or an XKB keymap (see `chatpad.xkb`).

## Physical grid → scancode (hex)

Same for all regions (Cliffle). Letters below are **US** keycap labels;
German keycaps differ where noted.

```
Row 1 (numbers):     17 16 15 14 13 12 11 67 66 65
                     1  2  3  4  5  6  7  8  9  0

Row 2:               27 26 25 24 23 22 21 76 75 64
                     Q  W  E  R  T  Y  U  I  O  P
                     Q  W  E  R  T  Z  U  I  O  P   ← German labels (Y/Z swap)

Row 3:               37 36 35 34 33 32 31 77 72 62
                     A  S  D  F  G  H  J  K  L  ,

Row 4:               -- 46 45 44 43 42 41 52 53 63
                     Sh Z  X  C  V  B  N  M  .  Enter   ← US
                     Sh Y  X  C  V  B  N  M  .  Enter   ← German (Y/Z swap)

Row 5:               -- -- 55 54 -- -- 51 71 --
                     Gr Pp ←  space →  Bksp  Or
```

`Sh` = Shift (mod bit0), `Gr` = Green (bit1), `Pp` = People (bit3),
`Or` = Orange (bit2). Left/right arrows = `0x55` / `0x51`. Space = `0x54`.
Backspace = `0x71`. Enter = `0x63`.

## German / European keycap layers (from photo)

White = base (no coloured modifier). Orange / green = with that modifier held
(or locked, on console firmware). Shift applies the usual upper-case / shifted
punctuation on the base Latin letters and the top-left punctuation on number
keys. Exact Shift+Orange “CAPS” semantics are firmware-side on Xbox; on PC we
only see the modifier bits.

### Number row

| Base | Shift (typical) | Orange (photo) | Green (photo) |
|------|-----------------|----------------|---------------|
| 1 | ! | | @ |
| 2 | " | | |
| 3 | § | | |
| 4 | $ | | |
| 5 | % | | |
| 6 | & | | |
| 7 | / | | |
| 8 | ( | | |
| 9 | ) | | |
| 0 | = | | |

(Photo also shows extra orange/green glyphs on letter keys; number-row green/orange
are sparse compared to the letter block.)

### Letter rows (German labels)

| Scancode | DE base | US base | Orange (photo) | Green (photo) |
|----------|---------|---------|----------------|---------------|
| 0x27 | Q | Q | ! | @ |
| 0x26 | W | W | " | |
| 0x25 | E | E | € | é |
| 0x24 | R | R | $ | ¥ |
| 0x23 | T | T | % | þ |
| 0x22 | **Z** | Y | & | ^ |
| 0x21 | U | U | / | ü |
| 0x76 | I | I | ( | í |
| 0x75 | O | O | ) | ó |
| 0x64 | P | P | = | \ |
| 0x37 | A | A | å | ä |
| 0x36 | S | S | ß | š |
| 0x35 | D | D | « | ð |
| 0x34 | F | F | » | £ |
| 0x33 | G | G | | ¢ |
| 0x32 | H | H | { | ` |
| 0x31 | J | J | } | ø |
| 0x77 | K | K | [ | æ |
| 0x72 | L | L | ] | œ |
| 0x62 | , | , | ' | # |
| 0x46 | **Y** | Z | < | ° |
| 0x45 | X | X | > | \| |
| 0x44 | C | C | ~ | ç |
| 0x43 | V | V | - | — |
| 0x42 | B | B | * | + |
| 0x41 | N | N | ; | ñ |
| 0x52 | M | M | : | µ |
| 0x53 | . | . | ? | ¿ |

Orange on Shift keycap: **CAPS**. Enter / space / arrows / bksp are single-purpose
on this photo (no extra colour legends).

### Bottom row

| Control | Scancode / mod | Linux (current xboxdrv) |
|---------|----------------|-------------------------|
| Green square | mod `0x02` | `KEY_LEFTALT` + one-shot sticky LED |
| People | mod `0x08` | `KEY_LEFTMETA` + one-shot sticky LED |
| Left | `0x55` | `KEY_LEFT` |
| Space | `0x54` | `KEY_SPACE` |
| Right | `0x51` | `KEY_RIGHT` |
| Backspace | `0x71` | `KEY_BACKSPACE` |
| Orange circle | mod `0x04` | `KEY_LEFTCTRL` + one-shot sticky LED |
| Shift | mod `0x01` | `KEY_LEFTSHIFT` + one-shot / CAPS LED |
| Enter | `0x63` | `KEY_ENTER` |

## Implementation status in xboxdrv

- [x] Base Latin layer via scancode → `KEY_*` (US letter names in code).
- [x] Modifier bits → left Shift/Alt/Ctrl/Meta (desktop placeholder).
- [x] One-shot sticky Shift/Green/Orange/People + LEDs (console-style).
- [x] Orange+Shift CAPS lock toggle (Shift LED latched while CAPS on).
- [ ] QWERTZ vs QWERTY letter labels (Y/Z) selectable by layout.
- [ ] Orange / green character layers (need compose table or XKB).
- [ ] Other regional keycaps (UK, FR, …) documented the same way.

Other regions ship different keycaps; capture a photo and extend this file
rather than assuming DE glyphs are universal.

## Protocol reminder

Report (interrupt IN, 5 bytes):

```
00 | modifier | key1 | key2 | 00
```

Up to two keys + modifier nibble per report. Keep-alives are separate control
transfers (`0x1e` / `0x1f` on interface 2), not key events.
