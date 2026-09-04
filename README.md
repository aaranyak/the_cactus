## `cactus` - a chess engine that is supposed to defeat humans in chess

<img width="1220" height="655" alt="image" src="https://github.com/user-attachments/assets/43929465-946a-4ded-b45a-839034db88b6" />

> The Cactus is a Bitboard chess engine written in C. It uses a search function with iterative depth-deepening and a simple handcrafted evaluation. It has a user interface written using GTK for linux and a simple windows implementation using `windows.h`. The engine can generally defeat beginner to intermediate level chess players but can easily be defeated by skilled humans.

### Move Generation
 - **The board is represented using twelve 64-bit bitboards - one for each piece and colour**
 - **The moves for jumping pieces (knights, pawns, king) are generated using simple lookup tables**
 - **The moves for sliding pieces (queen, rook, bishop) are generated using magic bitboards, a kind of hash table**
 - **The legality of a move is tested by playing the move, and testing if that threatens the king**.

### Search Function

### Evaluation Function
