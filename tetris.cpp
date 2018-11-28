#include <vector>
#include <string>
#include <queue>
#include <cstring>
#include <cstdlib>
#include <iostream>

using namespace std;

const int BEST_K_TO_GET = 30;
const int ROWS = 20;
const int COLS = 10;
const int BADNESS_OF_A_HOLE = 10;
const int MAX_FIGURES = 100000 + 5;
const int LEVELS_TO_DEEPEN = 12;

struct cell {
    int row, col;
    cell(int _row, int _col): row(_row), col(_col) {};
};

bool cellIsInTheTable(int row, int col) {
    return row >= 0 && row < ROWS && col >= 0 && col < COLS;
}

struct figure {
    string col[4];

    figure(string _col0, string _col1, string _col2, string _col3) {
        col[0] = _col0;
        col[1] = _col1;
        col[2] = _col2;
        col[3] = _col3;
    }
};

vector<figure> figuresRotates[7];

// Precalculation of the figures samples

void precalculateFigures() {
    // Figure 0 - the straight one
    figuresRotates[0].push_back(figure("1111",
                                       "0000",
                                       "0000",
                                       "0000"));

    figuresRotates[0].push_back(figure("1000",
                                       "1000",
                                       "1000",
                                       "1000"));

    // Figure 1 - the gamma-1
    figuresRotates[1].push_back(figure("1110",
                                       "0010",
                                       "0000",
                                       "0000"));

    figuresRotates[1].push_back(figure("0100",
                                       "0100",
                                       "1100",
                                       "0000"));

    figuresRotates[1].push_back(figure("1000",
                                       "1110",
                                       "0000",
                                       "0000"));

    figuresRotates[1].push_back(figure("1100",
                                       "1000",
                                       "1000",
                                       "0000"));

    // Figure 2 - the gamma-2
    figuresRotates[2].push_back(figure("0010",
                                       "1110",
                                       "0000",
                                       "0000"));

    figuresRotates[2].push_back(figure("1000",
                                       "1000",
                                       "1100",
                                       "0000"));

    figuresRotates[2].push_back(figure("1110",
                                       "1000",
                                       "0000",
                                       "0000"));

    figuresRotates[2].push_back(figure("1100",
                                       "0100",
                                       "0100",
                                       "0000"));

    // Figure 3 - the square
    figuresRotates[3].push_back(figure("1100",
                                       "1100", "0000", "0000"));

    // Figure 5 - T-like
    figuresRotates[5].push_back(figure("1110",
                                       "0100", "0000", "0000"));

    figuresRotates[5].push_back(figure("0100",
                                       "1100",
                                       "0100", "0000"));

    figuresRotates[5].push_back(figure("0100",
                                       "1110", "0000", "0000"));

    figuresRotates[5].push_back(figure("1000",
                                       "1100",
                                       "1000", "0000"));

    // Figure 4 - the lebed-1
    figuresRotates[4].push_back(figure("0110",
                                       "1100", "0000", "0000"));

    figuresRotates[4].push_back(figure("1000",
                                       "1100",
                                       "0100", "0000"));

    // Figure 6 - the lebed-2
    figuresRotates[6].push_back(figure("1100",
                                       "0110", "0000", "0000"));

    figuresRotates[6].push_back(figure("0100",
                                       "1100",
                                       "1000", "0000"));
}

int figCount;
int figures[MAX_FIGURES];

void readInput() {
    scanf("%d", &figCount);

    for (int i = 0; i < figCount; ++i) {
        scanf("%d", &figures[i]);
        --figures[i];
    }
}

struct pairRotationColumn {
    int rotation;
    int leftmostColumn;

    pairRotationColumn() {};

    pairRotationColumn(int rot, int col) : rotation(rot), leftmostColumn(col) {};
};

struct board {
    int columns[COLS];
    int boardBadness;
    pairRotationColumn historyUpToNow[LEVELS_TO_DEEPEN];
    int historyUpToNowSize;

    board() {
        for (int i = 0; i < COLS; ++i) columns[i] = 0;
        boardBadness = 0;
        historyUpToNowSize = 0;
    }

    int getColumnHeight(int col) { // The highest filled row in the column
        int height = 0;

        for (int row = ROWS - 1; row >= 0; --row)
            if (isFillCell(row, col)) {
                height = row;
                break;
            }

        return height;
    }

    bool isFreeCell(int row, int col) {
        return ( columns[col] & (1 << row) ) == false;
    }

    bool isFillCell(int row, int col) {
        return ! isFreeCell(row, col);
    }

    void setCell(int row, int col) {
        columns[col] |= (1 << row);
    }

    void unsetCell(int row, int col) {
        columns[col] &= ~(1 << row);
    }

    bool isFullRow(int row) {
        for (int col = 0; col < COLS; ++col)
            if (isFreeCell(row, col)) return false;

        return true;
    }

    // TO-DO: WHY WE SHOULD SWAP THE SIGN HERE ?!
    bool operator < (const board& right) const {
        return boardBadness > right.boardBadness;
    }

    void print() {
        for (int i = ROWS - 1; i >= 0; --i) {
            for (int j = 0; j < COLS; ++j) {
                printf("%d", isFillCell(i, j));
            }
            printf("\n");
        }
    }

    int getBoardBadnessEvaluation() {
        int badness = 0;
        int holes = 0;

        for (int col = 0; col < COLS; ++col) {
            bool filledCell = false;
            int height = 0;

            for (int row = ROWS - 1; row >= 0; --row) {
                if (filledCell && isFreeCell(row, col)) {
                    ++holes;
                }

                if (isFillCell(row, col)) filledCell = true;
            }

            badness += getColumnHeight(col);
        }

        badness += holes * BADNESS_OF_A_HOLE;
        return badness;
    }

    bool isPossibleToExtend(int col, figure nextFigure) {
        int rowToPut, columnToPut;

        for (int lowestRow = ROWS - 1; lowestRow >= 0; --lowestRow) {
            bool failed = false;

            for (int i = 0; i < 4 && ! failed; ++i)
                for (int j = 0; j < 4 && ! failed; ++j)
                    if (nextFigure.col[i][j] == '1') {
                        rowToPut = lowestRow + j;
                        columnToPut = col + i;

                        if (! cellIsInTheTable(rowToPut, columnToPut) || isFillCell(rowToPut, columnToPut)) {
                            failed = true;
                        }
                    }

            if (! failed) return true;
        }

        return false;
    }

    bool hasFullRows() {
        for (int row = 0; row < ROWS; ++row)
            if (isFullRow(row)) return true;

        return false;
    }
};

board deleteRows(board theBoard) {
    for (int row = 0; row < ROWS; ++row)
        if (theBoard.isFullRow(row))
            for (int col = 0; col < COLS; ++col)
                theBoard.unsetCell(row, col);

    return theBoard;
}

vector<cell> currentGroup;
bool used[ROWS][COLS];
board beginningBoard, currentBoard;

// DFS is based on the original board
void dfs(int row, int col) {
    if (! cellIsInTheTable(row, col) || beginningBoard.isFreeCell(row, col)) return;
    if (used[row][col]) return;

    currentGroup.push_back(cell(row, col));
    used[row][col] = true;

    dfs(row - 1, col);
    dfs(row + 1, col);
    dfs(row, col - 1);
    dfs(row, col + 1);
}

void clearOldCells(vector<cell>& currentGroup) {
    for (int i = 0; i < currentGroup.size(); ++i) {
        int row = currentGroup[i].row;
        int col = currentGroup[i].col;

        currentBoard.unsetCell(row, col);
    }
}

void revertOldCells(vector<cell>& currentGroup) {
    for (int i = 0; i < currentGroup.size(); ++i) {
        int row = currentGroup[i].row;
        int col = currentGroup[i].col;

        currentBoard.setCell(row, col);
    }
}

void putNewCells(vector<cell>& currentGroup, int rowsDownwards) {
    for (int i = 0; i < currentGroup.size(); ++i) {
        int row = currentGroup[i].row - rowsDownwards;
        int col = currentGroup[i].col;

        currentBoard.setCell(row, col);
    }
}

// With these methods we "down" pieces in a board
void putDownTheComponentOf(int row, int col) {
    currentGroup.clear();
    dfs(row, col);
    clearOldCells(currentGroup);

    // Let we delete them from the current board state and check whether we can move some rows down
    // If we cannot, just revert :)
    bool haveAnswer = false;

    int lowestRow;
    for (int i = 0; i < currentGroup.size(); ++i) {
        lowestRow = min(lowestRow, currentGroup[i].row);
    }

    int successfulDowning = 0;

    for(int i = lowestRow - 1; i >= 0; --i) {
        int differenceFromLowestRow = lowestRow - i;
        bool failed = false;

        int newRowJ, newColJ;
        for (int j = 0; j < currentGroup.size() && ! failed; ++j) {
            newRowJ = currentGroup[j].row - differenceFromLowestRow;
            newColJ = currentGroup[j].col;
            if (! cellIsInTheTable(newRowJ, newColJ) || currentBoard.isFillCell(newRowJ, newColJ)) failed = true;
        }

        if (failed && haveAnswer) break;
        if (! failed) {
            successfulDowning = differenceFromLowestRow;
            haveAnswer = true;
        }
    }

    putNewCells(currentGroup, successfulDowning);
}

board putGroupDown(board board) {
    currentBoard = board;
    beginningBoard = board;

    memset(used, 0, sizeof(used));

    for (int row = 0; row < ROWS; ++row)
        for (int col = 0; col < COLS; ++col)
            if (! used[row][col]) {
                putDownTheComponentOf(row, col);
            }

     return currentBoard;
}

// Get the first K used to continue
priority_queue<board> getFirstK(priority_queue<board>& currentLevelQueue) {
    priority_queue<board> resultQueue;

    for (int i = 0; i < BEST_K_TO_GET && ! currentLevelQueue.empty(); ++i) {
        resultQueue.push(currentLevelQueue.top());
        currentLevelQueue.pop();
    }

    return resultQueue;
}

// Put figure to the board
board putFigureToTheBoard(board theBoard, int col, figure nextFigure) {
    board boardToModify, returnedBoard;

    int rowToPut, columnToPut;
    bool haveResult = false;

    for (int lowestRow = ROWS - 1; lowestRow >= 0; --lowestRow) {
        boardToModify = theBoard;
        bool failed = false;

        for (int i = 0; i < 4 && ! failed; ++i)
            for (int j = 0; j < 4 && ! failed; ++j)
                if (nextFigure.col[i][j] == '1') {
                    rowToPut = lowestRow + j;
                    columnToPut = col + i;

                    if (! cellIsInTheTable(rowToPut, columnToPut) || boardToModify.isFillCell(rowToPut, columnToPut)) {
                        failed = true;
                    } else {
                        boardToModify.setCell(rowToPut, columnToPut);
                    }
                }

        if (failed && haveResult) return returnedBoard;
        if (! failed) returnedBoard = boardToModify, haveResult = true;
    }

    return returnedBoard;
}

vector<pairRotationColumn> answerHistory;

// TO-DO: EDNA SLABOST IMA - AKO PADNE STURCHEIKI, IZTRIQT SE I SPRE DA STARCHI -> produljavame igrata

// Return the last board - the optima

board solve() {
    board bestCurrentBoard, nextBestBoard; // empty in the beginning
    board answ;

    // Start to put figures
    for (int i = 0; i < figCount; i += LEVELS_TO_DEEPEN) {
        board bestCurrentBoardWithoutHistory = bestCurrentBoard;
        board nextBestBoard;
        bestCurrentBoardWithoutHistory.historyUpToNowSize = 0;

        // Initialise the queue
        priority_queue<board> currentLevelQueue;
        currentLevelQueue.push(bestCurrentBoardWithoutHistory);

        for (int j = i; j < min(i + LEVELS_TO_DEEPEN, figCount); ++j) {
            int currentFig = figures[j];
            board nextStepBoard;

            nextBestBoard = currentLevelQueue.top();
            priority_queue<board> nextLevelQueue;

            while (! currentLevelQueue.empty()) {
                board onTheTopBoard = currentLevelQueue.top();
                currentLevelQueue.pop();

                for (int col = 0; col < COLS; ++col) {
                    for (int rot = 0; rot < figuresRotates[currentFig].size(); ++rot) {
                        figure figureToPut = figuresRotates[currentFig][rot];

                        if (onTheTopBoard.isPossibleToExtend(col, figureToPut)) {
                            nextStepBoard = putFigureToTheBoard(onTheTopBoard, col, figureToPut);

                            // The pieces go down, and down, and down...
                            while (nextStepBoard.hasFullRows()) {
                                nextStepBoard = deleteRows(nextStepBoard);
                                nextStepBoard = putGroupDown(nextStepBoard);
                            }

                            // Calculate badness of the new board and add the (rot, col) tuple to the history
                            nextStepBoard.boardBadness = nextStepBoard.getBoardBadnessEvaluation();
                            nextStepBoard.historyUpToNow[nextStepBoard.historyUpToNowSize++] = pairRotationColumn(rot, col);

                            // Push to the next level queue
                            nextLevelQueue.push(nextStepBoard);
                        }
                    }
                }
            }

            if (nextLevelQueue.empty()) break; // If no new states, break
            else if(j == min(i + LEVELS_TO_DEEPEN, figCount) - 1) nextBestBoard = nextLevelQueue.top(); // If we are at the end, get the best solution
            else if (nextLevelQueue.size() > BEST_K_TO_GET) currentLevelQueue = getFirstK(nextLevelQueue); // If will continue, get first K
            else currentLevelQueue = nextLevelQueue; // or the whole queue
        }

        // Update current best board and add to its history
        for (int i = 0; i < COLS; ++i) {
            bestCurrentBoard.columns[i] = nextBestBoard.columns[i];
        }

        for (int i = 0; i < nextBestBoard.historyUpToNowSize; ++i) {
            answerHistory.push_back(nextBestBoard.historyUpToNow[i]);
        }
    }

    return bestCurrentBoard;
}

void writeOutput(board bestCurrentBoard) {

    int puttedWell = answerHistory.size();
    for (int i = 0; i < puttedWell; ++i) {
        printf("%d %d ", answerHistory[i].rotation, answerHistory[i].leftmostColumn);
    }

    // Dummy fillings to the end
    for (int i = 0; i < figCount - puttedWell; ++i) {
        printf("2 4 ");
    }

    /*
    // --------------
    // Demo the falling
    board startingBoard;
    board nextStepBoard;

    // Start to put figures
    for (int i = 0; i < puttedWell; i ++) {
        int currentFig = figures[i];
        int rot = bestCurrentBoard.historyUpToNow[i].rotation;
        int col = bestCurrentBoard.historyUpToNow[i].leftmostColumn;

        figure figureToPut = figuresRotates[currentFig][rot];
        nextStepBoard = putFigureToTheBoard(startingBoard, col, figureToPut);

        // The pieces go down, and down, and down...
        while (nextStepBoard.hasFullRows()) {
            nextStepBoard = deleteRows(nextStepBoard);
            nextStepBoard = putGroupDown(nextStepBoard);
        }

        startingBoard = nextStepBoard;

//        cout << "figure: " << currentFig + 1 << " ";
  //      cout << "col: " << col << " ";
    //    cout << "rot: " << rot << " ";
      //  cout << endl;

        // nextStepBoard.print();
    }
    */

}

int main() {

    // TO-DO: SMENQ BITOVATA MASKA OT COLONNA NA REDOVA !!
    precalculateFigures();

    readInput();

    writeOutput(solve());

    return 0;
}
