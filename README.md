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

This repository contains packages made by dimunyx, and **PKGBUILD** files for Arch Linux. \
Packages can be installed directly via `pacman` by adding the repository to your pacman configuration file.

---

## Package List

| Package | Version | Description |
|---------|---------|-------------|
| dim-ls | 0.0.2-1 | dimunyx's ls fork written on cpp |

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
sudo pacman -Syy
```

### 3 Install a package

```bash
sudo pacman -S <package-name>
```

---

## Building Packages

```bash
git clone https://github.com/dimunyx/dimunyx-arch-repo.git
cd dimunyx-arch-repo
makepkg -si
```

---

## Repository Structure

```
.
├── pkg/          # Prebuilt packages (.pkg.tar.zst)
├── PKGBUILD/     # Build scripts
├── repo-add      # Repository database
└── README.md     # This file
```
