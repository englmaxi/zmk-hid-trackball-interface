# ZMK trackball interface using HID indicators

This ZMK module allows interfacing with a trackball that can't run ZMK.
The state of the HID indicator LEDs is shared between input devices so they can be used for communication.
This module provides macros to send commands to the trackball and implements a listener to enable a mouse layer when movement is detected.

## Usage

To use, first add this module to your `config/west.yml` by adding a new entry to `remotes` and `projects`:

```yaml west.yml
manifest:
  remotes:
    - name: zmkfirmware
      url-base: https://github.com/zmkfirmware
    - name: englmaxi
      url-base: https://github.com/englmaxi
  projects:
    - name: zmk
      remote: zmkfirmware
      revision: main
      import: app/west.yml
    - name: zmk-hid-trackball-interface
      remote: englmaxi
      revision: main
  self:
    path: config
```

Then add the following to your keymap:
```c
#include <interfaces/hid-trackball.dtsi>
```

This gives you access to the following predefined behaviors:
- `&tb_cyc_dpi`: Cycles between the different DPI options of the trackball
- `&tb_bootloader`: Reboots the trackball into the bootloader
- `&tb_tg_scroll`: Toggles the trackball between move- and scroll-mode
- `&tb_mo_scroll`: Toggles the trackball between move- and scroll-mode while the key is held down
- `&tbs_mt 0 0`: `&tb_tg_scroll` on tap, `&tb_mo_scroll` on hold

If you want to automatically change to a layer or enable scrolling and change DPI on specific layers, add this (with the desired layer inside `<>`) to your keymap:
```dtsi
&hid_trackball_interface {
    automouse-layer = <1>;
    scroll-layers = <2>;
    snipe-layers = <3>;
};
```

- If a layer is defined in `automouse-layer`, it will be enabled while the mouse is moving
- If any layers are defined in `scroll-layers`, `&tb_tg_scroll` is executed by default when one of those layers is enabled or disabled.
- If any layers are defined in `snipe-layers`, `&tb_cyc_dpi` is executed by default when one of those layers is enabled or disabled
  (this is only usefull if it cycles between two DPI settings only)

If you use a [Ploopy Nano](https://github.com/ploopyco/nano-trackball), you can use the modified firmware in [trackball_firmware](/trackball_firmware).
Otherwise, you need to modify your firmware to listen to [specific HID indicator changes](#how-does-this-work?).

## How does this work?

The idea is based on the `lkbm` keymap for the Ploopy Nano, [created by @aidalgol](https://github.com/qmk/qmk_firmware/pull/17218).
It allows sending 2-bit commands by turning NLCK (`0b01`) and CLCK (`0b10`) on and off within a specified short time (<25 ms):
- `0b01` ... toggle scroll-mode
- `0b10` ... cycle DPI
- `0b11` ... bootloader

To enable the `automouse-layer`, the trackball keymap was extended to turn on SLCK while the mouse is moving, which gets detected by this module.

---

### Acknowledgements

Inorichi's [PMW3610 driver](https://github.com/inorichi/zmk-pmw3610-driver) and the original [`lkbm`](https://github.com/qmk/qmk_firmware/pull/17218) firmware 
were major inspirations for implementing this module.
