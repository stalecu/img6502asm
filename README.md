# img6502asm
Image to 6502 assembly converter

## Motivation
This project was born out of curiosity playing with [Easy
6502](http://skilldrick.github.io/easy6502/) and wanting to render
images. I discovered
[img2asm6502](https://github.com/billiegoose/img2asm6502), but I felt
frustrated with using it, and I wanted something that isn't coupled to
a browser and something I could use in scripts perhaps.

## Requirements
- a C90 compiler
- libpng

## Instructions
```
git clone https://github.com/stalecu/img6502asm
cd img6502asm
make clean
```

`img6502` currently takes 4 arguments:
- `-i` or `--input`, which is the input file (mandatory), it has to be
a 32x32 PNG file;
- `-o` or `--output`, which is the output PNG file, in case you want
to see the converted file. No support for alternate palettes, however
one can very simply change the source code;
- `-a` or `--algorithm`, which chooses the algorithm for finding the color difference. The options are:
  - `cielab` (the default), which uses the Delta-E with CIEDE2000. It's the most accurate option, at least according to my tests;
  - `euclidian`, which is just the Euclidian distance.
  - `weighted`, which takes into account the luminous efficacy curve and how humans perceive green better than other colors.
- `-s` or `--asm`, which is the output file for the generated 6502 assembly. By default, it prints the output to stdout.
- `-h` or `--help`, this is self-explanatory.

If you find any mistakes or want to contribute, write an issue or send a pull request!