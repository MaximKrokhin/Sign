#include "Signature.h"

using namespace std;

void Signature::getInputFile()
{
	cout << "Input file full path: " << endl;
	getline(cin,inputPath);
	inputFile.open(inputPath, ios::binary);
	if (!inputFile.is_open())
		throw runtime_error("Can't open this file");
}

void Signature::getOutputFile()
{
	cout << "Output file full path: " << endl;
	getline(cin, outputPath);
	if (outputPath.empty())
	{
		cout << "We will create output file out.txt in local directory" << endl;
		outputPath.assign("./out.txt");
	}
	outputFile.open(outputPath, ios::binary);
	if (!outputFile.is_open())
	{
		throw runtime_error("Can't find/create output file");
	}
}

void Signature::getChunkSize()
{
	cout << "Enter block size parameter(default - 1mb)" << endl;
	cin >> chunkSize;
	if (chunkSize == 0)
	{
		chunkSize = CHUNKSIZE;
		cout << "Default block size"<<endl;
	}
	chunkSize *= 1024;
}

void Signature::readInputFile()
{
	unsigned int fileSize;
	unsigned int _chunkSize = chunkSize;
	if (!inputFile)
	{
		throw runtime_error("Reading file is closed");
	}
	else
	{
		inputFile.seekg(0, ios::end);
		fileSize = inputFile.tellg();
		inputFile.seekg(0, ios::beg);
		while (fileSize)
		{
			shared_ptr<char> InChunkPtr(new char[chunkSize], [](char* buf) {delete[] buf; });
			if (fileSize < _chunkSize)
				_chunkSize = fileSize;
			inputFile.read(InChunkPtr.get(), _chunkSize);
			Writer.push(InChunkPtr);
			Hasher.push(InChunkPtr);
			fileSize -= _chunkSize;
			if (!writeF.load())
				writeF.store(true);
		}
		readfinishedF.store(true);
		inputFile.close();
	}
}

void Signature::writeOutputFile() 
{
	if (!outputFile)
	{
		throw runtime_error("Output file is closed");
	}
	else
	{
		outputFile.clear();
		while (!writeF.load())
			this_thread::sleep_for(chrono::milliseconds(100));
		while (true)
		{
			shared_ptr<char> OutChunkPtr(new char[chunkSize], [](char* buf) {delete[] buf; });
			Writer.pop(OutChunkPtr);
			outputFile.write(reinterpret_cast<char*>(OutChunkPtr.get()), chunkSize);
			while (!readyhashF.load()) 
				this_thread::sleep_for(chrono::milliseconds(50));
			Queue<string> temp = ReadyHash;
			ReadyHash.pop();
			readyhashF.store(false);
			outputFile.write(reinterpret_cast<char*>(& temp), HASHSIZE);
			if (readfinishedF.load() && Writer.isEmpty() && ReadyHash.isEmpty()) 
				break;
		}
		cout << "Finish:" << readfinishedF << endl;
	}
}

void Signature::hasher()
{
	while (!writeF.load())
		this_thread::sleep_for(chrono::milliseconds(100));
	while (true) 
	{
		hash<string> hash_fn;
		shared_ptr<char> HashChunkPtr(new char[chunkSize], [](char* buf) {delete[] buf; });
		Hasher.pop(HashChunkPtr);
		size_t hashtemp; 
		hashtemp^= hash_fn(HashChunkPtr.get());
		string result = to_string(hashtemp);
		while (!readyhashF.load())
			this_thread::sleep_for(chrono::milliseconds(10));
		ReadyHash.push(result);
		readyhashF.store(true);
		if (readfinishedF.load() && ReadyHash.isEmpty())
			break;
	}
}

void Signature::processor()
{
	thread Reader(&Signature::readInputFile, this);
	if (Reader.joinable())
		Reader.detach();
	thread Hasher(&Signature::hasher, this);
	if (Hasher.joinable())
		Hasher.detach();
	thread Writer(&Signature::writeOutputFile, this);
	if (Writer.joinable())
		Writer.detach();
}