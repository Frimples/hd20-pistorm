# Disk-image notes

The host backend accepts raw sector images up to exactly 2 GiB:

```text
2 * 1024 * 1024 * 1024 = 2147483648 bytes
```

The image must be sector aligned. The blank 2 GiB test image is only a storage-boundary artifact; it contains no Macintosh filesystem and is not bootable.

The uploaded `HD_20_Startup.img` is a separate 409,600-byte standard Macintosh MFS floppy. It contains HD20 utilities and a System Folder and is intended to boot from the physical floppy drive. It is not a prepared 2 GiB HD20 backing image and must not be renamed or represented as one.

Do not commit ROMs or copyrighted disk images to this repository. Reproduction requires the user to supply legally obtained media and ROM files separately.
