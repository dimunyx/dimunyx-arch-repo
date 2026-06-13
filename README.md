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
   [![Last Update](https://img.shields.io/github/last-commit/dimunyx/dimunyx-arch-repo?style=flat-square&color=blueviolet)](https://github.com/dimunyx/dimunyx-arch-repo/commits/main)

   <br>
   <br>
</div>

## About

This repository contains packages made by dimunyx, and **PKGBUILD** for Arch Linux. \
Packages can be installed directly via `pacman` by adding the repository to your pacman configuration. \
P.S: Soon will be added manual installation via `compiling`.

---

## Packages

| Package | Version | Description | Optimizations |
|----------|---------|-------------|---------------|
| dim-ls | 0.1.1-3 | A ls fork written on C++ with icons and colors | O3 |
| dim-ls-lto | 0.1.1-3 | A ls fork written on C++ with icons and colors | O3 + LTO |
| dimfetch | 0.3-3 | Minimalistic fetch written in C++ | O3 |
| dimfetch-lto | 0.3-3 | Minimalistic fetch written in C++ | O3 + LTO |

---

## Installation

### 1 Import the GPG key

```bash
curl -sS https://raw.githubusercontent.com/dimunyx/dimunyx-arch-repo/main/repo/x86_64/dimunyx-arch-repo.pubkey | sudo pacman-key --add - && sudo pacman-key --lsign-key 09D90FC946B01EFE
```

### 3 Add the repository

Put the following lines to `/etc/pacman.conf`:

```ini
[dimunyx-arch-repo]
SigLevel = Required DatabaseOptional
Server = https://raw.githubusercontent.com/dimunyx/dimunyx-arch-repo/main/repo/x86_64
```

### 3 Sync package databases

```bash
sudo pacman -Syy
```

### 4 Install a package

```bash
sudo pacman -S <package-name>
```

## Support
If you have any problems/issues with repository don't be shy and tell me the error [here](https://mail.google.com/mail/u/0/#inbox?compose=CllgCJNvvRJqDnqmwXRJKDghCRdrssCrWcmJDXHxNDgDqNpvsVtDfPnPmVLHXTXgRnKrgKlFsFL)
