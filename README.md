
<p align="center">
  <img src="docs/assets/calendarrr-outlined.svg" alt="Calendarrr Logo" width="150" />
</p>

<h1 align="center">Calendarrr</h1>

<p align="center">
  <b> A minimal calendar application built using GTK for XFCE </b>
</p>



<p align="center">
  <img src="https://img.shields.io/badge/Platform-Linux-blue" />
  <img src="https://img.shields.io/badge/Built_with-C++-blueviolet" />
  <img src="https://img.shields.io/badge/UI-GTK-8bc34a" />
  <img src="https://img.shields.io/badge/License-GPL%20v3-blue.svg" />
</p>


---

## Overview

A minimal calendar application for XFCE desktops that allows users to manage daily notes and events.

### Key Features
- **Daily Notes:** Save and load notes for any specific date.
- **Global Search:** Find notes by content and jump directly to the date.
- **Visual Indicators:** Days with saved notes are visually marked on the calendar.
- **Export/Import:** Backup your notes as text files or import them from existing data.
- **Note Management:** Easily delete notes with a safety confirmation dialog.
- **Quick Navigation:** Jump back to today's date with a single click.
- **Lightweight:** Built with GTK3 and C++ for minimal resource usage.

## Setup

To use calendarrr, follow these steps:

1. Install the required dependencies:  

    Distro | Package Name | Annotation
    --- | --- | ---
    Arch Linux | `gtk3` | Install using `sudo pacman -S gtk3`
    Debian 10 or later | `libgtk-3-dev` | Install using `sudo apt-get install libgtk-3-dev`
    Fedora | `gtk3-devel` | Install using `sudo dnf install gtk3-devel`

2. Clone the repository:
   ```bash
   git clone https://github.com/maskedsyntax/calendarrr.git
   cd calendarrr
   ```
3. Run the setup script:
    ```bash
    ./setup.sh
    ```
4. Launch the application:  
You can now find `Calendarrr` in your application menu or run it from the terminal:
    ```bash
    calendarrr
    ```

## License

This project is licensed under the [GNU General Public License v3](LICENSE).

## Contribute

Pull requests, issues, and suggestions are always welcome!  
