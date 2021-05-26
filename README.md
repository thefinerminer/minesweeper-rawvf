# ms-rawvf

Raw Video Format (RAWVF) is a "human readable minesweeper video format" created in 2009. Videos from any Minesweeper game can be translated into this format for cross-compatibility.

Supported Minesweeper versions are:

- <b>Minesweeper Arbiter</b> 0.35 to 0.52.3 (*AVF format)
- <b>Minesweeper Clone</b> 0.76 beta to 2007 release 2 (*MVF format)
- <b>Minesweeper X</b> 1.00 to 1.16 (*MVR format)
- <b>Vienna MineSweeper</b> All versions up to 3.0C & 3.0H (*UMF & *RMV format)
- <b>FreeSweeper</b> All versions up to R10 (*FVSF)

The games listed above store all mouse events in their video files. Each programmer can write a "parser" to translate their video format into RAW format. The parser will read the video and put a row into a text file for each mouse event. Most videos also store basic information about the program version and about the game such as width, height, mode, status, time and number of mines. 

This project hosts parser code for all listed versions except Minesweeper X since the programmer wishes to keep the MSX video encryption method private. The project also hosts compiled binaries. To use a parser, open it in a command prompt and type "parser-name video-name.xzy>video-name.txt".

In addition to each parser, you can use a master parser ("parser_raw") which reads parsed files and add board events and statistics. For example, this parser will add events such as flags, questionmarks, numbers, mines, openings and will tell you which squares are being opened. This saves you the trouble of calculating everything and makes it easy to build video players.

A great example of an online video player that reads RAWVF can be found here:

- https://github.com/hgraceb/web-minesweeper-player

This video player is used on the following Minesweeper rankings:

- https://minesweepergame.com
- http://www.saolei.wang

The original format was invented by Cryslon (Arbiter, FreeSweeper) with help from Curtis (MSX) and Tommy (ViennaSweeper). Clone bug fixes were added by crazyks before the project died in 2014 and the final format was called "RAW 5". This GitHub repo represents a major update in 2020 by thefinerminer and each C source code file contains detailed comments released as RAW 6.








