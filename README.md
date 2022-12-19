# PaleBoot
PoC tool to boot palera1n

## Why
So you can boot with palera1n on the go

## Running:
First, prepare the boot folder (see below)
<br>
Then, run:
<br>
### Linux:
> sudo ./PaleBoot
<br>
### macOS:
> ./PaleBoot
<br>
#### Add --tethered at the end of the command if using tethered.

## How to prepare:
Make a folder named boot.
<br>
Copy these files into the folder:

> All devices:
- boot/ibot.img4

> iPhone 7(+):
- other/payload/payload_t8010.bin

> iPhone 8(+):
- other/payload/payload_t8015.bin

> Other A10 and higher:
- Everything is already included in All devices

> A9(X) and lower:
- boot/iBSS.img4