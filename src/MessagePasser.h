/*
 * =====================================================================================
 *
 *       Filename:  MessagePasser.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2015年06月04日 23时22分23秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  zj, xiaojian_whu@163.com
 *        Company:  whu.sklse
 *
 * =====================================================================================
 */

#ifndef MESSAGEPASSER_INC
#define MESSAGEPASSER_INC

#include <mpi.h>
#include <cstdlib>
#include <time.h>
#include <vector>
#include <string>

using namespace std;

typedef unsigned int NodeNum;

#define Master 0
#define PoolSize 1 * 1024 * 1024
class MessagePasser
{
	public:
		static 	MessagePasser* getInstance();

		bool 	isMaster();

		void 	send(string data, NodeNum node);

		void	broadCast(string data);

		void*  	receive(int& count);

		void*	receive(int& count, NodeNum node);

		void	execute(string& cmd, NodeNum node);

		void	executeAll(string& cmd);

		void 	init(int* argc, char*** argv);

		void	clear();

		string 	getProName();

		int 	getRank();

		int	getSize();

		int	getSource();

		int	getTag();

		NodeNum	getMaster();

		~MessagePasser();
	private:
		MessagePasser();
	private:
		static MessagePasser* messagePasser;
		int 	rank;
		int 	size;
		int	source;
		int	tag;
		char	proName[20];
		int	nameLength;
		bool 	isInit;
		void*   recvPool;
		bool 	isLegal(NodeNum);
		vector<void*> buffer;

};

#endif
