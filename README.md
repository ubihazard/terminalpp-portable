# Terminal++

*Terminal++*, or `tpp` should be a minimalistic, simple to use virtual terminal that allows basic `VT100` emulation, similar to that of the windows console, as well as much more complex `++` mode which allows things like graphics to be displayed as well. 

> This looks like a good font: http://typeof.net/Iosevka/ 

## Links to study:

https://github.com/Microsoft/console/tree/master/samples/ConPTY/EchoCon

## TODOs

> When dealing with fonts, sometimes the bold font is thicker than normal font, which sounds rather bad. Also, nerd fornts do not behave like proper monospace fonts, which is bad too. 

> Refactor terminal window so that the renderer specific details are decoupled from the virtual terminal maintenance.

## Architecture Overview

To maximize cross-compatibility between various operating systems and rendering engines, `tpp` splits into different parts all briefly explained here:

### Virtual Terminal

The virtual terminal (`vterm`) describes the internal state of the terminal and holds all the data required to display its contents. 

### Terminal Window

Terminal window is a simple window which is attached to one virtual terminal. The terminal window displays the virtual terminal contents.

### Terminal Application

The terminal application is a collection of virtual terminals and terminal windows and manages the mapping between these. It also contains an extra virtual terminal displaying its own settings & control application. 






