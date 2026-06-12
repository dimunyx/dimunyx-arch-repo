<div align="center">
  <br>
  <img src="https://raw.githubusercontent.com/archlinux/archlinux/master/logo/archlinux-logo-dark-90dpi.png" alt="Arch Linux" width="120"/>
  <br>
  <br>

  # 📦 dimunyx-arch-repo

  **Персональный репозиторий пакетов для Arch Linux**

  <br>

  [![License](https://img.shields.io/badge/license-MIT-blue.svg?style=flat-square)](LICENSE)
  [![Arch Linux](https://img.shields.io/badge/Arch_Linux-1793D1?style=flat-square&logo=arch-linux&logoColor=white)](https://archlinux.org)
  [![Maintained](https://img.shields.io/badge/maintained-yes-green?style=flat-square)](https://github.com/dimunyx)
  [![Last Update](https://img.shields.io/github/last-commit/dimunyx/dimunyx-arch-repo?style=flat-square&color=blueviolet)](https://github.com/dimunyx/dimunyx-arch-repo/commits/main)

  <br>
  <br>
</div>

---

## 🚀 О репозитории

Данный репозиторий содержит **собранные пакеты** и **PKGBUILD** для Arch Linux.
Пакеты можно установить напрямую через `pacman`, добавив репозиторий в конфигурацию.

---

## 📋 Список пакетов

| Пакет | Версия | Описание |
|-------|--------|----------|
| —     | —      | *Скоро будет пополнение...* |

> 🛠 Пакеты находятся в процессе сборки. Следите за обновлениями!

---

## 🔧 Установка

### 1️⃣ Добавление репозитория

Добавьте следующие строки в конец файла `/etc/pacman.conf`:

```ini
[dimunyx-arch-repo]
SigLevel = Optional TrustAll
Server = https://github.com/dimunyx/dimunyx-arch-repo/raw/main
```

### 2️⃣ Обновление списка пакетов

```bash
sudo pacman -Sy
```

### 3️⃣ Установка пакета

```bash
sudo pacman -S <имя-пакета>
```

---

## 🏗 Сборка пакетов

```bash
git clone https://github.com/dimunyx/dimunyx-arch-repo.git
cd dimunyx-arch-repo
makepkg -si
```

---

## 📂 Структура репозитория

```
.
├── pkg/          # Собранные пакеты (.pkg.tar.zst)
├── PKGBUILD/     # Скрипты сборки
├── repo-add      # База данных репозитория
└── README.md     # Этот файл
```

---

## 📄 Лицензия

Распространяется под лицензией **MIT**. Подробнее — в файле [LICENSE](LICENSE).

---

<div align="center">
  <br>
  <sub>Сделано с ❤️ для Arch Linux сообщества</sub>
  <br>
  <br>
</div>
