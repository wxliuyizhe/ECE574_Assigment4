//hlsyn.cpp : Defines the entry point for the console application.



//---------------------------------------------------------------------------------------------------------------------//
//
//ECE 574A - C++ program with support for checking commandling arguments required for Assignment 4
//
//Authors: Yizhe Liu, Ding Ding 
//Date: 11-21-2013
//
//	  ---------------------------------------------------------------------------------------------------------------------//
//#include "stdafx.h"
#include <iostream>
#include <string>
#include "schedule.h"
#include <stdio.h>
#include <stdlib.h>
	  using namespace std;

int main(int argc, char** argv)
{
	
	if(argc == 2)// hlsyn -help case
	{
		if(strcmp(argv[1], "-help") != 0){
			cerr << endl << "Usage: " << argv[0] << " -help" << endl << endl;
			system("Pause");return -1;
		}
		cout << "Usage: 3 options are provided below: " << endl;
		cout << "Generate a high level state machine with NO scheduling: -ns cfile verilogfile" << endl;
		cout << "Generate a high level state machine with List_L scheduling: -listl cfile verilogfile mul add logic" << endl;
		cout << "Generate a high level state machine with List_R scheduling: -listr cfile verilogfile latency" << endl;
		system("Pause");return 0;
	}
	else 
	{

        Schedule schdl;
		if(argc == 4)// hlsyn -ns case
		{

			if(strcmp(argv[1], "-ns") != 0){
				cerr << endl << "Usage: " << argv[0] << " -ns cfile verilogfile" << endl << endl;
				system("Pause");return -1;
			}
			cout << "Command Executed: " << argv[0] << " " << argv[1] << " " << argv[2] << " " << argv[3] << endl;
            schdl.setAttribues(argv[2],argv[3], NS);
			schdl.parseCFile();
			schdl.gnrtStates_NS();
			schdl.write2VerilogFile_NS();
		}
		else
		{
			if(argc == 7)// hlsyn -listl case
			{

				if(strcmp(argv[1], "-listl") != 0){
					cerr << endl << "Usage: " << argv[0] << " -listl cfile verilogfile mul add logic" << endl << endl;
					system("Pause");return -1;
				}
				cout << "Command Executed: " << argv[0] << " " << argv[1] << " " << argv[2] << " " << argv[3] << " " << argv[4] << " " << argv[5] << " " << argv[6]<< endl;
                schdl.setAttribues(argv[2],argv[3],argv[4],argv[5],argv[6], LISTL);
				schdl.parseCFile();
				schdl.gnrtStates_LS();
				schdl.write2VerilogFile_LSR();
			}
			else
			{
				if(argc == 5)// hlsyn -listr case
				{
					if(strcmp(argv[1], "-listr") != 0){
						cerr << endl << "Usage: " << argv[0] << " -ns cfile verilogfile latency" << endl << endl;
						system("Pause");return -1;
					}
					cout << "Command Executed: " << argv[0] << " " << argv[1] << " " << argv[2] << " " << argv[3] << " " << argv[4] << endl;
                    schdl.setAttribues(argv[2],argv[3],argv[4],LISTR);
					schdl.parseCFile();
					schdl.gnrtStates_LR();
					schdl.write2VerilogFile_LSR();
				}
				else
				{
					cerr << endl << "Usage: " << argv[0] << " -help" << endl << endl;
					system("Pause");return -1;				
				}
			}
		}// end of determining program option 
		
	}
	cout << "Verilog file has been created." << endl;
	system("Pause");
	return 0;
}

