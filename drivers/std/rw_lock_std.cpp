/*************************************************************************/
/*  rw_lock_posix.cpp                                                    */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#if defined(STD_RW_LOCK)

#include "rw_lock_std.h"

#include <mutex>

void RWLockStd::read_lock() {
	int err = pthread_rwlock_rdlock(&rwlock);
	if (err != 0) {
		perror("Acquiring lock failed");
	}
}

void RWLockStd::read_unlock() {
	pthread_rwlock_unlock(&rwlock);
}

Error RWLockStd::read_try_lock() {
	if (pthread_rwlock_tryrdlock(&rwlock) != 0) {
		return ERR_BUSY;
	} else {
		return OK;
	}
}

void RWLockStd::write_lock() {
	int err = pthread_rwlock_wrlock(&rwlock);
}

void RWLockStd::write_unlock() {
	pthread_rwlock_unlock(&rwlock);
}

Error RWLockStd::write_try_lock() {
	if (pthread_rwlock_trywrlock(&rwlock) != 0) {
		return ERR_BUSY;
	} else {
		return OK;
	}
}

RWLock *RWLockStd::create_func_std() {
	return memnew(RWLockStd);
}

void RWLockStd::make_default() {
	create_func = create_func_std;
}

RWLockStd::RWLockStd() {
	//rwlock=PTHREAD_RWLOCK_INITIALIZER; fails on OSX
	pthread_rwlock_init(&rwlock, nullptr);
}

RWLockStd::~RWLockStd() {
	pthread_rwlock_destroy(&rwlock);
}

#endif
