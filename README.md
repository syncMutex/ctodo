# ctodo
Simple todo app made with C.

## Install

Requirements
- gcc
- make
- ncurses

to install:
```
make todo
```

to run the file:
```
./out
```

## Usage

#### Todo management

|command|action|
|-------|------|
|`n`|create new todo|
|`dd`|delete the todo|
|`x`|toggle mark todo complete|
|`i`|modify todo|
|`o`|view todo|

#### Movement

|command|action|
|-------|------|
|`k` or <kbd>&uarr;</kbd>|move cursor up|
|`j` or <kbd>&darr;</kbd>|move cursor down|
|`m`|toggle move mode|
