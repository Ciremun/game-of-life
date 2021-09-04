# game-of-life

## Controls

|       Key          |  Description  |
|--------------------|---------------|
| <kbd>1</kbd>       | Game Of Life  |
| <kbd>2</kbd>       | Brian's Brain |
| <kbd>Space</kbd>   | Pause         |
| <kbd>R</kbd>       | Reset         |
| <kbd>=/+</kbd>     | Grid Grow     |
| <kbd>-</kbd>       | Grid Shrink   |

### Android

Show keyboard by tapping top right corner  

## Build

### Android

[CNLohr's guide](https://github.com/cnlohr/rawdrawandroid#steps-for-gui-less-install-windows-wsl)

### Windows, Linux

    cc nobuild.c -o nobuild && ./nobuild run

### WASM

    make -f Makefile.wasm
