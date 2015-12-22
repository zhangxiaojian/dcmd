/*
 * =====================================================================================
 *
 *       Filename:  MessagePasser.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2015年06月04日 23时56分38秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  zj, xiaojian_whu@163.com
 *        Company:  whu.sklse
 *
 * =====================================================================================
 */

#include <cstdlib>
#include <cstring>
#include <iostream>
#include "MessagePasser.h"

MessagePasser* MessagePasser::messagePasser = new MessagePasser();

MessagePasser* MessagePasser::getInstance()
{
	if(messagePasser == NULL)
		return new MessagePasser();
	else
		return messagePasser;
}

MessagePasser::MessagePasser():isInit(false),
	size(7),source(0),tag(6)
{
	recvPool = malloc(PoolSize);
	if(recvPool == NULL)
		cerr << "MessagePasser init failed ! Memory is not enough" << endl;
}

MessagePasser::~MessagePasser()
{
	free(recvPool);
	MPI_Finalize();
}

string MessagePasser::getProName()
{
	string name(proName);
	return name;
}

int MessagePasser::getRank()
{
	return rank;
}

int MessagePasser::getSize()
{
	return size;
}

int MessagePasser::getSource()
{
	return source;
}

int MessagePasser::getTag()
{
	return tag;
}

NodeNum MessagePasser::getMaster()
{
	return Master;
}

bool MessagePasser::isMaster()
{
	return size < 1 ? false : rank == Master;
}

void MessagePasser::init(int* argc,char*** argv)
{
	if(!isInit)
	{
		MPI_Init(argc,argv);
		MPI_Comm_rank(MPI_COMM_WORLD,&rank);
		MPI_Comm_size(MPI_COMM_WORLD,&size);
		MPI_Get_processor_name(proName,&nameLength);
		isInit = true;
	}
}

bool MessagePasser::isLegal(NodeNum nodeNum)
{
	return nodeNum < size ? true : false;
}

/**
 * 发送数据到指定的结点 
 * MPI_Send 中的参数6相当于是端口号 这里直接指定 接收的时候注意一致即可
 * @param string
 * @param NodeNum
 */
void MessagePasser::send(string data, NodeNum nodeNum)
{
	if(isLegal(nodeNum))
	{
		if(data.empty())
			data += "\0";
		MPI_Send(data.c_str(), data.size() + 1, MPI_BYTE, nodeNum, 6, MPI_COMM_WORLD);
	}
}

void MessagePasser::broadCast(string data)
{
	for(NodeNum i = 1; i < size; ++i)
	{
		send(data, i);
	}
}

/**
 * 接收发送来的消息，为了避免重复申请空间，每次发来的消息都使用本机MessagePasser
 * 初始化时申请的空间。如果不够就扩充。随后都是用扩充后的空间大小
 *
 * @param int  实际接收到的数据大小 单位为Byte
 * @retrun  data  返回的指针，指向接收的数据
 */
void* MessagePasser::receive(int& count)
{
	if(recvPool != NULL)
	{
		MPI_Status status;
		MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		MPI_Get_count(&status, MPI_BYTE, &count);
		if(count > PoolSize)
		{
			free(recvPool);
			recvPool = malloc(count);
		}

		if(recvPool == NULL)
		{
			cerr << "no memory enough to receive message. " << endl;
			return NULL;
		}

		MPI_Recv(recvPool,PoolSize,MPI_BYTE,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
		source = status.MPI_SOURCE;
		tag = status.MPI_TAG;

		return recvPool;
	}

	return NULL;
}

void* MessagePasser::receive(int& count,NodeNum node)
{
	void* recvBuffer = NULL;
	MPI_Status status;
	MPI_Probe(MPI_ANY_SOURCE, 6, MPI_COMM_WORLD, &status);
	MPI_Get_count(&status, MPI_BYTE, &count);
	if((recvBuffer = malloc(count)) == NULL)
		cerr << "no memory enough to receive message. " << endl;
	else{
		buffer.push_back(recvBuffer);
		MPI_Recv(recvBuffer, count, MPI_BYTE, node, 6, MPI_COMM_WORLD, &status);
	}

	return recvBuffer;
}

void MessagePasser::execute(string& cmd, NodeNum node)
{
	int count = 0;
	send(cmd, node);
	string result = (char*)receive(count, node);
	if(!result.empty())
		cout << "node" << node << ":\n" << result << endl;
}

void MessagePasser::executeAll(string& cmd)
{
	for(int i = 1; i < size; ++i)
	{
		int count = 0;
		send(cmd, i);
		string result = (char*)receive(count, i);
		if(!result.empty())
			cout << "node" << i << ":\n" << result << endl;
	}
}

/**清除缓存
 * 由于接收到的message会malloc内存空间，但是部分反序列化导致应用层不知道什么时候
 * 释放内存，因此把内存指针放到一个集合里，统一释放。
 *
 */
void MessagePasser::clear()
{
	vector<void*>::iterator iter = buffer.begin();
	for(; iter != buffer.end(); ++iter)
	{
		if(*iter != NULL)
			free(*iter);
	}
	buffer.clear();
}
