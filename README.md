# axefx-usb-reset

A minimal macOS command-line tool that forces the **Fractal Audio Axe-Fx III** to re-enumerate on the USB bus, restoring audio when the device disappears after a system sleep or wake cycle.

## Problem

On macOS, the Axe-Fx III USB audio interface can stop working in two situations:

- **After system sleep/wake** — the device is still physically connected, but Core Audio no longer sees it.
- **After powering the Axe-Fx III on** — audio freezes or becomes unresponsive because macOS did not properly re-initialize the USB connection.

The usual fix — unplugging and re-plugging the cable — works, but is tedious. This tool does the same thing in software, without touching any cables.

## How it works

It uses the IOKit USB API to locate the Axe-Fx III by its USB Vendor ID (`0x2466`) and Product ID (`0x8010`), opens the device, and calls `USBDeviceReEnumerate`. This triggers macOS to tear down and rebuild the USB device stack, after which Core Audio rediscovers the audio interface within a few seconds.

## Requirements

- macOS 12 or later (Monterey+)

## Pre-built binary

A universal binary (arm64 + x86_64) is included in the repository. No build tools required — just install it:

```sh
make install
```

This copies `axefx-usb-reset` to `~/.local/bin/`.

## Build from source

Requires Xcode Command Line Tools (`xcode-select --install`).

```sh
make
```

Produces a universal binary targeting macOS 12+.

## Usage

```sh
# Re-enumerate the Axe-Fx III USB device (default)
axefx-usb-reset

# Kill coreaudiod — launchd restarts it automatically (may require sudo)
sudo axefx-usb-reset -k

# Do both at once
sudo axefx-usb-reset -a
```

| Option | Long form | Effect |
|---|---|---|
| *(none)* | | USB re-enumerate only |
| `-k` | `--kill-coreaudio` | Kill coreaudiod only |
| `-a` | `--all` | USB re-enumerate + kill coreaudiod |

Killing `coreaudiod` requires `sudo` because the process runs under a system account. `launchd` restarts it automatically within a second.
