#include "schedule.h"

Schedule::Schedule(void)
{
}

Schedule::~Schedule(void)
{
}
void Schedule::setAttribues(char *cfile, char *verilogfile,int optionType)
{
	this->cfile = cfile;
	this->verilogfile = verilogfile;
	this->optionType = optionType;
}

void Schedule::setAttribues(char *cfile, char *verilogfile,char *mul, char *add_sub,char *logicOp,int optionType)
{
	this->cfile = cfile;
	this->verilogfile = verilogfile;
	this->add_sub = atoi(add_sub);
	this->mul = atoi(mul);
	this->logicOp = atoi(logicOp);
	this->optionType = optionType;
}

void Schedule::setAttribues(char *cfile, char *verilogfile, char *latency,int optionType)
{
	this->cfile = cfile;
	this->verilogfile = verilogfile;
	this->latency = atoi(latency);
	this->optionType = optionType;
}

void Schedule::parseCFile()
{
	string strLine;
	int lineNum = 0;

	int latestDeclareLineNum = 0;
	int initCmnptLineNum = 0;
	int declREGNum = 0;
	//int declWireNum = 0;
	int declInputNum = 0;
	int declOutputNum = 0;

	ifstream in(this->cfile);
	if(!in){
		cerr << "Error when reading " << this->cfile << ". Program terminated!" << endl;
		system("Pause");exit(1);
	}
	while (getline(in, strLine))// start parsing cfile
	{
		lineNum++;
		int tokenIndx = 0;
		char *cstr = new char[strLine.length() + 1];
		char *ccstr = new char[strLine.length() + 1];

		strcpy(cstr, strLine.c_str());
		strcpy(ccstr, strLine.c_str());
		char *token = strtok(cstr, " \n\t\v\f\r");

		bool declFlag = false;// Flag sign of declaration part
		int varType;
		while(token != 0){// parsing each line

			if (!strcmp(token,";"))// check semicolons
			{
				cerr << "Syntax error is detected at line " << lineNum <<": semicolons "<< token<<" should not be here."<< endl;
				system("Pause");exit(1);
			}

			if (token[0] == '/')// When beginning with '/'
			{
				if (strlen(token) == 1)// There is "/ a ", this kind of syntax error, in file 
				{
					cerr << "In C file, syntax error is detected at line " << lineNum << endl;
					system("Pause");exit(1);
				}
				else
				{
					if (token[1] == '/')// Ignore the line beginning with "//"
					{
						break;
					}
					else
					{// There is "/a ", this kind of syntax error, in file
						cerr << "In C file, syntax error is detected at line " << lineNum << endl;
						system("Pause");exit(1);
					}
				}
			}

			else
			{
				if(tokenIndx == 0)
				{	
					if (!strcmp(token,"INPUTS"))
					{
						declInputNum ++;
						if (declInputNum > 2)
						{
							cerr << "Syntax error is detected at line " << lineNum <<": More than one line of"<< token<<" is decalred."<< endl;
							system("Pause");exit(1);
						}
						declFlag = true;
						varType = INPUTS;
					}

					if (!strcmp(token,"OUTPUTS"))
					{
						declOutputNum ++;
						if (declOutputNum > 2)
						{
							cerr << "Syntax error is detected at line " << lineNum <<": More than one line of"<< token<<" is decalred."<< endl;
							system("Pause");exit(1);
						}
						declFlag = true;
						varType = OUTPUTS;

					}

					if (!strcmp(token,"REGS"))
					{


						declREGNum ++;
						if (declREGNum > 2)
						{
							cerr << "Syntax error is detected at line " << lineNum <<": More than one line of"<< token<<" is decalred."<< endl;
							system("Pause");exit(1);
						}
						declFlag = true;
						varType = REGS;
					}

					if (!declFlag) //if not above cases, then instantiations should start from this line
					{
						// if number of total declarations so far is smaller than  (declREGNum + declWireNum + declInputNum + declOutputNum)
						// which means instantiation appear before certain declaration
						int totalDeclLineNum = declREGNum + declInputNum + declOutputNum;

						if(declREGNum)// if regs exist
						{
							if (totalDeclLineNum < 3)
							{	
								cerr << "C file syntax error at line " << lineNum <<": Instantiations come before declaration." << endl;

								system("Pause");exit(1);
							}

						}
						else{// if regs not exist
							if (totalDeclLineNum < 2)
							{	
								cerr << "C file syntax error at line " << lineNum <<": Instantiations come before declaration." << endl;

								system("Pause");exit(1);
							}
						}
						parseInstLine(ccstr,lineNum);// parse the instantiations lines
					}

				}
				if (tokenIndx == 1)
				{
					if ((declFlag == true)&&(strcmp(token, ":")))// if still declaration part, and 2nd token is not '.'
					{
						cerr << "C file syntax error at line " << lineNum << ": colon ':' should appear here."<< endl;
						system("Pause");exit(1);
					}
					else// 
					{
						if ((declFlag == true)&&(!strcmp(token, ":")))
						{
							parseDeclLine(ccstr,varType,lineNum);
						}

					}
				}
			}
			tokenIndx ++ ;
			token = strtok(NULL, " \n\t\v\f\r");

		}// End of parsing each line
	}// End of parsing C file
	if (!stkIdxInVrtcs.empty())
	{
		cerr << "C file syntax error : Lack '}' ."<< endl;
		system("Pause");exit(1);
	}
	
}


void Schedule::parseDeclLine(char *cstr,int varType, int lineNum){
	int tokenIndx = 0;// we have checked 'TYPE' and ':' before calling this function, so starts from 2
	char *token = strtok(cstr, " \n\t\v\f\r");
	int i;
	while (token != 0)
	{
		if(tokenIndx>1)
		{
			if (!strcmp(token,";"))// check semicolons
			{
				cerr << "Syntax error is detected at line " << lineNum <<": semicolons "<< token<<" should not be here."<< endl;
				system("Pause");exit(1);
			}
			checkDigOrLet(token,lineNum);
			Node *tmpNode = new Node;
			tmpNode->name = token;
			tmpNode->nodeType = varType;
			// check whether this token has been declared. 
			for (i = 0;i<nodes.size();i++)
			{
				char *cStrNodesName = new char[nodes[i]->name.length() + 1];
				strcpy(cStrNodesName, nodes[i]->name.c_str());
				if (!strcmp(token,cStrNodesName))
				{
					cerr << "C file syntax error at line " << lineNum << ": "<<token<< " is declared 2nd time"<< endl;	
					system("Pause");exit(1);
				}
			}
			nodes.push_back(tmpNode);	   
		}


		tokenIndx ++;
		token = strtok(NULL, " \n\t\v\f\r");
	}
}

void Schedule::checkDigOrLet(char *token, int lineNum)// check if this token is valid variable name
{
	int i = 0;
	bool constantFlag = false;
	for (i = 0;i< strlen(token);i ++)
	{
		if (!(isdigit(token[i])||isalpha(token[i])))
		{
			cerr << "C file syntax error at line " << lineNum << ": "<<token[i] << " is not a digit or letter."<< endl;	
			system("Pause");exit(1);
		}
		else
		{
			if (isalpha(token[i]))
			{
				constantFlag = true;
			}
		}
	}
	if (!constantFlag)
	{
		cerr << "C file syntax error at line " << lineNum << ": "<<token << " is a constant."<< endl;	
		system("Pause");exit(1);
	}

}

void Schedule::parseInstLine(char *cstr, int lineNum)
{
	int tokenIndx = 0;
	int varType = 0;
	int signType = 0;
	int ctrlStmtType = 0;
	bool boolCtrlStmtFlag = false;
	bool boolRegFlag = false;
	vector<Node *> nodesLine;

	char *token = strtok(cstr, " \n\t\v\f\r");


	while (token != 0)
	{
		if (!strcmp(token,";"))// check semicolons
		{
			cerr << "Syntax error is detected at line " << lineNum <<": semicolons "<< token<<" should not be here."<< endl;
			system("Pause");exit(1);
		}

		if (tokenIndx == 0)
		{
			ctrlStmtType = findCtrlStmtType(token,lineNum);
            if (ctrlStmtType != -1)
			{

				if (ctrlStmtType != CURLYBRKT_R)
				{
					Vertex *tmpVertex = new Vertex;
					tmpVertex->lineNum = lineNum;
					tmpVertex->ctrlStmtFlag = true;
					tmpVertex->scheduled = false;
					tmpVertex->ctrlType = ctrlStmtType;
					vertices.push_back(tmpVertex);
					stkIdxInVrtcs.push(vertices.size()-1);

				}
				else// if corresponding right curly bracket is found
				{
					vertices[stkIdxInVrtcs.top()]->endLineNum = lineNum;
					if (vertices[stkIdxInVrtcs.top()]->ctrlType == DO)
					{
						ctrlStmtType = DOWHILE;
						Vertex *tmpVertex = new Vertex;
						tmpVertex->lineNum = lineNum;
						tmpVertex->ctrlStmtFlag = true;
						tmpVertex->scheduled = false;
						tmpVertex->ctrlType = ctrlStmtType;
						vertices.push_back(tmpVertex);
					}
					else
					{
						ctrlStmtType = CURLYBRKT_R;
						Vertex *tmpVertex = new Vertex;
						tmpVertex->lineNum = lineNum;
						tmpVertex->ctrlStmtFlag = true;
						tmpVertex->scheduled = false;
						tmpVertex->ctrlType = ctrlStmtType;
						vertices.push_back(tmpVertex);
					}
					stkIdxInVrtcs.pop();
				   
				}
			}
			
			else// not control statement, i.e. NORMAL expression 
			{
				checkDigOrLet(token,lineNum);
				varType = checkNodesDecled(token,lineNum);//if declared , return variable type, if not, -1
				if (varType == -1)
				{
					cerr << "C file syntax error at line " << lineNum << ": "<<token << " is not declared in C file."<< endl;	
					system("Pause");exit(1);
				}
				if (varType == INPUTS)
				{
					cerr << "C file syntax error at line " << lineNum << ": "<<token << " is INPUT, which should not appear here."<< endl;	
					system("Pause");exit(1);
				}
				if ((varType == OUTPUTS))
				{
					Vertex *tmpVertex = new Vertex;
					tmpVertex->lineNum = lineNum;
                    tmpVertex->scheduled = false;
				    tmpVertex->vertexType = OUTPUTS;//but consider outputs as REG
					signType = OUTPUTS;	
					tmpVertex->ctrlStmtFlag = false;
					tmpVertex->addType4Out = -1;
					tmpVertex->ctrlType = NORMAL;
					vertices.push_back(tmpVertex);
					boolRegFlag = true;// this line is Reg instantiation
				}
				Node *tmpNode = new Node;
				tmpNode->name = token;
				tmpNode->nodeType = varType;
				nodesLine.push_back(tmpNode);
			}
		}
		if (tokenIndx == 1)
		{
			if (ctrlStmtType != -1)
			{

				if (ctrlStmtType != DOWHILE)
				{
					if (ctrlStmtType != DO)
					{
						if (strcmp(token,"("))
						{
							cerr << "C file syntax error at line " << lineNum << ": '(' should appear here."<< endl;	
							system("Pause");exit(1);
						}
					}
					else
					{
						if (strcmp(token,"{"))
						{
							cerr << "C file syntax error at line " << lineNum << ": '{' should appear here."<< endl;	
							system("Pause");exit(1);
						}
					}
				}
				else
				{
					if (strcmp(token,"while"))
					{
						cerr << "C file syntax error at line " << lineNum << ": keyword 'while' should appear here."<< endl;	
						system("Pause");exit(1);
					}
				}
			}

			else// not control statement 
			{
				if (strcmp(token,"="))
				{
					cerr << "C file syntax error at line " << lineNum << ": '=' should appear here."<< endl;	
					system("Pause");exit(1);
				}
			}
		}
		if (tokenIndx == 2)
		{
			if (ctrlStmtType != -1)
			{
				if ((ctrlStmtType != DOWHILE)&&(ctrlStmtType != DO))
				{

					checkDigOrLet(token,lineNum);
					varType = checkNodesDecled(token,lineNum);
					if (varType == -1)
					{
						cerr << "C file syntax error at line " << lineNum << ": "<<token << " is not declared in C file."<< endl;	
						system("Pause");exit(1);
					}
					Node *tmpNode = new Node;
					tmpNode->name = token;
					tmpNode->nodeType = varType;
					vertices[vertices.size() - 1]->condNode = tmpNode;

				}
				else
				{
					if(ctrlStmtType == DOWHILE)
					{
						if (strcmp(token,"("))
						{
							cerr << "C file syntax error at line " << lineNum << ": '(' should appear here."<< endl;	
							system("Pause");exit(1);
						}
					}
					else
					{
						if (strcmp(token,"}"))
						{
							cerr << "C file syntax error at line " << lineNum << ": '}' should appear here."<< endl;	
							system("Pause");exit(1);
						}
					}
				}
			}

			else// not control statement 
			{

				checkDigOrLet(token,lineNum);
				varType = checkNodesDecled(token,lineNum);
				if (varType == -1)
				{
					cerr << "C file syntax error at line " << lineNum << ": "<<token << " is not declared in C file."<< endl;	
					system("Pause");exit(1);
				}
				Node *tmpNode = new Node;
				tmpNode->name = token;
				tmpNode->nodeType = varType;
				nodesLine.push_back(tmpNode);
				if(boolRegFlag)
				{
					Edge *tmpEdge0 = new Edge;
					tmpEdge0->node = nodesLine[nodesLine.size() - 2];
					tmpEdge0->vertex = vertices[vertices.size() - 1];
					tmpEdge0->v2n = true;
					edges.push_back(tmpEdge0);

					Edge *tmpEdge1 = new Edge;
					tmpEdge1->node = nodesLine[nodesLine.size() - 1];
					tmpEdge1->vertex = vertices[vertices.size() - 1];
					tmpEdge1->v2n = false;
					edges.push_back(tmpEdge1);

				}

			}
		}
		if (tokenIndx == 3)
		{
			if (ctrlStmtType != -1)
			{

				if (ctrlStmtType != DOWHILE)
				{

					if (strcmp(token,")"))
					{
						cerr << "C file syntax error at line " << lineNum << ": ')' should appear here."<< endl;	
						system("Pause");exit(1);
					}
				}
				else
				{
					checkDigOrLet(token,lineNum);
					varType = checkNodesDecled(token,lineNum);
					if (varType == -1)
					{
						cerr << "C file syntax error at line " << lineNum << ": "<<token << " is not declared in C file."<< endl;	
						system("Pause");exit(1);
					}
					Node *tmpNode = new Node;
					tmpNode->name = token;
					tmpNode->nodeType = varType;
					vertices[vertices.size() - 1]->condNode = tmpNode;				
				}
			}
			else// not control statement 
			{
				
				signType = findOpTypeBySign(token);
				if (signType == -1)
				{
					cerr << "C file syntax error at line " << lineNum << ": "<<token << " is invalid operator."<< endl;	
					system("Pause");exit(1);
				}

				Vertex *tmpVertex = new Vertex;
				
				if (boolRegFlag)// needs additional op for this vertex 
				{
					vertices.pop_back();
					edges.pop_back();
					edges.pop_back();
					tmpVertex->vertexType = OUTPUTS;
					tmpVertex->addType4Out = signType;
				}
				else
				{
					tmpVertex->vertexType = signType;
					tmpVertex->addType4Out = -1;
				}
				
				
				tmpVertex->lineNum = lineNum;
				tmpVertex->ctrlStmtFlag = false;
				tmpVertex->ctrlType = NORMAL;
				tmpVertex->scheduled = false;

				vertices.push_back(tmpVertex);

				Edge *tmpEdge0 = new Edge;
				tmpEdge0->node = nodesLine[nodesLine.size() - 2];
				tmpEdge0->vertex = vertices[vertices.size() - 1];
				tmpEdge0->v2n = true;
				edges.push_back(tmpEdge0);

				Edge *tmpEdge1 = new Edge;
				tmpEdge1->node = nodesLine[nodesLine.size() - 1];
				tmpEdge1->vertex = vertices[vertices.size() - 1];
				tmpEdge1->v2n = false;
				edges.push_back(tmpEdge1);
			}

		}
		if (tokenIndx == 4)
		{

			if (ctrlStmtType != -1)
			{
				if (ctrlStmtType != DOWHILE)
				{
					if (strcmp(token,"{"))
					{
						cerr << "C file syntax error at line " << lineNum << ": '{' should appear here."<< endl;	
						system("Pause");exit(1);
					}

				}
				else
				{
					if (strcmp(token,")"))
					{
						cerr << "C file syntax error at line " << lineNum << ": ')' should appear here."<< endl;	
						system("Pause");exit(1);
					}
				}
			}

			else// not control statement 
			{
				checkDigOrLet(token,lineNum);
				varType = checkNodesDecled(token,lineNum);
				if (varType == -1)
				{
					cerr << "C file syntax error at line " << lineNum << ": "<<token << " is not declared in C file."<< endl;	
					system("Pause");exit(1);
				}
				Node *tmpNode = new Node;
				tmpNode->name = token;
				tmpNode->nodeType = varType;
				nodesLine.push_back(tmpNode);

				Edge *tmpEdge = new Edge;
				tmpEdge->node = nodesLine[nodesLine.size() - 1];
				tmpEdge->vertex = vertices[vertices.size() - 1];
				tmpEdge->v2n = false;
				edges.push_back(tmpEdge);
			}
		}
		if (tokenIndx == 5)// only MUX2X1 has so many arguments
		{
			if (signType == MUX2X1)
			{
				if (strcmp(token,":"))
				{
					cerr << "C file syntax error at line " << lineNum << ": ':' should appear here."<< endl;	
					system("Pause");exit(1);
				}
			}
			else
			{
				cerr << "C file syntax error at line " << lineNum << ": Too many arguments or invalid expression."<< endl;	
				system("Pause");exit(1);
			}
		}
		if (tokenIndx == 6)
		{
			if (signType == MUX2X1)
			{
				checkDigOrLet(token,lineNum);
				varType = checkNodesDecled(token,lineNum);
				if (varType == -1)
				{
					cerr << "C file syntax error at line " << lineNum << ": "<<token << " is not declared in C file."<< endl;	
					system("Pause");exit(1);
				}
				Node *tmpNode = new Node;
				tmpNode->name = token;
				tmpNode->nodeType = varType;
				nodesLine.push_back(tmpNode);

				Edge *tmpEdge = new Edge;
				tmpEdge->node = nodesLine[nodesLine.size() - 1];
				tmpEdge->vertex = vertices[vertices.size() - 1];
				tmpEdge->v2n = false;
				edges.push_back(tmpEdge);
			}
			else
			{
				cerr << "C file syntax error at line " << lineNum << ": Too many arguments or invalid expression."<< endl;	
				system("Pause");exit(1);
			}
		}

		tokenIndx ++;
		token = strtok(NULL, " \n\t\v\f\r");

	}
	if ((tokenIndx == 3)&&(nodesLine[0]->nodeType == REGS))
	{
		Vertex *tmpVertex = new Vertex;
		tmpVertex->lineNum = lineNum;
		tmpVertex->scheduled = false;
		tmpVertex->vertexType = REG;//but consider outputs as REG	
		tmpVertex->ctrlStmtFlag = false;
		tmpVertex->ctrlType = NORMAL;
		vertices.push_back(tmpVertex);
		Edge *tmpEdge0 = new Edge;
		tmpEdge0->node = nodesLine[nodesLine.size() - 2];
		tmpEdge0->vertex = vertices[vertices.size() - 1];
		tmpEdge0->v2n = true;
		edges.push_back(tmpEdge0);

		Edge *tmpEdge1 = new Edge;
		tmpEdge1->node = nodesLine[nodesLine.size() - 1];
		tmpEdge1->vertex = vertices[vertices.size() - 1];
		tmpEdge1->v2n = false;
		edges.push_back(tmpEdge1);

	}

	checkArgumentNum(signType,tokenIndx,lineNum);
}

void Schedule::checkArgumentNum(int signType,int tokenIndx,int lineNum)
{
	if ((signType == MUL32)||(signType == ADD32)||(signType == SUB32)||(signType == COMPEQ)||(signType == COMPGT)||(signType == COMPLT)||(signType == SHL)||(signType == SHR))
	{
		if (tokenIndx != 5)
		{
			cerr << "C file syntax error at line " << lineNum << ": Wrong number of arguments or invalid expression."<< endl;	
			system("Pause");exit(1);
		}
	}
	if ((signType == OUTPUTS))
	{
		if (tokenIndx != 3)
		{
			cerr << "C file syntax error at line " << lineNum << ": Wrong number of arguments or invalid expression."<< endl;	
			system("Pause");exit(1);
		}
	}
	if (signType == MUX2X1)
	{
		if (tokenIndx != 7)
		{
			cerr << "C file syntax error at line " << lineNum << ": Wrong number of arguments or invalid expression."<< endl;	
			system("Pause");exit(1);
		}
	}
}

int Schedule::checkNodesDecled(char *token,int lineNum)
{
	int i;
	bool decled = false;
	for (i=0;i < nodes.size();i++)
	{
		char *cStrNodesName = new char[nodes[i]->name.length() + 1];
		strcpy(cStrNodesName, nodes[i]->name.c_str());
		if (!strcmp(token,cStrNodesName))
		{
			decled = true;
			return nodes[i]->nodeType;
		}
	}
	if (!decled)
	{
		return -1;
	}


}

int Schedule::findOpTypeBySign(char *signOp){
	if (signOp[0] == '+')
	{
		return ADD32;
	}
	if (signOp[0] == '-')
	{
		return SUB32;
	}
	if (signOp[0] == '*')
	{
		return MUL32;
	}
	if( !strcmp(signOp,"=="))
	{
		return COMPEQ;
	}
	if (signOp[0] == '<')
	{
		if (!strcmp(signOp,"<<"))
		{
			return SHL;
		}
		return COMPLT;
	}
	if (signOp[0] == '>')
	{
		if (!strcmp(signOp,">>"))
		{
			return SHR;
		}
		return COMPGT;
	}
	if (signOp[0] == '?')
	{
		return MUX2X1;
	}
	return -1;// NOT found
}

int Schedule::findCtrlStmtType(char *token,int lineNum)
{
	if (!strcmp(token,"if"))
	{
		return IFTHEN;
	}

	if (!strcmp(token,"while"))
	{
		return WHILE;
	}
	if (!strcmp(token,"do"))
	{
		return DO;
	}
	if (!strcmp(token,"}"))
	{
		return CURLYBRKT_R;
	}
	return -1;
}

void Schedule::gnrtStates_NS()
{

	int i,idxNextStateInVec;
    for (i = 0;i < vertices.size();i++)
    {
   
		if ((vertices[i]->ctrlStmtFlag)&&(vertices[i]->ctrlType != DO)&&(vertices[i]->ctrlType != CURLYBRKT_R))// if control statement and NOT DO statement 
		{
			State *tmpState = new State;
			tmpState->stateType = vertices[i]->ctrlType;
			tmpState->lineNum = vertices[i]->lineNum;
			tmpState->stateVec.push_back(vertices[i]);

            idxNextStateInVec = findNextStateVec(vertices[i],true);// find the next state when cond is true
            tmpState->nextState_1.push_back(vertices[idxNextStateInVec]);
			idxNextStateInVec = findNextStateVec(vertices[i],false);// find the next state when cond is false
			if (idxNextStateInVec != -1)
			{
				tmpState->nextState_0.push_back(vertices[idxNextStateInVec]);		
			}
			

			states.push_back(tmpState);
		}
		else// if normal expression or DO statement or "}"
		{
			State *tmpState = new State;
			if (!vertices[i]->ctrlStmtFlag)
			{
				tmpState->stateType = NORMAL;
			}
			else
			{
				tmpState->stateType = vertices[i]->ctrlType;// type of this state will be changed to DUMMY
			}
			tmpState->lineNum = vertices[i]->lineNum;
			tmpState->stateVec.push_back(vertices[i]);

			idxNextStateInVec = findNextStateVec(vertices[i], true);// if normal expression, don't need to care cond, set cond to be true;
			if (idxNextStateInVec != -1)
			{
				tmpState->nextState_1.push_back(vertices[idxNextStateInVec]);
			}
	

			states.push_back(tmpState);
		}
    }

}
/*
 findNextStateVec Usage: for IFTHEN, WHILE,DOWHILE, findNextStateVec(tmpVertex,cond)
                         for DO, normal,findNextStateVec(tmpVertex, true)

*/

int Schedule::findNextStateVec(Vertex *tmpVertex, bool cond)
{
	int idxNextStateInVec,i;	

	if ((!tmpVertex->ctrlStmtFlag)||((tmpVertex->ctrlType == IFTHEN)||(tmpVertex->ctrlType == DO)))// if while or ifthen or do control statement or noraml expression, there should be 2 next states(true of false)
	{
		if (cond)// if cond is true, find the statement of next line as next state
		{
			for (i = 0;i < vertices.size();i++)
			{
				if (vertices[i]->lineNum > tmpVertex->lineNum)
				{
					idxNextStateInVec = i;//vertices[i].lineNum;
					return idxNextStateInVec;
				}

			}
			
		}
		else // if cond is false, find the corresponding right curly bracket as the next DUMMY state
		{
			for (i = 0;i < vertices.size();i++)
			{
				if (vertices[i]->lineNum == tmpVertex->endLineNum)
				{
					idxNextStateInVec = i;//vertices[i].lineNum;
					return idxNextStateInVec;
				}

			}
		}
	}

	if ((tmpVertex->ctrlStmtFlag)&&(tmpVertex->ctrlType == WHILE))
	{
		if (cond)// if cond is true, find the statement of next line as next state
		{
			for (i = 0;i < vertices.size();i++)
			{
				if (vertices[i]->lineNum > tmpVertex->lineNum)
				{
					idxNextStateInVec = i;//vertices[i].lineNum;
					return idxNextStateInVec;
				}

			}

		}
		else // if cond is false, find the next line after corresponding right curly bracket as the next state
		{
			for (i = 0;i < vertices.size();i++)
			{
				if (vertices[i]->lineNum > tmpVertex->endLineNum)
				{
					idxNextStateInVec = i;//vertices[i].lineNum;
					return idxNextStateInVec;
				}

			}
		}
	}
	if ((tmpVertex->ctrlStmtFlag)&&(tmpVertex->ctrlType == CURLYBRKT_R))
	{
		for (i = 0;i < vertices.size();i++)
		{
			if (vertices[i]->endLineNum == tmpVertex->lineNum)
			{
				if (vertices[i]->ctrlType == IFTHEN)
				{
					for (i++;i<vertices.size();i++)
					{
						if (vertices[i]->lineNum > tmpVertex->lineNum)
						{
							idxNextStateInVec = i;
							return idxNextStateInVec;
						}
					}

				}
				if (vertices[i]->ctrlType == WHILE)
				{
					idxNextStateInVec = i;//vertices[i].lineNum;
					return idxNextStateInVec;
				}
				
			}

		}
	}

	if ((tmpVertex->ctrlStmtFlag)&&(tmpVertex->ctrlType == DOWHILE))
	{
		if (cond)// if cond is true, find the corresponding DO statement as next state
		{
			for (i = 0;i < vertices.size();i++)
			{
				if (vertices[i]->endLineNum == tmpVertex->lineNum)// 
				{
					idxNextStateInVec = i;//vertices[i].lineNum;
					return idxNextStateInVec;
				}

			}

		}
		else // if cond is false, find the next expression as next state
		{
			for (i = 0;i < vertices.size();i++)
			{
				if (vertices[i]->lineNum > tmpVertex->lineNum)
				{
					idxNextStateInVec = i;//vertices[i].lineNum;
					return idxNextStateInVec;
				}

			}
		}
	}

	return -1;// means this expression is last one, no more next state.

}
void Schedule::write2VerilogFile_NS()
{
	
	outFile.open(this->verilogfile);

	if(!outFile){
		cerr << "Error when open file" << this->verilogfile << ".v. Program terminated!" << endl ;
		exit(1);
	}
	outFile << "`timescale 1ns / 1ps\n\n\n\n";
	outFile << "module HLSM (Clk, Rst, Start, Done,";
	writeDecl();
	writePrmt();
	outFile << "\talways @(posedge Clk) begin\n";
	outFile << "\t\tif (Rst == 1 ) begin\n";
    outFile << "\t\t\tState <= Wait;\n";
    string intledOutReg = intlOutReg(); 
	outFile << intledOutReg;
	outFile << "\t\tend\n\t\telse begin\n\t\t\tcase (State) \n";
	writeStates_NS();
	outFile << "\t\t\tendcase\n\t\tend\n\tend\nendmodule\n";
	outFile.close();
}

void Schedule::writeDecl()
{
	int i;

	strInputs = "";
    strOutputs ="";
	strRegs = "";
	string tmpStr = "";
    for (i = 0;i < nodes.size();i++)
    {

		if (nodes[i]->nodeType == INPUTS)
		{
			strInputs += nodes[i]->name;
			strInputs += ", ";
		}
		if (nodes[i]->nodeType == OUTPUTS)
		{
			strOutputs += nodes[i]->name;
			strOutputs += ", ";
		}
		if (nodes[i]->nodeType == REGS)
		{
			strRegs += nodes[i]->name;
			strRegs += ", ";
		}
    }
    tmpStr += strInputs;
	tmpStr += strOutputs;
	tmpStr.erase(tmpStr.end()-2);
	tmpStr += ");\n\n";
	outFile << tmpStr;
	outFile << "\tinput Clk, Rst, Start;\n";
	tmpStr = "";// write inputs
	tmpStr += "\tinput [31:0] ";
	tmpStr += strInputs;
	tmpStr.erase(tmpStr.end()-2);
    tmpStr += ";\n";
	outFile << tmpStr;
	tmpStr = "";// write outputs
	outFile << "\toutput reg Done;\n";
	tmpStr += "\toutput reg [31:0] ";
	tmpStr += strOutputs;
	tmpStr.erase(tmpStr.end()-2);
	tmpStr += ";\n";
	outFile << tmpStr;
	tmpStr = "";// write regs
	tmpStr += "\treg [31:0] ";
	tmpStr += strRegs;
	tmpStr.erase(tmpStr.end()-2);
	tmpStr += ";\n";
	outFile << tmpStr;
	tmpStr = "";
	outFile << "\treg [31:0] State;\n\n";
}
void Schedule::writePrmt()
{
	int stateNum, i;
	string tmpStr = "";
	outFile << "\tparameter Wait = 0;\n\tparameter Final = 1;\n";
	for (i = 0;i < states.size();i++)
	{
		states[i]->ID = i;
		tmpStr += "\tparameter State";
		tmpStr += convertInt2Str(i);
		tmpStr += " = ";
		tmpStr += convertInt2Str(i+2);
		tmpStr += " ;\n";
		outFile << tmpStr;
		tmpStr = "";

	}
    outFile << "\n";
}
string Schedule::convertInt2Str(int number)
{
	stringstream ss;//create a stringstream
	ss << number;//add number to the stream
	return ss.str();//return a string with the contents of the stream
}

string Schedule::intlOutReg()
{
	int i;
	string tmpStr = "";
	for (i = 0;i < nodes.size(); i++)
	{
		if ((nodes[i]->nodeType == OUTPUTS)||(nodes[i]->nodeType == REGS))
		{
			tmpStr += "\t\t\t";
			tmpStr += nodes[i]->name;
			tmpStr += " = 0;\n";
		}
	}
	return tmpStr;
}

void Schedule::writeStates_NS()
{
	int i;
	string tmpStr = ""; 
	outFile << "\t\t\t\tWait:begin\n";
	outFile << "\t\t\t\t\tif (Start == 0) begin\n";
	outFile << "\t\t\t\t\t\tState <= Wait;\n";
	outFile << "\t\t\t\t\tend\n\t\t\t\t\telse begin\n";
	if (states.empty())
	{
		outFile << "\t\t\t\t\t\tState <= Final";
        outFile << "\t\t\t\t\tend\n";
		outFile << "\t\t\t\tend\n";
	}
	else
	{
		outFile << "\t\t\t\t\t\tState <= State0;\n";
		outFile << "\t\t\t\t\tend\n";
		outFile << "\t\t\t\tend\n";
		for (i = 0; i < states.size();i++)
		{
			// when come to the last state before Final
			if ((i == states.size()-1)||(((states[i]->stateType == WHILE)||(states[i]->stateType == DOWHILE))&&(states[i]->nextState_0.empty())))
			{

				if (((states[i]->stateType == NORMAL)&&(states[i]->nextState_1.empty()))||((states[i]->stateType == CURLYBRKT_R)&&(states[i]->nextState_1.empty())))// if the last state
				{
					tmpStr += "\t\t\t\tState";
					tmpStr += convertInt2Str(i);
					tmpStr += " : begin\n";
					if ((states[i]->stateType == NORMAL))
					{
						tmpStr += state2Expression(states[i]);
					}
					
				    tmpStr += "\t\t\t\t\tState <= Final;\n";
					tmpStr += "\t\t\t\tend\n";

				}
				if ((states[i]->stateType == CURLYBRKT_R)&&(!states[i]->nextState_1.empty()))
				{
					tmpStr += "\t\t\t\tState";
					tmpStr += convertInt2Str(i);
					tmpStr += " : begin\n";
					tmpStr += "\t\t\t\t\tState <= ";
                    tmpStr += findStateByVertex(states[i]->nextState_1[0]);
                    tmpStr += "\n\t\t\t\tend\n";

				}
				if (((states[i]->stateType == WHILE)||(states[i]->stateType == DOWHILE))&&(states[i]->nextState_0.empty()))// the !cond case of WHILE is Final
				{
					tmpStr += "\t\t\t\tState";
					tmpStr += convertInt2Str(i);
					tmpStr += " : begin\n";
					tmpStr += "\t\t\t\t\tif (";
					tmpStr += states[i]->stateVec[0]->condNode->name;
					tmpStr += " == 0) begin\n";// then cond is false
					tmpStr += "\t\t\t\t\t\tState <= Final";
					tmpStr += " ;\n\t\t\t\t\tend\n\t\t\t\t\telse begin\n\t\t\t\t\t\t";// if cond is true
					tmpStr += "State <= ";
					tmpStr += findStateByVertex(states[i]->nextState_1[0]);
					tmpStr += " ;\n\t\t\t\t\tend\n";
					tmpStr += "\t\t\t\tend\n";
				}
			}
			else{
				if ((states[i]->stateType == NORMAL)||(states[i]->stateType == DO)||((states[i]->stateType == CURLYBRKT_R)))// not last statement
				{

					tmpStr += "\t\t\t\tState";
					tmpStr += convertInt2Str(i);
					tmpStr += " : begin\n";

					if ((states[i]->stateType == NORMAL))
					{
						tmpStr += state2Expression(states[i]);
					}
					tmpStr += nextStateExpression(states[i]);//(states[i]->stateType == DO)||(states[i]->stateType == CURLYBRKT_R)
					tmpStr += "\t\t\t\tend\n";
				}
				else
				{
					tmpStr += "\t\t\t\tState";
					tmpStr += convertInt2Str(i);
					tmpStr += " : begin\n";
					tmpStr += "\t\t\t\t\tif (";
					tmpStr += states[i]->stateVec[0]->condNode->name;
					tmpStr += " == 0) begin\n";// then cond is false
					tmpStr += "\t\t\t\t\t\tState <= ";
					tmpStr += findStateByVertex(states[i]->nextState_0[0]);
					tmpStr += " ;\n\t\t\t\t\tend\n\t\t\t\t\telse begin\n\t\t\t\t\t\t";// if cond is true
					tmpStr += "State <= ";
					tmpStr += findStateByVertex(states[i]->nextState_1[0]);
					tmpStr += " ;\n\t\t\t\t\tend\n";
					tmpStr += "\t\t\t\tend\n";
				}
			}
		}
	}
	tmpStr += "\t\t\t\tFinal : begin\n\t\t\t\t\tDone <= 1;\n\t\t\t\t\tState <= Wait;\n\t\t\t\tend\n";
	outFile << tmpStr;


}

int Schedule::checkCurlyBktRType(Vertex *startVertex)
{
	for (int i = 0; i < vertices.size(); i++)
	{
		if (startVertex->lineNum == vertices[i]->endLineNum)
		{
			return vertices[i]->vertexType;
		}
	}

}

string Schedule::state2Expression(State *curtState)
{
	string exprsStr = "";
	int i,j;
	vector<Node *> adjNodes;
		for (i = 0; i < curtState->stateVec.size();i++)
		{

			adjNodes = findAdjNodes(curtState->stateVec[i]);
			if (((curtState->stateVec[i]->vertexType == OUTPUTS)&&(curtState->stateVec[i]->addType4Out == -1))||(curtState->stateVec[i]->vertexType == REG))
			{
				j = 0;
				exprsStr += "\t\t\t\t\t";
				exprsStr += adjNodes[0]->name;
				exprsStr += " <= ";
				exprsStr += adjNodes[1]->name;
				exprsStr += " ;\n";
			}
			else{
				if (curtState->stateVec[i]->vertexType == MUX2X1)
				{
					if (adjNodes.size() != 4)
					{
						cerr << "Syntax error is detected at line " << curtState->stateVec[i]->lineNum <<": NOT enough parameters!"<< endl;
						system("Pause");exit(1);
					}
					exprsStr += "\t\t\t\t\t";
					exprsStr += adjNodes[0]->name;
					exprsStr += " ";
					exprsStr += " <= ";
					exprsStr += adjNodes[1]->name;
					exprsStr += " ? ";
					exprsStr += adjNodes[2]->name;
					exprsStr += " : ";
					exprsStr += adjNodes[3]->name;
					exprsStr += " ;\n";
				}
				else
				{

					exprsStr += "\t\t\t\t\t";
					exprsStr += adjNodes[0]->name;
					exprsStr += " <= ";
					exprsStr += adjNodes[1]->name;
					exprsStr += " ";
					if ((curtState->stateVec[i]->vertexType == OUTPUTS)&&(curtState->stateVec[i]->addType4Out != -1))
					{
						exprsStr += findSignByType(curtState->stateVec[i]->addType4Out);
					}
					else
					{
						exprsStr += findSignByType(curtState->stateVec[i]->vertexType);
					}
					exprsStr += " ";
					exprsStr += adjNodes[2]->name;
					exprsStr += " ;\n";

				}
			}

		}
		return exprsStr;
}

string Schedule::nextStateExpression(State *curtState)
{
	string exprsStr = "";
	int i,j;
	vector<Node *> adjNodes;

	if ((curtState->stateType == NORMAL)||(curtState->stateType == DO)||(curtState->stateType == CURLYBRKT_R))
	{
		exprsStr += "\t\t\t\t\t";
		exprsStr += "State <= ";
		exprsStr += findStateByVertex(curtState->nextState_1[0]);
		exprsStr += " ;\n";
	}
	if ((curtState->stateType == IFTHEN)||(curtState->stateType == DOWHILE)||(curtState->stateType == WHILE))
	{
		exprsStr += "\t\t\t\t\tif (";
		exprsStr += curtState->stateVec[0]->condNode->name;
		exprsStr += " == 0) begin\n";// then cond is false
		exprsStr += "\t\t\t\t\t\tState <= ";
		exprsStr += findStateByVertex(curtState->nextState_0[0]);
		exprsStr += " ;\n\t\t\t\t\tend\n\t\t\t\t\telse begin\n\t\t\t\t\t\t";// if cond is true
		exprsStr += "State <= ";
		exprsStr += findStateByVertex(curtState->nextState_1[0]);
		exprsStr += " ;\n\t\t\t\t\tend\n";
	}
	return exprsStr;
}

string Schedule::findStateByVertex(Vertex *startVertex)
{
	int i;

	string tmpStr = "";
	for (i = 0; i < states.size(); i ++)
	{
		if (states[i]->lineNum == startVertex->lineNum)
		{

			tmpStr += "State";
			tmpStr += convertInt2Str(i);
			return tmpStr;

		}
	}
}

vector<Node *> Schedule::findAdjNodes(Vertex *startVertex)
{
	vector<Node *> adjNodes;
	Node *tmpNode = new Node;
	for (int j = 0;j < edges.size();j ++)
	{	
		if (edges[j]->vertex->lineNum == startVertex->lineNum)
		{
			tmpNode= edges[j]->node;
			adjNodes.push_back(tmpNode);

		}
	}
	return adjNodes;
}
bool Schedule::checkConnected(Node *tmpNode, Vertex *startVertex)
{
	bool connectedFlag = false;
	int i,j;
	vector<Edge *> adjEdges;
	adjEdges = findAdjEdges(startVertex);

	for (i =0 ;i< adjEdges.size();i++)
	{

			if ((adjEdges[i]->node->name == tmpNode->name))
			{
				connectedFlag = true;
				return connectedFlag;
			}

	}
	return connectedFlag;
}

vector<Edge *> Schedule::findAdjEdges(Vertex *startVertex){

	vector<Edge *> adjEdges;
	Edge *tmpEdge = new Edge;
	for (int i =0 ;i< edges.size();i++)
	{
		if (edges[i]->vertex->lineNum == startVertex->lineNum)
		{
			tmpEdge = edges[i];
			adjEdges.push_back(tmpEdge);
		}
	}
	return adjEdges;
}

string Schedule::findSignByType(int opType){
	if (opType == ADD32)
	{
		return "+";
	}
	if (opType == SUB32)
	{
		return "-";
	}
	if (opType == MUL32)
	{
		return "*";
	}
	if( opType == COMPEQ)
	{
		return "==";
	}
	if (opType == COMPLT)
	{
		return "<";
	}
	if (opType == SHL)
	{
		return "<<";
	}
	if (opType == COMPGT)
	{
		return ">";
	}
	if (opType == SHR)
	{
		return ">>";
	}
	if (opType == MUX2X1)
	{
		return ":";
	}
}

void Schedule::gnrtStates_LS()
{
	int i, j, k, timeStep = 1;
	vector<int> startVertices;
    vector<int> candVertices;
	vector<int> unfnVertices;
	vector<int> outputsVertices;
	
    startVertices = findStartVerticesIdx();
	intlVerCycle();
	outputsVertices = findOutputsVerIdx();
	do// LIST_L
	{
		if (timeStep == 1)
		{
			candVertices = startVertices;
		}
		else
		{
			candVertices = findCandVerticesIdx();
		}
		unfnVertices = findUNFNVerticesIdx();
		drmnAndScdlVertces_LS(candVertices,unfnVertices,timeStep);
		timeStep ++;

	}while (!allScdled(outputsVertices));

	for (i = 1;i < timeStep; i++)
	{
		State *tmpState = new State;
		for (j = 0;j < vertices.size();j++)
		{
			if (vertices[j]->scheduledTime == i )
			{
				tmpState->stateVec.push_back(vertices[j]);
				tmpState->timeStep = i;
				
			}
		}
		states.push_back(tmpState);
	}
}
void Schedule::write2VerilogFile_LSR()
{

	outFile.open(this->verilogfile);

	if(!outFile){
		cerr << "Error when open file" << this->verilogfile << ".v. Program terminated!" << endl ;
		exit(1);
	}
	outFile << "`timescale 1ns / 1ps\n\n\n\n";
	outFile << "module HLSM (Clk, Rst, Start, Done,";
	writeDecl();
	writePrmt();
	outFile << "\talways @(posedge Clk) begin\n";
	outFile << "\t\tif (Rst == 1 ) begin\n";
	outFile << "\t\t\tState <= Wait;\n";
	string intledOutReg = intlOutReg(); 
	outFile << intledOutReg;
	outFile << "\t\tend\n\t\telse begin\n\t\t\tcase (State) \n";
	writeStates_LSR();
	outFile << "\t\t\tendcase\n\t\tend\n\tend\nendmodule\n";
	outFile.close();
}

void Schedule::writeStates_LSR()
{
	int i;
	string tmpStr = ""; 
	outFile << "\t\t\t\tWait:begin\n";
	outFile << "\t\t\t\t\tif (Start == 0) begin\n";
	outFile << "\t\t\t\t\t\tState <= Wait;\n";
	outFile << "\t\t\t\t\tend\n\t\t\t\t\telse begin\n";
	if (states.empty())
	{
		outFile << "\t\t\t\t\t\tState <= Final";
		outFile << "\t\t\t\t\tend\n";
		outFile << "\t\t\t\tend\n";
	}
	else
	{
		outFile << "\t\t\t\t\t\tState <= State0;\n";
		outFile << "\t\t\t\t\tend\n";
		outFile << "\t\t\t\tend\n";
		for (i = 0; i < states.size();i++)
		{
			// when come to the last state before Final
			if ((i == states.size()-1))
			{				
					tmpStr += "\t\t\t\tState";
					tmpStr += convertInt2Str(i);
					tmpStr += " : begin\n";
					tmpStr += state2Expression(states[i]);
					tmpStr += "\t\t\t\t\tState <= Final;\n";
					tmpStr += "\t\t\t\tend\n";
			}
			else{
				tmpStr += "\t\t\t\tState";
				tmpStr += convertInt2Str(i);
				tmpStr += " : begin\n";
				tmpStr += state2Expression(states[i]);
				tmpStr += nextStateExpression(states[i]);//(states[i]->stateType == DO)||(states[i]->stateType == CURLYBRKT_R)
				tmpStr += "\t\t\t\tend\n";
			}
		}
	}
	tmpStr += "\t\t\t\tFinal : begin\n\t\t\t\t\t State <= Wait;\n\t\t\t\tend\n";
	outFile << tmpStr;


}


void Schedule::drmnAndScdlVertces_LS(vector<int> candVertices, vector<int> unfnVertices, int timeStep)
{
	int h, i,j,k;
	int mulCand = 0,add_subCand = 0, logicOpCand = 0;
	int mulUnfn = 0,add_subUnfn = 0, logicOpUnfn = 0;
    for (h = 0; h < candVertices.size(); h ++)
    {
		if (vertices[candVertices[h]]->vertexType == MUL32)
		{
			mulCand++;
		}
		else
		{
			if ((vertices[candVertices[h]]->vertexType == ADD32)||(vertices[candVertices[h]]->vertexType == SUB32))
			{
				add_subCand++;
			}
			else
				logicOpCand ++;
		}
    }

	for (h = 0; h < unfnVertices.size(); h ++)
	{
		if (vertices[unfnVertices[h]]->vertexType == MUL32)
		{
			mulUnfn++;
		}
		else
		{
			if ((vertices[unfnVertices[h]]->vertexType == ADD32)||(vertices[unfnVertices[h]]->vertexType == SUB32))
			{
				add_subUnfn++;
			}
			else
				logicOpUnfn ++;
		}
	}
    // for MUL32
	i = 0;
	if ((mul-mulUnfn) > 0)
	{
		
		if ((mul-mulUnfn) >= mulCand)
		{
			
			for (h = 0;(h < candVertices.size())&&(i < mulCand);h++)
			{
				if (vertices[candVertices[h]]->vertexType == MUL32)
				{
					i++;
					vertices[candVertices[h]]->scheduled = true;
					vertices[candVertices[h]]->scheduledTime = timeStep;
					vertices[candVertices[h]]->cycle = vertices[candVertices[h]]->cycle - 1;
				}
			}
		}
		else
		{

			for (h = 0;(h < candVertices.size())&&(i < (mul-mulUnfn));h++)
			{
				if (vertices[candVertices[h]]->vertexType == MUL32)
				{
					i++;
					vertices[candVertices[h]]->scheduled = true;
					vertices[candVertices[h]]->scheduledTime = timeStep;
					vertices[candVertices[h]]->cycle = vertices[candVertices[h]]->cycle - 1;
				}
			}
		}
	}
	i = 0;
	for (h = 0;(h < unfnVertices.size())&&(i < mulUnfn);h++)
	{
		if (vertices[unfnVertices[h]]->vertexType == MUL32)
		{
			i++;
			vertices[unfnVertices[h]]->cycle = vertices[unfnVertices[h]]->cycle - 1;
		}
	}
	// for ADD32 and SUB32
	i = 0;
	if ((add_sub-add_subUnfn) > 0)
	{

		if ((add_sub-add_subUnfn) >= add_subCand)
		{

			for (h = 0;(h < candVertices.size())&&(i < add_subCand);h++)
			{
				if ((vertices[candVertices[h]]->vertexType == ADD32)||(vertices[candVertices[h]]->vertexType == SUB32))
				{
					i++;
					vertices[candVertices[h]]->scheduled = true;
					vertices[candVertices[h]]->scheduledTime = timeStep;
					vertices[candVertices[h]]->cycle = vertices[candVertices[h]]->cycle - 1;
				}
			}
		}
		else
		{

			for (h = 0;(h < candVertices.size())&&(i < (add_sub-add_subUnfn));h++)
			{
				if ((vertices[candVertices[h]]->vertexType == ADD32)||(vertices[candVertices[h]]->vertexType == SUB32))
				{
					i++;
					vertices[candVertices[h]]->scheduled = true;
					vertices[candVertices[h]]->scheduledTime = timeStep;
					vertices[candVertices[h]]->cycle = vertices[candVertices[h]]->cycle - 1;
				}
			}
		}
	}
	i = 0;
	for (h = 0;(h < unfnVertices.size())&&(i < add_subUnfn);h++)
	{
		if ((vertices[unfnVertices[h]]->vertexType == ADD32)||(vertices[unfnVertices[h]]->vertexType == SUB32))
		{
			i++;
			vertices[unfnVertices[h]]->cycle = vertices[unfnVertices[h]]->cycle - 1;
		}
	}

	// for others
	i = 0;
	if ((logicOp-logicOpUnfn) > 0)
	{

		if ((logicOp-logicOpUnfn)  >= logicOpCand)
		{

			for (h = 0;(h < candVertices.size())&&(i < logicOpCand);h++)
			{
				if ((vertices[candVertices[h]]->vertexType == REG)||(vertices[candVertices[h]]->vertexType == COMPGT)||(vertices[candVertices[h]]->vertexType == COMPLT)||(vertices[candVertices[h]]->vertexType == SHL)||(vertices[candVertices[h]]->vertexType == SHR)||(vertices[candVertices[h]]->vertexType == COMPEQ)||(vertices[candVertices[h]]->vertexType == MUX2X1)||(vertices[candVertices[h]]->vertexType == OUTPUTS))
				{
					i++;
					vertices[candVertices[h]]->scheduled = true;
					vertices[candVertices[h]]->scheduledTime = timeStep;
					vertices[candVertices[h]]->cycle = vertices[candVertices[h]]->cycle - 1;
				}
			}
		}
		else
		{

			for (h = 0;(h < candVertices.size())&&(i < (logicOp-logicOpUnfn));h++)
			{
				if ((vertices[candVertices[h]]->vertexType == REG)||(vertices[candVertices[h]]->vertexType == COMPGT)||(vertices[candVertices[h]]->vertexType == COMPLT)||(vertices[candVertices[h]]->vertexType == SHL)||(vertices[candVertices[h]]->vertexType == SHR)||(vertices[candVertices[h]]->vertexType == COMPEQ)||(vertices[candVertices[h]]->vertexType == MUX2X1)||(vertices[candVertices[h]]->vertexType == OUTPUTS))
				{
					i++;
					vertices[candVertices[h]]->scheduled = true;
					vertices[candVertices[h]]->scheduledTime = timeStep;
					vertices[candVertices[h]]->cycle = vertices[candVertices[h]]->cycle - 1;
				}
			}
		}
	}
	i = 0;
	for (h = 0;(h < unfnVertices.size())&&(i < logicOpUnfn);h++)
	{
		if ((vertices[unfnVertices[h]]->vertexType == REG)||(vertices[unfnVertices[h]]->vertexType == COMPGT)||(vertices[unfnVertices[h]]->vertexType == COMPLT)||(vertices[unfnVertices[h]]->vertexType == SHL)||(vertices[unfnVertices[h]]->vertexType == SHR)||(vertices[unfnVertices[h]]->vertexType == COMPEQ)||(vertices[unfnVertices[h]]->vertexType == MUX2X1)||(vertices[unfnVertices[h]]->vertexType == OUTPUTS))
		{
			i++;
			vertices[unfnVertices[h]]->cycle = vertices[unfnVertices[h]]->cycle - 1;
		}
	}

    	

}
vector<int>  Schedule::findCandVerticesIdx()
{

	int i;
	vector<int> candVerticesIdx;
    vector<int> tmpVerticesIdx;
	for (i = 0; i < vertices.size();i++ )
	{
		if (!vertices[i]->scheduled)
		{
			tmpVerticesIdx = findPreAdjVerticesIdx(vertices[i]);
			if (allScdled(tmpVerticesIdx))
			{
				candVerticesIdx.push_back(i);
			}

		}
	}
    return candVerticesIdx;
}
vector<int>  Schedule::findUNFNVerticesIdx()
{

	int i;
	vector<int > unfnVertices;
	for (i = 0; i < vertices.size();i++ )
	{
		if (vertices[i]->scheduled)
		{

			if ((vertices[i]->cycle))
			{
				unfnVertices.push_back(i);
			}

		}
	}
	return unfnVertices;
}
void  Schedule::intlVerCycle()
{
	int i;


	for (i = 0;i < vertices.size();i++)
	{
		if (vertices[i]->vertexType == MUL32)
		{
			vertices[i]->cycle = 2;
		}
		else
			vertices[i]->cycle = 1;
	}

}
vector<int>  Schedule::findStartVerticesIdx()
{
	int i,j;
	int n2vCount = 0;
	int adjInputCount = 0;
	vector<int> startVerticesVector;

	for (i = 0;i < vertices.size();i++)
	{
		int n2vCount = 0;
		int adjInputCount = 0;
		vector<Edge *> adjEdges = findAdjEdges(vertices[i]);
		for (j = 0; j< adjEdges.size();j++)
		{
			if((adjEdges[j]->v2n == false))
			{
				n2vCount ++;
				if ((adjEdges[j]->node->nodeType == INPUTS))
				{
					adjInputCount++;
				}

			}
		}
		if (n2vCount == adjInputCount)// if every node which goes to vertex is input one  
		{
			startVerticesVector.push_back(i);
		}
	}
	return startVerticesVector;

}
vector<int> Schedule::findOutputsVerIdx()
{
	int i,j;
	vector<int> outputsVertices;
	for (i = 0;i < vertices.size(); i++)
	{
			if (vertices[i]->vertexType== OUTPUTS)
			{
				outputsVertices.push_back(i);
			}
	}
	return outputsVertices;
}

bool Schedule::allScdled(vector<int> outputsVerticesIdx)
{
	int i;
	for (i = 0;i < outputsVerticesIdx.size();i++)
	{
		if (!vertices[outputsVerticesIdx[i]]->scheduled)
		{
			return false;
		}
	}
    return true;
}
vector<int> Schedule::findDescAdjVerticesIdx(Vertex *startVertex)// find the descendant 
{
	vector<int> adjVertices;
	Vertex *tmpVertex = new Vertex;
	for (int i = 0;i<vertices.size();i++)
	{
		tmpVertex = vertices[i];
		if (tmpVertex->lineNum != startVertex->lineNum)
		{
			if (checkVertexConnected(startVertex,tmpVertex))
			{

				adjVertices.push_back(i);
			}
		}

	}
	return adjVertices;
}
vector<int > Schedule::findPreAdjVerticesIdx(Vertex *startVertex)// find the predessesors
{
	vector<int> adjVertices;
	Vertex *tmpVertex = new Vertex;
	for (int i = 0;i<vertices.size();i++)
	{
		tmpVertex = vertices[i];
		if (tmpVertex->lineNum != startVertex->lineNum)
		{
			if (checkVertexConnected(tmpVertex,startVertex))
			{
				adjVertices.push_back(i);
			}
		}

	}
	return adjVertices;
}
bool Schedule::checkVertexConnected(Vertex *v1, Vertex *v2){// the connection direction is downward, i.e. whether v1 to v2
	bool connectedFlag = false;
	int i,j;
	vector<Edge *> adjEdges1;
	adjEdges1 = findAdjEdges(v1);
	vector<Edge *> adjEdges2;
	adjEdges2 = findAdjEdges(v2);
	for (i =0 ;i< adjEdges1.size();i++)
	{
		for (j = 0;j<adjEdges2.size();j++)
		{
			if ((adjEdges1[i]->node->name == adjEdges2[j]->node->name)&&(adjEdges1[i]->v2n)&&(! adjEdges2[j]->v2n))
			{
				connectedFlag = true;
				return connectedFlag;
			}
		}
	}
	return connectedFlag;
}


void Schedule::gnrtStates_LR()
{
	int i, j, k, timeStep = 1;
	add_sub = 1;
	mul = 1;
	logicOp = 1;
	vector<int> startVertices;
	vector<int> candVertices;
	vector<int> unfnVertices;
	vector<int> outputsVertices;

	startVertices = findStartVerticesIdx();
	intlVerCycle();
	outputsVertices = findOutputsVerIdx();
	ALAP(outputsVertices,latency);
	do// LIST_R
	{
		if (timeStep == 1)
		{
			candVertices = startVertices;
		}
		else
		{
			candVertices = findCandVerticesIdx();
		}
		unfnVertices = findUNFNVerticesIdx();
		drmnAndScdlVertces_LR(candVertices,unfnVertices,timeStep);
		timeStep ++;

	}while (!allScdled(outputsVertices));

	for (i = 1;i < timeStep; i++)
	{
		State *tmpState = new State;
		for (j = 0;j < vertices.size();j++)
		{
			if (vertices[j]->scheduledTime == i )
			{
				tmpState->stateVec.push_back(vertices[j]);
				tmpState->timeStep = i;
				
			}
		}
		states.push_back(tmpState);
	}
}

void Schedule::drmnAndScdlVertces_LR(vector<int> candVertices, vector<int> unfnVertices, int timeStep)
{
	int h, i,j,k;
	int mulCand = 0,add_subCand = 0, logicOpCand = 0;
	int mulUnfn = 0,add_subUnfn = 0, logicOpUnfn = 0;

	vector<int> candMULVertices;vector<int> candADDVertices;vector<int> candOtherVertices;
    vector<int> unfnMULVertices;vector<int> unfnADDVertices;vector<int> unfnOtherVertices;
	for (h = 0; h < candVertices.size(); h ++)
	{
		if (vertices[candVertices[h]]->vertexType == MUL32)
		{
			mulCand++;
			candMULVertices.push_back(candVertices[h]);
		}
		else
		{
			if ((vertices[candVertices[h]]->vertexType == ADD32)||(vertices[candVertices[h]]->vertexType == SUB32))
			{
				add_subCand++;
				candADDVertices.push_back(candVertices[h]);
			}
			else
			{
				logicOpCand ++;
				candOtherVertices.push_back(candVertices[h]);
			}
		}
	}

	for (h = 0; h < unfnVertices.size(); h ++)
	{
		if (vertices[unfnVertices[h]]->vertexType == MUL32)
		{
			mulUnfn++;
			unfnMULVertices.push_back(candVertices[h]);
		}
		else
		{
			if ((vertices[unfnVertices[h]]->vertexType == ADD32)||(vertices[unfnVertices[h]]->vertexType == SUB32))
			{
				add_subUnfn++;
				unfnADDVertices.push_back(candVertices[h]);
			}
			else
			{
				logicOpUnfn ++;
				unfnOtherVertices.push_back(candVertices[h]);
			}
		}
	}
	// calcuate the slack

	for (h = 0;h < vertices.size();h++)
	{
		vertices[h]->slackTime = vertices[h]->alapTime - timeStep;
	}



	// for MUL32
	i = 0;
	for (h = 0;(h < candVertices.size())&&(i < mulCand);h++)
	{
		if (vertices[candVertices[h]]->vertexType == MUL32)
		{
			i++;
			if ((vertices[candVertices[h]]->slackTime == 0))
			{
				mul ++;			
			}

		}
	}
	i = 0;
	if ((mul-mulUnfn) > 0)
	{

		if ((mul-mulUnfn) >= mulCand)
		{

			for (h = 0;(h < candVertices.size())&&(i < mulCand);h++)
			{
				if (vertices[candVertices[h]]->vertexType == MUL32)
				{
					i++;
					vertices[candVertices[h]]->scheduled = true;
					vertices[candVertices[h]]->scheduledTime = timeStep;
					vertices[candVertices[h]]->cycle = vertices[candVertices[h]]->cycle - 1;
				}
			}
		}
		else
		{

	
					vector<int> smallSlackVec = pickSmallestSlack(candMULVertices,(mul-mulUnfn));
					vertices[smallSlackVec[h]]->scheduled = true;
					vertices[smallSlackVec[h]]->scheduledTime = timeStep;
					vertices[smallSlackVec[h]]->cycle = vertices[smallSlackVec[h]]->cycle - 1;

	
		}
	}
	i = 0;
	for (h = 0;(h < unfnVertices.size())&&(i < mulUnfn);h++)
	{
		if (vertices[unfnVertices[h]]->vertexType == MUL32)
		{
			i++;
			vertices[unfnVertices[h]]->cycle = vertices[unfnVertices[h]]->cycle - 1;
		}
	}
	// for ADD32 and SUB32
	i = 0;
	if ((add_sub-add_subUnfn) > 0)
	{

		if ((add_sub-add_subUnfn) >= add_subCand)
		{

			for (h = 0;(h < candVertices.size())&&(i < add_subCand);h++)
			{
				if ((vertices[candVertices[h]]->vertexType == ADD32)||(vertices[candVertices[h]]->vertexType == SUB32))
				{
					i++;
					vertices[candVertices[h]]->scheduled = true;
					vertices[candVertices[h]]->scheduledTime = timeStep;
					vertices[candVertices[h]]->cycle = vertices[candVertices[h]]->cycle - 1;
				}
			}
		}
		else
		{

				    vector<int> smallSlackVec = pickSmallestSlack(candADDVertices,(add_sub-add_subUnfn));
					vertices[smallSlackVec[h]]->scheduled = true;
					vertices[smallSlackVec[h]]->scheduledTime = timeStep;
					vertices[smallSlackVec[h]]->cycle = vertices[smallSlackVec[h]]->cycle - 1;

		}
	}
	i = 0;
	for (h = 0;(h < unfnVertices.size())&&(i < add_subUnfn);h++)
	{
		if ((vertices[unfnVertices[h]]->vertexType == ADD32)||(vertices[unfnVertices[h]]->vertexType == SUB32))
		{
			i++;
			vertices[unfnVertices[h]]->cycle = vertices[unfnVertices[h]]->cycle - 1;
		}
	}

	// for others
	i = 0;
	if ((logicOp-logicOpUnfn) > 0)
	{

		if ((logicOp-logicOpUnfn)  >= logicOpCand)
		{

			for (h = 0;(h < candVertices.size())&&(i < logicOpCand);h++)
			{
				if ((vertices[candVertices[h]]->vertexType == REG)||(vertices[candVertices[h]]->vertexType == COMPGT)||(vertices[candVertices[h]]->vertexType == COMPLT)||(vertices[candVertices[h]]->vertexType == SHL)||(vertices[candVertices[h]]->vertexType == SHR)||(vertices[candVertices[h]]->vertexType == COMPEQ)||(vertices[candVertices[h]]->vertexType == MUX2X1)||(vertices[candVertices[h]]->vertexType == OUTPUTS))
				{
					i++;
					vertices[candVertices[h]]->scheduled = true;
					vertices[candVertices[h]]->scheduledTime = timeStep;
					vertices[candVertices[h]]->cycle = vertices[candVertices[h]]->cycle - 1;
				}
			}
		}
		else
		{

				    vector<int> smallSlackVec = pickSmallestSlack(candOtherVertices,(logicOp-logicOpUnfn));
					vertices[candVertices[h]]->scheduled = true;
					vertices[candVertices[h]]->scheduledTime = timeStep;
					vertices[candVertices[h]]->cycle = vertices[candVertices[h]]->cycle - 1;
		}
	}
	i = 0;
	for (h = 0;(h < unfnOtherVertices.size());h++)
	{
			vertices[unfnOtherVertices[h]]->cycle = vertices[unfnOtherVertices[h]]->cycle - 1;
	}



}
int Schedule::ALAP(vector<int> outputsVertices, int curLatency)
{

	if (curLatency < 1)
	{
			cerr << "No feasible solution with latency " << latency<<" ."<< endl;
			system("Pause");exit(1);
	}
    
    int i;
	vector<int> preVerricesIds;
	for (i = 0;i < outputsVertices.size();i++)
	{
		if (vertices[outputsVertices[i]]->vertexType == MUL32)
		{

			vertices[outputsVertices[i]]->alapTime = curLatency - 1;
		}

		else{
			vertices[outputsVertices[i]]->alapTime = curLatency;
		}
		vertices[outputsVertices[i]]->alapTime = curLatency;
		preVerricesIds = findPreAdjVerticesIdx(vertices[outputsVertices[i]]);


		if (preVerricesIds.empty())
		{
			continue;
		}
		else
		{
			if (vertices[outputsVertices[i]]->vertexType == MUL32)
			{

				ALAP(preVerricesIds, vertices[outputsVertices[i]]->alapTime - 2);
			}

			else{
				ALAP(preVerricesIds, vertices[outputsVertices[i]]->alapTime - 1);
			}
		}
		
	}
	return -1;

}

vector<int> Schedule:: pickSmallestSlack(vector<int>candTypeVec,int num)
{
	int h,i,smallerSlack,smallerSlackIdx;
	vector<int> pickedSmallestVec;
	for (i = 0; i < num; i++)
	{
		smallerSlackIdx = candTypeVec[0];
		smallerSlack = vertices[candTypeVec[0]]->slackTime;
		for (h = 1; h < candTypeVec.size();h++)
		{
			if (vertices[candTypeVec[h]]->slackTime < smallerSlack)
			{
				smallerSlack = vertices[candTypeVec[h]]->slackTime;
				smallerSlackIdx = candTypeVec[h];
			}
		}
		pickedSmallestVec.push_back(smallerSlackIdx);
		candTypeVec.erase(candTypeVec.begin()+smallerSlackIdx);
	}

    return pickedSmallestVec;
}