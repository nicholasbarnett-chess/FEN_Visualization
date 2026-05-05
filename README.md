# Forsyth-Edwards Notation (FEN)
Any position on a chess board can be described using FEN. An FEN sequence is composed exclusively of ASCII characters, making it easy for computers to interact with. An FEN sequence is really just a string containing six fields. These fields are where the pieces are on the board, which color's turn is it, who can castle where, what squares can en passant take place, number of half moves since the last pawn advance or piece capture, and number of completed turns in the game. FEN is the most common format in the chess community to relay a position on the board. 
This repository will take in an FEN sequence and generate a .png file of a board rendering the position. This is currently a visualization tool to represent a chess position from an FEN into an image. Much of this code was written by interacting with ChatGBT, and some image visualization code was used from here: https://github.com/nothings/stb/blob/master/.

# FEN to .png
To execute the BoardVisualizer script and make a png one would follow the format below.

Making an image of the starting position of a chess game:
root 'BoardVisualizer.C("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "StartingPosition.png")'

Making an image of the starting position of an Evans gambit:
root 'BoardVisualizer.C("r1bqk1nr/pppp1ppp/2n5/2b1p3/1PB1P3/5N2/P1PP1PPP/RNBQK2R b KQkq - 0 4", "EvansGambit.png")'
