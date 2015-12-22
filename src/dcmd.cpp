/*
 * =====================================================================================
 *
 *       Filename:  dcmd.cpp
 *
 *    Description: 分布式环境中执行shell命令 
 *
 *        Version:  1.0
 *        Created:  2015年09月17日 10时17分01秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  zj (), zj@163.com
 *        Company:  whu.sklse
 *
 * =====================================================================================
 */

#include <mpi.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include "MessagePasser.h"

using namespace std;

static const char* cmd[] = {"bye","help","quit"};
MessagePasser* mp = MessagePasser::getInstance();

void leader();
void server();
void command(string line);
void stripWhite(char* line);
bool dispatch(char* line);
int  findCmd(char* line);
bool executeShell(const char* command, vector<string>& values);
void printUsage();
string toString(vector<string>& vec);

int main(int argc, char** argv)
{
	mp->init(&argc, &argv);
	if(mp->isMaster())
	{
		leader();
	}else{
		server();
	}

	return 0;
}

void leader()
{
	char *line = NULL;
	bool live = true;
	while(live)
	{
		line = readline("\033[32mshell>\033[0m");
		if(line == NULL)
			break;
		if(*line == 0){
			free(line);
			continue;
		}
		add_history(line);
		live = dispatch(line);
		mp->clear();
	}
	free(line);
}

void server()
{
	bool live = true;
	while(live)
	{
		int count = 0;
		char* data = (char*)mp->receive(count);
		if((strcmp(data, "quit") == 0) || (strcmp(data, "bye") == 0))
		{
			live = false;
		}else{
			vector<string> result;
			executeShell(data, result);
			NodeNum master = mp->getMaster();
			mp->send(toString(result),master);
		}	
	}
}

bool dispatch(char* line)
{
	stripWhite(line);
	int index = findCmd(line);

	switch(index){
		case 0:
			mp->broadCast("bye");
			return false;
		case 1:
			printUsage();
			break;
		case 2:
			mp->broadCast("quit");
			return false;
		default:
			command(line);
			break;
	}

	return true;
}

void command(string line)
{
	if(line.find("(") != string::npos)
	{
		int begin = line.find("(");
		int end = line.find(")");
		string cmd = line.substr(0, begin - 1);
		string nstr = line.substr(begin + 1, end - begin -1);

		for(int i = 0; i < nstr.size(); ++i)
		{
			int node = nstr[i] - 48;
			if(node == 0)
			{
				vector<string> value;
				executeShell(cmd.c_str(), value);
				if(!value.empty())
					cout << "node0\n" << toString(value) << endl;
			}else{
				mp->execute(cmd, node);
			}
		}
	}else{
		vector<string> value;
		executeShell(line.c_str(), value);
		if(!value.empty())
			cout << "node0\n" << toString(value) << endl;
		mp->executeAll(line);
	}
}

bool executeShell(const char* command, vector<string>& values)
{
	values.clear();
	FILE* file = popen(command,"r");

	if(!file)
	{
		cerr << "\033[31mERROR:\033[0m Excute shell command " << command << " failed !!" << endl;
		return false;
	}

	char temp[1024];
	while( fgets(temp,sizeof(temp),file) != NULL)
	{
		if( temp[strlen(temp) - 1] == '\n')
			temp[strlen(temp) - 1] = '\0';

		values.push_back(temp);
	}

	pclose(file);

	return true;
}

void stripWhite(char* line)
{
	while(*line != 0 && (*line == ' ' || *line == '\t'))
		line++;

	int index = strlen(line) - 1;
	while(line[index] == ' ' || line[index] == '\t')
	{
		line[index] = '\0';
		index--;
	}
}

int findCmd(char* line)
{
	int len = sizeof(cmd) / sizeof(cmd[0]);
	for(int i = 0; i < len; ++i)
	{
		if(strcmp(line,cmd[i]) == 0)
			return i;
	}

	return -1;
}

void printUsage()
{
	cout << "Usage:\n"
		<<"\thelp,quit,bye\n"
		<<"\tshell (nodelist)" << endl;
	cout << "Example: rm -r ~/csm (123)" << endl;
}

string toString(vector<string>& vec)
{
	string result;
	vector<string>::iterator iter = vec.begin();
	for(; iter != vec.end(); ++iter)
	{
		result += *iter;
		result += "\n";
	}
	return result;
}
