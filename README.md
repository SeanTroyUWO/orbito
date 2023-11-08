# orbito
This progress designs a brute for solution to [Orbito](https://flexiqgames.com/en/product/orbito/), a two player game from FlexIQ. The first player can win orbito with perfect play. 

It utilizes the alpha-beta pruning negamax algorithm with a state table and an efficient representation (32 bits). It should weakly solve the game on most machines in less than a minute while asserts are not compiled in. The conclusion can be verified with random walks, or by an interactive player mode where the perfect AI can be tested.

There are many optimizations to be made here but this runs quickly enough. 

# Total States

The best upper board for the number of states is 10165779. Certainly some of these are invalid, or can never be reached. This can we found with the equation below, where $S$ is the size of the board. 
```math
1+\sum_{i=1}^{S/2}(\frac{S!}{i!\times(S-i)!} \frac{(S-i)!}{i!\times(S-2i)!} + \frac{S!}{i!\times(S-i)!} \frac{(S-i)!}{(i-1)!\times(S-2i-1)!})
```

# Representation

Every game state can be efficient represented by one 32 bit integer. The bottom 16 bits represent the locations of the first players marbles, while the top 16 bits represent the locations of the second players marbles. The map of bit locations can be found below. For the second player, or the top player, all of these values are 16 higher:

| 9  | 8  | 7  | 6 |
|----|----|----|---|
| 10 | 15 | 14 | 5 |
| 11 | 12 | 13 | 4 |
| 0  | 1  | 2  | 3 |

# Verification

After running 100 Million random walks, no second player victories were found. 


