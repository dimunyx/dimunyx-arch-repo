<div align="center">
  <br>
  <img src="assets/arch.svg" width="50" height="50" alt="Home" />
  <br>
  <br>

  **Personal package repository for Arch Linux**

  <br>

  [![License](https://img.shields.io/badge/license-MIT-blue.svg?style=flat-square)](LICENSE)
  [![Arch Linux](https://img.shields.io/badge/Arch_Linux-1793D1?style=flat-square&logo=arch-linux&logoColor=white)](https://archlinux.org)
  [![Maintained](https://img.shields.io/badge/maintained-yes-green?style=flat-square)](https://github.com/dimunyx)
  [![Last Update](https://img.shields.io/github/last-commit/dimunyx/dimunyx-arch-repo?style=flat-square&color=blueviolet)](https://github.com/dimunyx/dimunyx-arch-repo/commits/main)

  <br>
  <br>
</div>

---

## 🚀 About

This repository contains **prebuilt packages** and **PKGBUILD** files for Arch Linux.
Packages can be installed directly via `pacman` by adding the repository to your configuration.

---

## 📋 Package List

| Package | Version | Description |
|---------|---------|-------------|
| —       | —       | *Coming soon...* |

> 🛠 Packages are being built. Stay tuned!

---

## 🔧 Installation

### 1️⃣ Add the repository

Append the following lines to `/etc/pacman.conf`:

```ini
[dimunyx-arch-repo]
SigLevel = Optional TrustAll
Server = https://github.com/dimunyx/dimunyx-arch-repo/raw/main
```

### 2️⃣ Sync package databases

```bash
sudo pacman -Sy
```

### 3️⃣ Install a package

```bash
sudo pacman -S <package-name>
```

---

## 🏗 Building Packages

```bash
git clone https://github.com/dimunyx/dimunyx-arch-repo.git
cd dimunyx-arch-repo
makepkg -si
```

---

## 📂 Repository Structure

```
.
├── pkg/          # Prebuilt packages (.pkg.tar.zst)
├── PKGBUILD/     # Build scripts
├── repo-add      # Repository database
└── README.md     # This file
```

---

## 📄 License

Distributed under the **MIT** license. See [LICENSE](LICENSE) for details.

---

<div align="center">
  <br>
  <sub>Made with ❤️ for the Arch Linux community</sub>
  <br>
  <br>
</div>
