<div align="center">
   <br>
   <img
       src="assets/arch.svg"   
       width="50"
       height="50"
       alt="Home"
   />
   <br>
   <br>

   **Personal package repository for Arch Linux**

   <br>

   [![Arch Linux](https://img.shields.io/badge/Arch_Linux-1793D1?style=flat-square&logo=arch-linux&logoColor=white)](https://archlinux.org)
   [![Maintained](https://img.shields.io/badge/maintained-yes-green?style=flat-square)](https://github.com/dimunyx)
   [![Last Update](https://img.shields.io/github/last-commit/dimunyx/dimunyx-arch-repo?style=flat-square&color=blueviolet)](https://github.com/dimunyx/dimunyx-arch-repo/commits/main)

   <br>
   <br>
</div>

## About

This repository contains packages made by dimunyx, and **PKGBUILD** for Arch Linux. \
Packages can be installed directly via `pacman` by adding the repository to your pacman configuration. \
P.S: Soon will be added manual install via `compilin`.

---

## Package List

| Package | Version |
|---------|---------|
| dim-ls  | 0.0.6-1 |

---

## Installation

### 1 Add the repository

Put the following lines to `/etc/pacman.conf`:

```ini
[dimunyx-arch-repo]
SigLevel = Optional TrustAll
Server = https://raw.githubusercontent.com/dimunyx/dimunyx-arch-repo/main/pkg
```

### 2 Sync package databases

```bash
sudo pacman -Sy
```

### 3 Install a package

```bash
sudo pacman -S <package-name>
```

## Support
### If you have any problems/issues with repository don't be shy and tell me the error [here](https://mail.google.com/mail/u/0/#inbox?compose=CllgCJNvvRJqDnqmwXRJKDghCRdrssCrWcmJDXHxNDgDqNpvsVtDfPnPmVLHXTXgRnKrgKlFsFL)
