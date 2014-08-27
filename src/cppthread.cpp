#include "cppthread.h"

CppThread::CppThread()
{
	this->exitCode = 0;
	this->started = false;
	pthread_mutex_init(&criticalMutex, NULL);
}

CppThread::~CppThread()
{
	pthread_mutex_destroy(&criticalMutex);
}

void CppThread::process()
{
}

void* CppThread::beginThread(void* ptr)
{
	CppThread* thread = (CppThread*)ptr;
	thread->started = true;
	thread->process();
	thread->started = false;
	return (void*)thread->exitCode;
}

void CppThread::joinThread()
{
	pthread_join(this->threadHandle, NULL);
}

bool CppThread::startThread()
{
	if(this->started)
		return false;
	while(0 != pthread_create(&this->threadHandle, NULL, beginThread, this));
	return true;
}

void CppThread::lock()
{
	pthread_mutex_lock(&this->criticalMutex);
}

void CppThread::unlock()
{
	pthread_mutex_unlock(&this->criticalMutex);
}

