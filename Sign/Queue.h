#pragma once

#include <queue>
#include <mutex>
#include <memory>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <exception>
#include <thread>
#include <atomic>
#include <queue>
#include <array>
#include <filesystem>
#include <string>

using namespace std;

template <class T>
class Queue
{
public:
	Queue() {};
	Queue(Queue const& other) 
	{
		lock_guard<mutex> qlock(other.qmutex);
		myqueue = other.myqueue;
	}

	~Queue()
	{
		
	};

	Queue& operator= (const Queue&) = delete;

	void push(T value)
	{
		lock_guard<mutex> qlock(qmutex);
		myqueue.push(value);
		condition.notify_one();
	}

	void pop(T& value)
	{
		unique_lock<mutex> qlock(qmutex);
		condition.wait(qlock, [this] {return !myqueue.empty(); });
		value = myqueue.front();
		myqueue.pop();
	}

	T pop() 
	{
		unique_lock<mutex> qlock(qmutex);
		condition.wait(qlock, [this] {return !myqueue.empty(); });
		T value = myqueue.front();
		myqueue.pop();
		return value;
	}

	bool isEmpty() const 
	{
		lock_guard<mutex> qlock();
		return myqueue.empty();
	}

private:
	mutable mutex qmutex;
	queue<T> myqueue;
	condition_variable condition;
};

