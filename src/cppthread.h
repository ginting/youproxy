#ifndef CPPTHREAD_H
#define CPPTHREAD_H

#include <pthread.h>

class CppThread {
private:
	pthread_t		threadHandle;
	int				exitCode;
	static void*	beginThread(void* ptr);
	bool			started;
	pthread_mutex_t	criticalMutex;
public:
	CppThread();
	virtual ~CppThread();
	virtual void	process();
	void			joinThread();
	bool			startThread();
	void			lock();
	void			unlock();
};

#endif // CPPTHREAD_H
