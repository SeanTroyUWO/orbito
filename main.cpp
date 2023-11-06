#include <cassert>
#include <iostream>
#include <unordered_map>
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

std::vector<unsigned int> allPositions;
std::unordered_map<unsigned int, int> allPositionsAlpha; //values in table are saved before rotating
std::unordered_map<unsigned int, int> allPositionsBeta; //values in table are saved before rotating

//  9,  8,  7, 6
// 10, 15, 14, 5
// 11, 12, 13, 4
//  0,  1,  2, 3



//bot is going to be white, and is first for everything that can be first
//scores: -9 -> 10, 0 is a draw. Positive bot wins, negative top wins
//You get 1 point for every unplaced stone, plus 1
// (BOARD_SIZE - (botCount + topCount)) + 1

//Alpha: min for the maximizer, bot
//Beta: max for the minimizer, top

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
    //This does not rotate
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

int countBotPlaces(unsigned int inputBoard)
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

int countTopPlaces(unsigned int inputBoard)
{
    int count = 0;
    int topBoard = (inputBoard & TOP_BOARD) >> 16;
    while(topBoard)
    {
        count += topBoard & 1;
        topBoard >>= 1;
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

void generateScoreMap()
{
    for(int i = 0; i < allPositions.size(); i++)
    {
        int marblesPlaced = countBotPlaces(allPositions[i]) + countTopPlaces(allPositions[i]);

        if(marblesPlaced == BOARD_SIZE) //rotate 5 times rule
        {
            unsigned int rotatedBoard = rotate(allPositions[i]);
            int winValue = 0;
            for(int rotations = 1; rotations <= 5; rotations++)
            {
                winValue = winShift(rotatedBoard);
                if(winValue) break;
                rotatedBoard = rotate(rotatedBoard);
            }

            if(winValue == 1)
            {
                int boundValue = (BOARD_SIZE - marblesPlaced) + 1;
                allPositionsAlpha[allPositions[i]] = boundValue;
                allPositionsBeta[allPositions[i]] = boundValue;
            }
            else if(winValue == 2)
            {
                int boundValue = -((BOARD_SIZE - marblesPlaced) + 1);
                allPositionsAlpha[allPositions[i]] = boundValue;
                allPositionsBeta[allPositions[i]] = boundValue;
            }
            else if(winValue == 3)
            {
                allPositionsAlpha[allPositions[i]] = 0;
                allPositionsBeta[allPositions[i]] = 0;
            }
            else
            {
                allPositionsAlpha[allPositions[i]] = -400; //unknown
                allPositionsBeta[allPositions[i]] = 400; //unknown
            }

            continue;
        }

        if(int winValue = winShift(rotate(allPositions[i])))
        {
            if(winValue == 1)
            {
                int boundValue = (BOARD_SIZE - marblesPlaced) + 1;
                allPositionsAlpha[allPositions[i]] = boundValue;
                allPositionsBeta[allPositions[i]] = boundValue;
            }
            else if(winValue == 2)
            {
                int boundValue = -((BOARD_SIZE - marblesPlaced) + 1);
                allPositionsAlpha[allPositions[i]] = boundValue;
                allPositionsBeta[allPositions[i]] = boundValue;
            }
            else if(winValue == 3)
            {
                allPositionsAlpha[allPositions[i]] = 0;
                allPositionsBeta[allPositions[i]] = 0;
            }
        } else
        {
            allPositionsAlpha[allPositions[i]] = -400;
            allPositionsBeta[allPositions[i]] = 400;
        }
    }
}

void generateBotMoves(std::vector<unsigned int>& resultBoardsList, unsigned int board)
{
    //Doesn't rotate
    assert((countBotPlaces(board) - countTopPlaces(board)) == 0);

    static const std::vector<std::vector<int>> nearby{{11,1},{0,12,2},{1, 3,3},{2,4},{13,5,3},{14,6,4},{7,5},{8,6,14},{9,7,15},{8,10},
                                                      {9,15,11},{10,12,0},{11,15,13,1},{12,14,4,2},{15,7,5,13},{10,8,14,12}};

    for(int i = 16; i < 32; i++) // For every opponent marble. This is optional
    {
        if(!(board & (1<<i))) continue;
        for(int localMove : nearby[i-16])
        {
            if(board & ((1<<localMove) | (1<<(localMove+16)))) continue; // if spot to move opponents is filled
            for(int j = 0; j < 16; j++ ) // For every move option we have
            {
                //if: taken by me, taken by top, unless moved by us
                //spot we are opp marble moving to
                if((board & ((1<<j) | (1<<(j+16)) & ~(1 << i))) ||
                   (j == localMove)) continue;

                //i: newly free spot from opponent marble we moved
                //localMove: newly taken spot of opp marble
                //j: our new marble spot
                resultBoardsList.push_back((board | (1<<j) | (1<<(localMove+16))) & ~(1 << i));
            }
        }
    }

    for(int j = 0; j < 16; j++ ) // For every move option we have
    {
        //if: taken by me, taken by top
        if(board & ((1<<j) | (1<<(j+16)))) continue;

        //j: our new marble spot
        resultBoardsList.push_back(board | (1<<j));
    }
}

void generateTopMoves(std::vector<unsigned int>& resultBoardsList, unsigned int board)
{
    //Doesn't rotate
    assert((countBotPlaces(board) - countTopPlaces(board)) == 1);

    static const std::vector<std::vector<int>> nearby{{11,1},{0,12,2},{1, 3,3},{2,4},{13,5,3},{14,6,4},{7,5},{8,6,14},{9,7,15},{8,10},
                                                      {9,15,11},{10,12,0},{11,15,13,1},{12,14,4,2},{15,7,5,13},{10,8,14,12}};

    for(int i = 0; i < 16; i++) // For every opponent marble. This is optional
    {
        if(!(board & (1<<i))) continue;
        for(int localMove : nearby[i])
        {
            if(board & ((1<<localMove) | (1<<(localMove+16)))) continue; // if spot to move opponents is filled
            for(int j = 16; j < 32; j++) // For every move option we have
            {
                //if: taken by me, taken by bot, unless moved by us
                //spot we are opp marble moving to
                if((board & ((1<<j) | (1<<(j-16)) & ~(1 << i))) ||
                   (j-16 == localMove)) continue;

                //i: newly free spot from opponent marble we moved
                //localMove: newly taken spot of opp marble
                //j: our new marble spot
                resultBoardsList.push_back((board | (1<<j) | (1<<(localMove))) & ~(1 << i));
            }
        }
    }

    for(int j = 16; j < 32; j++) // For every move option we have
    {
        //if: taken by me, taken by top
        if(board & ((1<<j) | (1<<(j-16)))) continue;

        //j: our new marble spot
        resultBoardsList.push_back(board | (1<<j));
    }
}

double progressCheck()
{
    double states = allPositionsAlpha.size() + allPositionsBeta.size();
    int filledStates = 0;

    for(auto alphaPair: allPositionsAlpha) if(alphaPair.second != -400) filledStates+=1;
    for(auto betaPair: allPositionsBeta) if(betaPair.second != 400) filledStates+=1;
    return filledStates/states;
}

int botNegamax(unsigned int board, int alpha, int beta, int& nodes);
int topNegamax(unsigned int board, int alpha, int beta, int& nodes);

int botNegamax(unsigned int board, int alpha, int beta, int& nodes)
{
    assert(alpha < beta);
    assert(allPositionsBeta.count(board) && allPositionsAlpha.count(board));
    assert((countBotPlaces(board) - countTopPlaces(board)) == 0);

    int alphaCheck = allPositionsAlpha[board];
    int betaCheck = allPositionsBeta[board];

    if(alphaCheck > alpha) alpha = alphaCheck;
    if(betaCheck < beta) beta = betaCheck;

    if(alpha >= beta) return alpha;

    nodes++;
    if(!(nodes%100000))
    {
        std::cout << "nodes: " << nodes /*<< " progress: " << progressCheck()*/ << std::endl;
    }

    unsigned int rotatedBoard = rotate(board);
    assert(!winShift(rotatedBoard));

    std::vector<unsigned int> resultBoardsList;
    generateBotMoves(resultBoardsList, rotatedBoard);

    for(unsigned int moveBoard: resultBoardsList)
    {
        int currentScore = topNegamax(moveBoard, alpha, beta, nodes);
        if(currentScore > alpha)
        {
            alpha = currentScore;
            allPositionsAlpha[board] = alpha;
            if(alpha >= beta)
            {
                return alpha;
            }
        }
    }
    resultBoardsList.clear();

    allPositionsBeta[board] = alpha;
    return alpha;
}

int topNegamax(unsigned int board, int alpha, int beta, int& nodes)
{
    assert(alpha < beta);
    assert(allPositionsAlpha.count(board) && allPositionsBeta.count(board));
    assert((countBotPlaces(board) - countTopPlaces(board)) == 1);

    int alphaCheck = allPositionsAlpha[board];
    int betaCheck = allPositionsBeta[board];

    if(alphaCheck > alpha) alpha = alphaCheck;
    if(betaCheck < beta) beta = betaCheck;

    if(alpha >= beta) return beta;

    nodes++;
    if(!(nodes%100000))
    {
        std::cout << "nodes: " << nodes /*<< " progress: " << progressCheck()*/ << std::endl;
    }

    unsigned int rotatedBoard = rotate(board);
    assert(!winShift(rotatedBoard));

    std::vector<unsigned int> resultBoardsList;
    generateTopMoves(resultBoardsList, rotatedBoard);

    for(unsigned int moveBoards: resultBoardsList)
    {
        int currentScore = botNegamax(moveBoards, alpha, beta, nodes);
        if(currentScore < beta)
        {
            beta = currentScore;
            allPositionsBeta[board] = beta;
            if(alpha >= beta)
            {
                return beta;
            }
        }
    }
    resultBoardsList.clear();

    allPositionsAlpha[board] = beta;
    return beta;
}

int main()
{
    int numTestBoards = 6;
    unsigned int testBoards[numTestBoards] = {1252664661, 2147487744, 2420247048, 4327977, 1145342088, 2048};

//    for(int i = 0; i < numTestBoards; i++)
//    {
//        std::cout << i << ":" << std::endl;
//        printBoard(testBoards[i]);
//        testBoards[i] = rotate(testBoards[i]);
//        printBoard(testBoards[i]);
//    }

    //Try to generate all positions?
    int botPlaced = 0;
    while(botPlaced <= BOARD_SIZE/2)
    {
        std::cout << "placed: " << botPlaced << std::endl;
        generateBotBinarys(allPositions, 0, 0, botPlaced);
        botPlaced += 1;
    }
    std::cout << "size: " << allPositions.size() << std::endl;


//    for(int i = 10160000; i < allPositions.size(); i++)
//    {
//        printBoard(allPositions[i]);
//        std::cout << "score is: " << winShift(allPositions[i]) << " for i: " << i << std::endl;
//        if(winShift(allPositions[i]) != 0 )
//        {
//            std::cout << "  ^win" << std::endl;
//        }
//    }

    generateScoreMap();

//    std::vector<unsigned int> testMoves;
//    for(int i = 2160000; i < 3160000; i++)
//    {
//        std::cout << "Starting board: " << std::endl;
//        printBoard(allPositions[i]);
////        generateBotMoves(testMoves, allPositions[i]);
//        generateTopMoves(testMoves, allPositions[i]);

//        for(unsigned int move: testMoves)
//        {
//            printBoard(move);
//        }
//        testMoves.clear();
//    }

    int nodes = 0;
    int value = botNegamax(0, -BOARD_SIZE, BOARD_SIZE, nodes);

    std::cout << "score: " << value << std::endl;

    int stopper =0;
    return 0;
}
