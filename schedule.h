#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <queue>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stack>

// Variable Types
#define INPUTS 0
#define OUTPUTS 1
#define REGS 2

// Operator Types
#define REG 4
#define ADD32 5
#define SUB32 6
#define MUL32 7
#define COMPGT 8
#define COMPLT 9
#define COMPEQ 10
#define MUX2X1 11
#define SHL 12
#define SHR 13

//State Types
#define NORMAL 14
#define IFTHEN 15
#define WHILE 16
#define DOWHILE 17
#define DUMMY 18
#define DO 19
// Schedule Option Types
#define NS 20
#define LISTL 21
#define LISTR 22

//Bracket Types
#define PARENT_R 23
#define PARENT_L 24
#define CURLYBRKT_R 25
#define CURLYBRKT_L 26

using namespace std;

typedef struct node{
	int nodeType;
	string name;
} Node;// Node is variable in c-like file

typedef struct vertex{
	int lineNum;
	int vertexType;
	int cycle;
	int alapTime;
	int slackTime;
	bool scheduled;
	int scheduledTime;
	bool ctrlStmtFlag;// if true, this vertex is considered as a control statement
	Node *condNode;
	int ctrlType;
	int endLineNum;
	int addType4Out;
} Vertex;// Vertex is operator or a control statement if ctrlStmtFlag is true in c-like file

typedef struct edge{
	Vertex *vertex;
	Node *node;
	bool v2n;// edge direction, if 1, vertex to node,if 0, node to vertex
} Edge;// Edge connected between Node and Vertex

typedef struct state{
	int stateType;
	int ID;
	int lineNum;// used to identify states, when -ns
	int timeStep;// used when -listr or -listl
	vector<Vertex *> stateVec;// The vertices contained in this state
	vector<Vertex *> nextState_0;// next state when cond is false 
	vector<Vertex *> nextState_1;// next state when cond is true, if no cond, this is used as nextState
} State;// State is the state data in HLSM

class Schedule
{
public:
	Schedule(void);
	~Schedule(void);
	void setAttribues(char *cfile, char *verilogfile, int optionType);// -ns
    void setAttribues(char *cfile, char *verilogfile, char *mul, char *add_sub, char *logicOp, int optionType);// -listl
	void setAttribues(char *cfile, char *verilogfile, char *latency, int optionType);// -listr
	void parseCFile();
	void gnrtStates_NS();
	void gnrtStates_LS();
    void gnrtStates_LR();
	void write2VerilogFile_NS();
	void write2VerilogFile_LSR();
private:
	// attributes
	char *cfile;
	char *verilogfile;
	int mul;
	int add_sub;
	int logicOp;
	int latency;
    int optionType;
    ofstream outFile;
	//////////////////////////////////////////////////////////////////////////

	vector<Vertex *> vertices;
	vector<Node *> nodes;
	vector<Edge *> edges;
	vector<State *> states;
	stack<int> stkIdxInVrtcs;
	string strInputs;
	string strOutputs ;
	string strRegs ;
	//functions 

	// parse C-like file
	void parseInstLine(char *cstr, int lineNum);
	void parseDeclLine(char *cstr, int nodeType, int lineNum);
	void checkDigOrLet(char *token, int lineNum);
	int checkNodesDecled(char *token,int lineNum);
	int findOpTypeBySign(char *signOp);
	int findCtrlStmtType(char *token,int lineNum);
	
	int findNextStateVec(Vertex *tmpVertex, bool cond);

	void writeDecl();
	void writePrmt();
	string convertInt2Str(int number);
	void writeStates_NS();
	string intlOutReg();
	string state2Expression(State *curtState);
	string nextStateExpression(State *curtState);
	string findStateByVertex(Vertex *startVertex);
	vector<Node *> findAdjNodes(Vertex *startVertex);
	bool checkConnected(Node *tmpNode, Vertex *startVertex);
	vector<Edge *> findAdjEdges(Vertex *startVertex);
	string findSignByType(int opType);
	void writeStates_LSR();
	void drmnAndScdlVertces_LS(vector<int> candVertices, vector<int> unfnVertices, int timeStep);
	vector<int>  findCandVerticesIdx();
	vector<int>  findUNFNVerticesIdx();
	void  intlVerCycle();
	vector<int>  findStartVerticesIdx();
	vector<int> findOutputsVerIdx();
	bool allScdled(vector<int> outputsVerticesIdx);
	vector<int> findDescAdjVerticesIdx(Vertex *startVertex);
    vector<int> findPreAdjVerticesIdx(Vertex *startVertex);
	bool checkVertexConnected(Vertex *v1, Vertex *v2);
	void drmnAndScdlVertces_LR(vector<int> candVertices, vector<int> unfnVertices, int timeStep);
	int ALAP(vector<int> outputsVertices, int curLatency);
	vector<int>  pickSmallestSlack(vector<int>candTypeVec,int num);
	void checkArgumentNum(int signType,int tokenIndx,int lineNum);
	int checkCurlyBktRType(Vertex *startVertex);
};
