
# Z80 pc Open-source Emulator

This project is an emulatior of a custom Z80-based PC, featuring modules for GPU, I/O, and Memory management. It enables programming of GPU and I/O modules, simulates storage and timer behavior, and handles CPU execution and debugging.

The emulator is written in C++ and relies on multithreading and modern object-oriented design for module interaction.


## Features

#### GPU Emulation:

- Simulates a GPU module for character display and graphics rendering.
- Mutex-protected command processing.

#### I/O Module:

- Simulates input/output operations and HID interaction.
- Generates and handles interrupts.
- Manages keyboard and storage device operations.

#### Memory Module:

- Simulates a paged memory structure with customizable ROM and RAM.
- Supports program loading for emulation startup.

#### CPU Integration:

- Implements Z80 CPU emulation.
- Debugging support with logging.
- Breakpoint and callback handling available.

#### Interactive User Interface:

- Keyboard emulation for input testing.
- Interfaces for GPU and I/O programming.
## Run Locally

```bash
  git clone https://github.com/mitom2/ZOE.git
  cd ZOE
  mkdir build
  cd build
  cmake ..
  make
```

Copy gpuCharacters directory to location where compiled program is located. Also create there OS directory, where you can place your programs, and ioDisk directory.

Run the emulator:
```bash
  ./ZOE
```
## Usage

- Launch the emulator.
- Enter path to program (OS) file or enter its name.
- Emulated display should at this point be operational.
- Use the interactive menu to emulate keyboard or execute custom commands on GPU and I/O modules.
- View logs in debug.log for troubleshooting.
## License

This project is licensed under the terms of the MIT license. Check the LICENSE file for details.
## Acknowledgements

 - [SUZUKI PLAN - Z80 Emulator](https://github.com/suzukiplan/z80)
 - [HelloACM](https://helloacm.com/modern-getch-implementation-on-windows-cc/)

