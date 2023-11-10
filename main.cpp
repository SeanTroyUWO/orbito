#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

const unsigned int BOT_BOARD = 65535;
const unsigned int TOP_BOARD = 4294901760;
const unsigned int BOARD_SIZE = 16;

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

const unsigned short TABLE_ENTRY_LOWER_BOUND = 255;
const unsigned short TABLE_ENTRY_UPPER_BOUND = TABLE_ENTRY_LOWER_BOUND << 8;
const int TABLE_ENTRY_ADJUSTMENT = BOARD_SIZE;

const unsigned int ROTATE_1_BACK_3    = 267915256;
const unsigned int ROTATE_1_FORWARD_9 = 458759;
const unsigned int ROTATE_1_BACK_1    = 3758153728;
const unsigned int ROTATE_1_FORWARD_3 = 268439552;

const unsigned int ROTATE_2_BACK_6    = 264245184;
const unsigned int ROTATE_2_FORWARD_6 = 4128831;
const unsigned int ROTATE_2_FORWARD_2 = 805318656;
const unsigned int ROTATE_2_BACK_2    = 3221274624;

const unsigned int ROTATE_3_FORWARD_3 = 33489407;
const unsigned int ROTATE_3_BACK_9    = 234884608;
const unsigned int ROTATE_3_FORWARD_1 = 1879076864;
const unsigned int ROTATE_3_BACK_3    = 2147516416;

class TableEntry
{
public:
    unsigned short value;
    TableEntry() {}

    TableEntry(int lowerInput, int upperInput) {
        value = ((upperInput + BOARD_SIZE) << 8) | (lowerInput + BOARD_SIZE);
        assert(this->lowerBound() == lowerInput && this->upperBound() == upperInput);
    }
    int lowerBound() {return (value & TABLE_ENTRY_LOWER_BOUND) - BOARD_SIZE;}
    int upperBound() {return ((value & TABLE_ENTRY_UPPER_BOUND) >> 8) - BOARD_SIZE;}
    void saveLower(int input) {
        value = (value & ~TABLE_ENTRY_LOWER_BOUND) | (input + BOARD_SIZE);
    }
    void saveUpper(int input) {
        value = (value & ~TABLE_ENTRY_UPPER_BOUND) | ((input + BOARD_SIZE) << 8);
    }
};

std::vector<unsigned int> allPositions;
std::unordered_map<unsigned int, TableEntry> allPositionsBounds; //values in table are saved before rotating
//The top byte of the table value is the top bound, the bottom byte is a lower bound
//All values are shifted up by BOARD_SIZE so no negative values

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
    static const int printOrder[16] = {9,8,7,6,10,15,14,5,11,12,13,4,0,1,2,3};

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

unsigned int endGameWinShift(unsigned int board) //End of game 5 rotations rule
{
    if(countBotPlaces(board) + countTopPlaces(board) == BOARD_SIZE)
    {
        board;
        int winValue = 0;
        for(int i = 0; i < 6; i++) // Rules say 5 "more times", so standard rotate not included
        { //should be 6 checks in total than
            winValue = winShift(board);
            if(winValue) return winValue;
            board = rotate(board);
        }
        return 3;
    }
    return winShift(board);
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
                allPositionsBounds[allPositions[i]] = TableEntry(boundValue, boundValue);
            }
            else if(winValue == 2)
            {
                int boundValue = -((BOARD_SIZE - marblesPlaced) + 1);
                allPositionsBounds[allPositions[i]] = TableEntry(boundValue, boundValue);
            }
            else
            {
                allPositionsBounds[allPositions[i]] = TableEntry(0, 0); //End game draw
            }

            continue;
        }

        if(int winValue = winShift(rotate(allPositions[i])))
        {
            if(winValue == 1)
            {
                int boundValue = (BOARD_SIZE - marblesPlaced) + 1;
                allPositionsBounds[allPositions[i]] = TableEntry(boundValue, boundValue);
            }
            else if(winValue == 2)
            {
                int boundValue = -((BOARD_SIZE - marblesPlaced) + 1);
                allPositionsBounds[allPositions[i]] = TableEntry(boundValue, boundValue);
            }
            else if(winValue == 3)
            {
                allPositionsBounds[allPositions[i]] = TableEntry(0, 0);
            }
        } else
        {
            allPositionsBounds[allPositions[i]] = TableEntry(-BOARD_SIZE, BOARD_SIZE); //Unknown
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

int botNegamax(unsigned int board, int alpha, int beta, int& nodes);
int topNegamax(unsigned int board, int alpha, int beta, int& nodes);

int depthVar = 0;
int botNegamax(unsigned int board, int alpha, int beta, int& nodes)
{
    int thisDepthVar = depthVar++;

    assert(alpha < beta);
    assert(allPositionsBounds.count(board));
    assert((countBotPlaces(board) - countTopPlaces(board)) == 0);

    TableEntry tableValue = allPositionsBounds[board];

    alpha = std::max(tableValue.lowerBound(), alpha);
    beta = std::min(tableValue.upperBound(), beta);

    if(alpha >= beta)
    {
        depthVar--;
        return alpha;
    }

    nodes++;
    if(!(nodes%1000000))
    {
        std::cout << "nodes: " << nodes << std::endl;
    }

    unsigned int rotatedBoard = rotate(board);
    assert(!endGameWinShift(rotatedBoard));

    std::vector<unsigned int> resultBoardsList;
    generateBotMoves(resultBoardsList, rotatedBoard);

    for(unsigned int moveBoard: resultBoardsList)
    {
        int currentScore = topNegamax(moveBoard, alpha, beta, nodes);
        if(currentScore > alpha)
        {
            alpha = currentScore;
            allPositionsBounds[board].saveLower(alpha);
            if(alpha >= beta)
            {
                depthVar--;
                return alpha;
            }
        }
    }
    resultBoardsList.clear();

    allPositionsBounds[board].saveUpper(alpha);
    depthVar--;
    return alpha;
}

int topNegamax(unsigned int board, int alpha, int beta, int& nodes)
{
    assert(alpha < beta);
    assert(allPositionsBounds.count(board));
    assert((countBotPlaces(board) - countTopPlaces(board)) == 1);

    TableEntry tableValue = allPositionsBounds[board];

    alpha = std::max(tableValue.lowerBound(), alpha);
    beta = std::min(tableValue.upperBound(), beta);

    if(alpha >= beta)
    {
        return beta;
    }

    nodes++;
    if(!(nodes%1000000))
    {
        std::cout << "nodes: " << nodes << std::endl;
    }

    unsigned int rotatedBoard = rotate(board);
    assert(!endGameWinShift(rotatedBoard));

    std::vector<unsigned int> resultBoardsList;
    generateTopMoves(resultBoardsList, rotatedBoard);

    for(unsigned int moveBoard: resultBoardsList)
    {
        int currentScore = botNegamax(moveBoard, alpha, beta, nodes);
        if(currentScore < beta)
        {
            beta = currentScore;
            allPositionsBounds[board].saveUpper(beta);
            if(alpha >= beta)
            {
                return beta;
            }
        }
    }
    resultBoardsList.clear();

    allPositionsBounds[board].saveLower(beta);
    return beta;
}

void interactiveBotPlay(unsigned int board);
void playerTopPlay(unsigned int board);

void interactiveBotPlay(unsigned int board)
{
    std::cout << "\nPosition:" << std::endl;
    printBoard(board);

    if(endGameWinShift(board) == 1)
    {
        std::cout << "First Player Wins. Press enter to Reverse" << std::endl;
        std::cin.get();
        return;
    }
    assert(!endGameWinShift(board));

    std::vector<unsigned int> resultBoardsList;
    generateBotMoves(resultBoardsList, board);
    unsigned int newBoard = 0;

    for(unsigned int moveBoard: resultBoardsList)
    {
        int nodes;
        int outputScore = topNegamax(moveBoard, -1, 1, nodes);
        if(outputScore >= 1)
        {
            newBoard = moveBoard;
            break;
        }
    }
    assert(newBoard != 0);

    std::cout << "\nPlays:" << std::endl;
    printBoard(newBoard);

    std::cout << "\nRotates To:" << std::endl;
    newBoard = rotate(newBoard);
    printBoard(newBoard);

    if(endGameWinShift(newBoard) == 1)
    {
        std::cout << "First Player Wins. Press enter to Reverse" << std::endl;
        std::cin.get();
        return;
    }

    assert(!endGameWinShift(newBoard));

    playerTopPlay(newBoard);
}

bool isInt(const std::string& input)
{
    return !input.empty() && std::find_if(input.begin(),
        input.end(), [](unsigned char c) { return !std::isdigit(c); }) == input.end();
}

void playerTopPlay(unsigned int board)
{
    std::string inputI;
    std::string inputJ;
    std::string inputK;

    std::vector<unsigned int> resultBoardsList;
    generateTopMoves(resultBoardsList, board);

    while(true)
    {
        std::cout << "\nWhat would you like to play? (H for Help)" << std::endl;

        std::cin >> inputI >> inputJ >> inputK;
        std::cin.ignore(INT_MAX,'\n');

        if(inputI.length() == 1 && std::tolower(inputI[0]) == 'h')
        {
            std::cout << "Indices for moves:" << std::endl;;
            std::cout << " 9,  8,  7, 6\n";
            std::cout << "10, 15, 14, 5\n";
            std::cout << "11, 12, 13, 4\n";
            std::cout << " 0,  1,  2, 3\n\n";

            std::cout << "Help (This Message): h\n";
            std::cout << "Go back one move: k\n";
            std::cout << "Play a move without moving an opponents marble: i\nWhere i is an integer value of the chosen location\n\n";
            std::cout << "Play a move with moving an opponents marble: i j k\n"
                         "Where i is an integer value of location you wish to place your marble\n"
                         "j is an integer value of the location of an opponents marble you wish to move\n"
                         "k is an integer value of the location you wish to move the opponents marble" << std::endl;
        } else if(inputI.length() == 1 && std::tolower(inputI[0]) == 'k')
        {
            std::cout << "Returning..." << std::endl;
            return;
        } else if(inputI.length() <= 2  && isInt(inputI) && inputJ == "" && inputK == "")
        {
            int i = std::stoi(inputI)+16;

            unsigned int newPosition = board | (1<<i);
            auto moveIter = std::find(resultBoardsList.begin(), resultBoardsList.end(), newPosition);
            if(moveIter == resultBoardsList.end())
            {
                std::cout << "Invalid move." << std::endl;
                continue;
            } else
            {
                std::cout << "After your move:\n";
                printBoard(newPosition);
                interactiveBotPlay(rotate(newPosition));
                std::cout << "You've returned to here:\n";
                printBoard(board);
            }
        } else if(inputI != "" && inputJ != "" && inputK != "" && isInt(inputI) && isInt(inputJ) && isInt(inputK))
        {
            int i = std::stoi(inputI)+16;
            int j = std::stoi(inputJ);
            int k = std::stoi(inputK);

            std::cout << i << j << k << "\n";

            unsigned int newPosition = (board | (1<<i) | (1<<k)) & ~(1 << j);

            auto moveIter = std::find(resultBoardsList.begin(), resultBoardsList.end(), newPosition);
            if(moveIter == resultBoardsList.end())
            {
                std::cout << "Invalid move." << std::endl;
            } else
            {
                std::cout << "After your move:\n";
                printBoard(newPosition);
                interactiveBotPlay(rotate(newPosition));

                std::cout << "You've returned to here:\n ";
                printBoard(board);
            }
        } else
        {
            std::cout << "Unrecognized move." << std::endl;
        }
    }
}

void randomWalk()
{
    std::vector<unsigned int> resultBoardsList;
    std::srand(1);

    std::cout << "Starting Random Walks: " << std::endl;

    int i = 0;
    while(true)
    {
        std::cout << "i: " << i;
        unsigned int board = 0;
        int winValue = 0;

        while(true)
        {
            //BOT PLAYS
            resultBoardsList.clear();
            generateBotMoves(resultBoardsList, board);

            unsigned int newBoard = 0;
            for(unsigned int moveBoard: resultBoardsList)
            {
                int nodes;
                int outputScore = topNegamax(moveBoard, -1, 1, nodes);
                if(outputScore >= 1)
                {
                  newBoard = moveBoard;
                  break;
                }
            }
            assert(newBoard);

            std::cout << " " << newBoard;

            board = rotate(newBoard);
            winValue = endGameWinShift(board);

            if(winValue == 1)
            {
              std::cout << " Win" << std::endl;
              break;
            } else if(winValue)
            {
              std::cout << "Error?" << std::endl;
              assert(0);
              break;
            }

            //TOP PLAYS:
            resultBoardsList.clear();
            generateTopMoves(resultBoardsList, board);

            board = resultBoardsList[std::rand() % resultBoardsList.size()];
            std::cout << " " << board;

            board = rotate(board);
            winValue = endGameWinShift(board);
            if(winValue == 1)
            {
              std::cout << " Win" << std::endl;
              break;
            } else if(winValue)
            {
              std::cout << "Error?" << std::endl;
              assert(0);
              break;
            }
        }

        i++;
    }
}

int main()
{
    //Board asserts
    assert(ROTATE_1_BACK_3    >> 16 == (BOT_BOARD & ROTATE_1_BACK_3));
    assert(ROTATE_1_FORWARD_9 >> 16 == (BOT_BOARD & ROTATE_1_FORWARD_9));
    assert(ROTATE_1_BACK_1    >> 16 == (BOT_BOARD & ROTATE_1_BACK_1));
    assert(ROTATE_1_FORWARD_3 >> 16 == (BOT_BOARD & ROTATE_1_FORWARD_3));
    assert((ROTATE_1_BACK_3 | ROTATE_1_FORWARD_9 | ROTATE_1_BACK_1 | ROTATE_1_FORWARD_3) == (((unsigned long long)1 << 32) - 1));
    assert((ROTATE_1_BACK_3 & ROTATE_1_FORWARD_9 & ROTATE_1_BACK_1 & ROTATE_1_FORWARD_3) == 0);

    assert(ROTATE_2_BACK_6    >> 16 == (BOT_BOARD & ROTATE_2_BACK_6));
    assert(ROTATE_2_FORWARD_6 >> 16 == (BOT_BOARD & ROTATE_2_FORWARD_6));
    assert(ROTATE_2_FORWARD_2 >> 16 == (BOT_BOARD & ROTATE_2_FORWARD_2));
    assert(ROTATE_2_BACK_2    >> 16 == (BOT_BOARD & ROTATE_2_BACK_2));
    assert((ROTATE_2_BACK_6 | ROTATE_2_FORWARD_6 | ROTATE_2_FORWARD_2 | ROTATE_2_BACK_2) == (((unsigned long long)1 << 32) - 1));
    assert((ROTATE_2_BACK_6 & ROTATE_2_FORWARD_6 & ROTATE_2_FORWARD_2 & ROTATE_2_BACK_2) == 0);

    assert(ROTATE_3_FORWARD_3 >> 16 == (BOT_BOARD & ROTATE_3_FORWARD_3));
    assert(ROTATE_3_BACK_9    >> 16 == (BOT_BOARD & ROTATE_3_BACK_9));
    assert(ROTATE_3_FORWARD_1 >> 16 == (BOT_BOARD & ROTATE_3_FORWARD_1));
    assert(ROTATE_3_BACK_3    >> 16 == (BOT_BOARD & ROTATE_3_BACK_3));
    assert((ROTATE_3_FORWARD_3 | ROTATE_3_BACK_9 | ROTATE_3_FORWARD_1 | ROTATE_3_BACK_3) == (((unsigned long long)1 << 32) - 1));
    assert((ROTATE_3_FORWARD_3 & ROTATE_3_BACK_9 & ROTATE_3_FORWARD_1 & ROTATE_3_BACK_3) == 0);

    int botPlaced = 0;
    while(botPlaced <= BOARD_SIZE/2)
    {
        generateBotBinarys(allPositions, 0, 0, botPlaced);
        botPlaced += 1;
    }
    std::cout << "Position Table Size: " << allPositions.size() << std::endl;

    generateScoreMap();

    int nodes = 0;
    int winValue = botNegamax(0, -1, 1, nodes);

    if(winValue == 1)
    {
        std::cout << "First player wins after " << nodes << " nodes." << std::endl;
    }else
    {
         std::cout << "...What?";
         assert(0);
    }

//    std::cout << "\n\nPress Enter to play." <<std::endl;
//    std::cin.get();
//    interactiveBotPlay(0);

//    randomWalk();

    return 0;
}
