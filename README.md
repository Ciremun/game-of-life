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

### Linux

    make linux

### Windows

[Build Tools for Visual Studio 2019](https://visualstudio.microsoft.com/downloads/)  

In Visual Studio Installer:  
Windows 10 SDK  
MSVC Build Tools & Libraries  

    cmd.exe
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\Common7\Tools\VsDevCmd.bat"
    cd game-of-life
    build_win.bat
