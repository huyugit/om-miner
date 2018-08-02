#ifndef BOARDTESTER_H
#define BOARDTESTER_H

class BoardTester
{
public:
    BoardTester(int _slaveId, int _boardId);
    bool test();

private:
    int slaveId;
    int boardId;

    bool testBoardParams();
};

#endif // BOARDTESTER_H
