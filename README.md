# ms-rawvf

Raw Video Format (RAWVF) is a "human readable minesweeper video format" created in 2009. This allow videos from any Minesweeper game to be translated into a common format for cross-compatibility.

Supported Minesweeper versions are:

- Minesweeper Arbiter versions 0.35 to 0.52.3 (*AVF format)
- Minesweeper Clone versions 0.76 beta to 2007 release 2 (*MVF format)
- Minesweeper X versions 1.00 to 1.16 (*MVR format)
- Vienna MineSweeper versions 2 beta to 3.0C & 3.0H (*RMV format)
- Vienna MineSweeper historical tournament versions (*UMF format)
- FreeSweeper (*FVSF) PENDING

Each Minesweeper game requires a "parser". The parser translates the proprietary video format into a RAWVF text file with all mouse events. This "golden source" video data does not include data interpretations such as board events or statistics.

To add board events and statistics you can use the RAWVF parser. This saves you the trouble of interpreting raw video data. For example, the RAWVF parser will take the list of mouse events and translate these into board events such as flags, questionmarks, numbers, mines and openings.

The original format was invented by Cryslon (Arbiter, FreeSweeper) with help from Curtis (MSX) and Tommy (ViennaSweeper) with Clone bug fixes by crazyks. Unfortunately, the project died in 2014. This GitHub report represents a major update in 2020 by thefinerminer and each C source code file contains detailed comments.

A great example of an online video player that reads RAWVF can be found here:

- https://github.com/hgraceb/web-minesweeper-player







