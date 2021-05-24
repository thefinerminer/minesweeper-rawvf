# ms-rawvf

Raw Video Format (RAWVF) is a "human readable minesweeper video format" created in 2009. Videos from any Minesweeper game can be translated into this common format for cross-compatibility.

Supported Minesweeper versions are:

- <b>Minesweeper Arbiter</b> 0.35 to 0.52.3 (*AVF format)
- <b>Minesweeper Clone</b> 0.76 beta to 2007 release 2 (*MVF format)
- <b>Minesweeper X</b> 1.00 to 1.16 (*MVR format)
- <b>Vienna MineSweeper</b> 2 beta to 3.0C & 3.0H (*RMV format)
- <b>Vienna MineSweeper</b> historical versions (*UMF format)
- <b>FreeSweeper</b> (*FVSF) PENDING

Each Minesweeper game requires a "parser". The parser translates the proprietary video format into a RAWVF text file with all mouse events. This "golden source" video data does not include data interpretations such as board events or statistics.

To add board events and statistics use the RAWVF parser. This saves you the trouble of interpreting raw video data. For example, the RAWVF parser will take the list of mouse events and translate these into board events such as flags, questionmarks, numbers, mines and openings.

The original format was invented by Cryslon (Arbiter, FreeSweeper) with help from Curtis (MSX) and Tommy (ViennaSweeper) with Clone bug fixes by crazyks. Unfortunately, the project died in 2014. This GitHub report represents a major update in 2020 by thefinerminer and each C source code file contains detailed comments.

A great example of an online video player that reads RAWVF can be found here:

- https://github.com/hgraceb/web-minesweeper-player

This video player is used on the following Minesweeper rankings:

- https://minesweepergame.com
- http://www.saolei.wang







