#pragma once

#include "Queue.h"

#define CHUNKSIZE 1024
#define HASHSIZE 20
#define MAXLENGTH 128

using namespace std;

class Signature
{
private:
	string inputPath;
	string outputPath;
	int chunkSize;
	ifstream inputFile;
	ofstream outputFile;
	Queue<shared_ptr<char>> Writer; // буфер записи
	Queue<shared_ptr<char>> Hasher; // буфер хэша
	Queue<string> ReadyHash; //	буфер уже готовго хэша
	atomic<bool> readfinishedF; // флаг окончания чтения файла
	atomic<bool> writeF; // флаг очереди данных на запись
	atomic<bool> readyhashF; // флаг заполненной очереди хэша
public:
	Signature()
	{
		readfinishedF.store(false);
		writeF.store(false);
		readyhashF.store(false);
		chunkSize = 0;
		inputFile.clear();
		outputFile.clear();
	}

	~Signature()
	{
		if (!inputFile.is_open())
			inputFile.close();
		if (outputFile.is_open())
			outputFile.close();
	}

	void getInputFile();
	void getOutputFile();
	void getChunkSize();
	void hasher();
	void processor();
	void readInputFile();
	void writeOutputFile();
};
