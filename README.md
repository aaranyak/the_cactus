## `cactus` - a chess engine that is supposed to defeat humans in chess

<img width="1220" height="655" alt="image" src="https://github.com/user-attachments/assets/43929465-946a-4ded-b45a-839034db88b6" />

> The Cactus is a Bitboard chess engine written in C. It uses a search function with iterative depth-deepening and a simple handcrafted evaluation. It has a user interface written using GTK for linux and a simple windows implementation using `windows.h`. The engine can generally defeat beginner to intermediate level chess players but can easily be defeated by skilled humans.

### Move Generation
 - **The board is represented using twelve 64-bit bitboards - one for each piece and colour**
 - **The moves for jumping pieces (knights, pawns, king) are generated using simple lookup tables**
 - **The moves for sliding pieces (queen, rook, bishop) are generated using magic bitboards, a kind of hash table**
 - **The legality of a move is tested by playing the move, and testing if that threatens the king**.

### Search Optimisations
 - **The move ordering scheme totals scores for**
     -  Move on hash table (first priority)
     -  Capture bonus by captured value minus capturer value
     -  If pawn is promoted then value of promoted piece
     -  Extra bonus for killer moves
     -  Penalty for possible capture on moving
 - **After searching to the maximum depth, the program runs a quiescence search**
     - Use of SEE to attempt an early cuttoff
     - Delta pruning to attempt early cuttoff
 - **Search extensions for check**
 - **Late Move Reductions with two reduction depths**
 - **Null Move Pruning**

### Evaluation Function

### Transposition Table
