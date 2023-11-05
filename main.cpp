#include <cassert>
#include <iostream>

//wining function
//test winning funciton
//copy paste the alpha beta

const unsigned int TOP_BOARD = 4294901760;
const unsigned int BOT_BOARD = 65535;
const size_t BOARD_SIZE = 16;

const unsigned int ROTATES_UP = 2013231103;
const unsigned int ROTATES_BACK_11 = 134219776;
const unsigned int ROTATES_BACK_3 = 2147516416;

//  9,  8,  7, 6
// 10, 15, 14, 5
// 11, 12, 13, 4
//  0,  1,  2, 3

//bot is going to be white, and is first for everything that can be first

void printBoard(int board)
{
    static const int printOrder[16] = {9,8,7,6,10,15,14,5,11,12,13,4,0,1,2,3};//order to get the right print

    int bot = board & BOT_BOARD;
    int top = (board & TOP_BOARD) >> 16;

    for(int i = 0; i < BOARD_SIZE; i++)
    {
        bool currentBot = bot & (1 << printOrder[i]);
        bool currentTop = top & (1 << printOrder[i]);

        assert(!(currentBot && currentTop));

        if(currentBot) std::cout << " @ ";
        else if(currentTop) std::cout << " X ";
        else std::cout << " . ";

        if(i%4 == 3) std::cout << std::endl;
    }

    std::cout << std::endl;
    return;
}

unsigned int rotate(unsigned int board)
{
    return    ((board & ROTATES_UP) << 1)
            | ((board & ROTATES_BACK_11) >> 11)
            | ((board & ROTATES_BACK_3) >> 3);
}

int main()
{
    int numTestBoards = 6;
    unsigned int testBoards[numTestBoards] = {1252664661, 2147487744, 2420247048, 4327977, 1145342088, 2048};

    for(int i = 0; i < numTestBoards; i++)
    {
        std::cout << i << ":" << std::endl;
        printBoard(testBoards[i]);
        testBoards[i] = rotate(testBoards[i]);
        printBoard(testBoards[i]);
    }

    return 0;
}
