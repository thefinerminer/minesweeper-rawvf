# Minesweeper RAW Video Format

Minesweeper RAW Video Format is a "human readable minesweeper video format" created in 2009. Minesweeper videos can be translated into this format for cross-compatibility. This project hosts code for converting different videos formats into RAWVF.

Supported Minesweeper games and video formats are:

- <b>Minesweeper Arbiter</b> 0.35 to 0.52.3 (*AVF format)
- <b>Minesweeper Clone</b> 0.76 beta to 2007 release 2 (*MVF format)
- <b>Minesweeper X</b> 1.00 to 1.16 (*MVR format)
- <b>Vienna MineSweeper</b> All versions up to 3.0C & 3.0H (*UMF & *RMV format)
- <b>FreeSweeper</b> All versions up to R10 (*FVSF)

Videos contain mouse events and timestamps. Each programmer can write a "parser" to translate their proprietary video format into RAW format. The parser reads the video and puts a row into a text file for each mouse event. Most videos formats also store basic information about the game such as program, version, width, height, mode, status, time and number of mines. 

This project hosts parser code for all listed versions except Minesweeper X as its programmer wishes to keep the MSX video encryption method private. The project also hosts compiled binaries. To use a parser, open it in a command prompt and type "parser_xyz video.xzy>video.txt". Using Windows type "parser_xyz.exe video.xzy>video.txt".

In addition to each parser, you can use the master parser ("parser_raw") which reads parsed files and calculates board events and statistics. For example, the master parser adds events such as flags, questionmarks, numbers, mines, openings and tells you which squares were opened. This saves you the trouble of calculating everything and makes it easy to build a video player. In the command prompt type "parser_raw video.txt>raw.txt".

A great example of an online video player that reads RAWVF can be found here:

- https://github.com/hgraceb/web-minesweeper-player

This video player is used on the following Minesweeper rankings:

- <a href="https://minesweepergame.com">minesweepergame.com</a> - World Rankings
- <a href="http://www.saolei.wang">saolei.wang</a> - China Rankings

The original format was invented by Cryslon (Arbiter, FreeSweeper) with help from Curtis (MSX) and Tommy (ViennaSweeper). Clone bug fixes were added by crazyks before the project died in 2014 and the final format was called "RAW 5". This GitHub repo represents a major update in 2020 by thefinerminer and each C source code file contains detailed comments released as RAW 6.

Details of the format can be found here:

- <a href="https://minesweepergame.com/forum/viewtopic.php?p=619&sid=173c88eefbcf15bc16285932ec58dcb0#p619">Open human-readable minesweeper video format</a>




