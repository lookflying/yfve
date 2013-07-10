
/*
 * Connection.h
 *
 *  Created on: Jul 7, 2013
 *      Author: ushrimp
 */
#ifndef SCOPE_LOCK_H_
#define SCOPE_LOCK_H_

#include <pthread.h>

class ScopeLock {
public:
	ScopeLock(pthread_mutex_t *mutex)
		: mutex_(mutex)
	{
		pthread_mutex_lock(mutex_);
	}

	~ScopeLock()
	{
		pthread_mutex_unlock(mutex_);
	}
private:
	pthread_mutex_t *mutex_;
};

#endif // SCOPE_LOCK_H_
