#include <cassert>
#include <iostream>
#include <vector>

//wining function
//test winning funciton
//copy paste the alpha beta

const unsigned int BOT_BOARD = 65535;
const unsigned int TOP_BOARD = 4294901760;
const size_t BOARD_SIZE = 16;

const unsigned int ROTATES_UP = 2013231103;
const unsigned int ROTATES_BACK_11 = 134219776;
const unsigned int ROTATES_BACK_3 = 2147516416;

const unsigned int BOT_OUTSIDE_WIN = 584;
const unsigned int BOT_LAST_OUTSIDE_WIN = 3585;
const unsigned int BOT_TOP_HORZ_WIN = 50208;
const unsigned int BOT_BOT_HORZ_WIN = 14352;
const unsigned int BOT_LEFT_VERT_WIN = 37122;
const unsigned int BOT_RIGHT_VERT_WIN = 24708;
const unsigned int BOT_SINISTER_WIN = 20545;
const unsigned int BOT_DEXTER_WIN = 41480;

const unsigned int TOP_OUTSIDE_WIN = BOT_OUTSIDE_WIN << 16;
const unsigned int TOP_LAST_OUTSIDE_WIN = BOT_LAST_OUTSIDE_WIN << 16;
const unsigned int TOP_TOP_HORZ_WIN = BOT_TOP_HORZ_WIN << 16;
const unsigned int TOP_BOT_HORZ_WIN = BOT_BOT_HORZ_WIN << 16;
const unsigned int TOP_LEFT_VERT_WIN = BOT_LEFT_VERT_WIN << 16;
const unsigned int TOP_RIGHT_VERT_WIN = BOT_RIGHT_VERT_WIN << 16;
const unsigned int TOP_SINISTER_WIN = BOT_SINISTER_WIN << 16;
const unsigned int TOP_DEXTER_WIN = BOT_DEXTER_WIN << 16;


//  9,  8,  7, 6
// 10, 15, 14, 5
// 11, 12, 13, 4
//  0,  1,  2, 3

//bot is going to be white, and is first for everything that can be first
//scores: -9 -> 10, 0 is a draw. Positive bot wins, negative top wins
//You get 1 point for every unplaced stone, plus 1
// (BOARD_SIZE - (botCount + topCount)) + 1

void printBoard(unsigned int board)
{
    static const int printOrder[16] = {9,8,7,6,10,15,14,5,11,12,13,4,0,1,2,3};//order to get the right print

    unsigned int bot = board & BOT_BOARD;
    unsigned int top = (board & TOP_BOARD) >> 16;

    for(int i = 0; i < BOARD_SIZE; i++)
    {
        bool currentBot = bot & (1 << printOrder[i]);
        bool currentTop = top & (1 << printOrder[i]);

        assert(!(currentBot && currentTop));

        if(currentBot) std::cout << "\033[47m @ \033[0m";
        else if(currentTop) std::cout << "\033[40m @ \033[0m";
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

bool botChecks(unsigned int board)
{
    if((BOT_LAST_OUTSIDE_WIN & board) == BOT_LAST_OUTSIDE_WIN) return true;
    if((BOT_TOP_HORZ_WIN & board)     == BOT_TOP_HORZ_WIN) return true;
    if((BOT_BOT_HORZ_WIN & board)     == BOT_BOT_HORZ_WIN) return true;
    if((BOT_LEFT_VERT_WIN & board)    == BOT_LEFT_VERT_WIN) return true;
    if((BOT_RIGHT_VERT_WIN & board)   == BOT_RIGHT_VERT_WIN) return true;
    if((BOT_SINISTER_WIN & board)     == BOT_SINISTER_WIN) return true;
    if((BOT_DEXTER_WIN & board)       == BOT_DEXTER_WIN) return true;

    return false;
}

bool topChecks(unsigned int board)
{
    if((TOP_LAST_OUTSIDE_WIN & board) == TOP_LAST_OUTSIDE_WIN) return true;
    if((TOP_TOP_HORZ_WIN & board)     == TOP_TOP_HORZ_WIN) return true;
    if((TOP_BOT_HORZ_WIN & board)     == TOP_BOT_HORZ_WIN) return true;
    if((TOP_LEFT_VERT_WIN & board)    == TOP_LEFT_VERT_WIN) return true;
    if((TOP_RIGHT_VERT_WIN & board)   == TOP_RIGHT_VERT_WIN) return true;
    if((TOP_SINISTER_WIN & board)     == TOP_SINISTER_WIN) return true;
    if((TOP_DEXTER_WIN & board)       == TOP_DEXTER_WIN) return true;

    return false;
}

unsigned int winShift(unsigned int board) //0: no 1:bot 2:top 3:draw
{
   unsigned int outside = board & (board << 1) & (board << 2) & (board << 3); //outside

   unsigned int botWin = BOT_OUTSIDE_WIN & outside;
   unsigned int topWin = TOP_OUTSIDE_WIN & outside;

   if(botWin && topWin) return 3;

   if(!botWin)
   {
       botWin = botChecks(board);
   }

   if(!topWin)
   {
       topWin = topChecks(board);
   }

   if(botWin && topWin) return 3;
   if(topWin) return 2;
   if(botWin) return 1;
   return 0;
}

int countBotPlaces(int inputBoard)
{
    int count = 0;
    int botBoard = inputBoard & BOT_BOARD;
    while(botBoard)
    {
        count += botBoard & 1;
        botBoard >>= 1;
    }
    return count;
}

void generateTopBinarys(std::vector<unsigned int>& boards, unsigned int board, int startIndex, int remainingBits)
{
    if(remainingBits <= 0)
    {
        if(remainingBits < 0) return;
        boards.push_back(board);
        return;
    }

    for(int i = startIndex; i <= 32 - remainingBits; i++)
    {
        if(board & (1 << (i-16))) continue;
        generateTopBinarys(boards, board | (1 << i), i+1, remainingBits-1);
    }
}

void generateBotBinarys(std::vector<unsigned int>& boards, unsigned int board, int startIndex, int remainingBits)
{
    if(!remainingBits)
    {
        int botPlaces = countBotPlaces(board);
        generateTopBinarys(boards, board, 16, botPlaces-1);
        generateTopBinarys(boards, board, 16, botPlaces);
        return;
    }

    for(int i = startIndex; i <= 16 - remainingBits; i++)
    {
        generateBotBinarys(boards, board | (1 << i), i+1, remainingBits-1);
    }
}

int botNegamax(int board, int alpha, int beta)
{
    unsigned int top = (board & TOP_BOARD) >> 16;
    for(int i = 16; i < 32; i++) // For every opponent marble. This is optional
    {
        if(!(board & (1<<i))) continue;
        for
    }
    int score = 0;
    return score;
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

    //Try to generate all positions?
    int botPlaced = 0;
    std::vector<unsigned int> allPositions;

    while(botPlaced <= BOARD_SIZE/2)
    {
        std::cout << "placed: " << botPlaced << std::endl;
        generateBotBinarys(allPositions, 0, 0, botPlaced);
        botPlaced += 1;
    }

    for(int i = 10160000; i < allPositions.size(); i++)
    {
        printBoard(allPositions[i]);
        std::cout << "score is: " << winShift(allPositions[i]) << " for i: " << i << std::endl;
        if(winShift(allPositions[i]) != 0 )
        {
            std::cout << "  ^win" << std::endl;
        }
    }

    //missed win:550952, 550898

//    printBoard(allPositions[550952]);
//    std::cout << winShift(allPositions[550952]);

//    printBoard(allPositions[550898]);
//    std::cout << winShift(allPositions[550898]);

    std::cout << "size: " << allPositions.size() << std::endl;

    return 0;
}
