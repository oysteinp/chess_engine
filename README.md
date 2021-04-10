<h1>Chess engine</h1>

This is simple chess engine project developed in C. 

<h2>Board</h2>
The state of the board is represented in a "0x88 chess board representation". 

    int board[128];
The benefit from this approach for the simplification of some important calculations due to bitwise operators.

<h2>Evaluation</h2>
The engine uses a very simple position evaluation. Positive evaluation means that white has the better positioin, while negative means that black has the upper hand.
1. Material. The difference in material between white and black. 
2. Piece placements. Each piece has its own table with a score for each position on the board. For instance does the knights prefer central positions while a corner placement is the absolute worst.

<h2>Search for moves</h2>
The engine uses alpha beta pruning to search the move tree for the best moves. The moves are ordered such that all the captures are evaluated first at any given position. This is an attempt to achieve more pruning in the search. At all leaf nodes a quiescence search is performed to ensure that the evaluation is based on a quiet positiion where no captures are available. 

<h2>Compile</h2>
To compile the executable from the source code and run the engine simply do:
    
    make && ./chess newGame
    
<h2>Game play</h2>
Currently the engine only works while playing the white pieces. The engine supports levels from 1-4.
When the engine asks for the user move it will first ask for the square you move from and then for the target square. Only the square should be given, meaning that no captures flag should be provided.

Example:
From square: d7
To square: d5

When promoting a piece the promoted piece is the third character. The possible alternatives for black is:
* q - black queen
* r - black rook
* n - black knight
* b - black bishop

Example:
From square: d2
To square: d1q

Good luck!
    


