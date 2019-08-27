#ifndef _WORKER_H_
#define _WORKER_H_

#include <pthread.h>
#include "util.h"
#include <random>
#include "ppoint.h"
#include "workload.h"
#include "flags.h"

#define WARMUP 0
#define MEASURE 1
#define WRAPUP 2
#define DONE 3

template<class T>
class Worker {
protected:
	int id;
	T *lock;
	WorkLoad *workload;
	size_t readDelay;
	size_t writeDelay;
	double writePercent;
	size_t outsideDelay;

	std::random_device rd;
	std::mt19937 gen;
	std::uniform_real_distribution<double> dis;

	size_t startTime;
	std::atomic<int> *phase;
	std::atomic<int> count;
	size_t dummy;

public:
	// Worker() { }
	virtual ~Worker() {
	}

	Worker(int id, T *lock, WorkLoad *workload, std::atomic<int> *phase) :
			id(id), lock(lock), workload(workload), readDelay(FLAGS_readDelay), writeDelay(
					FLAGS_writeDelay), writePercent(FLAGS_writePercent), outsideDelay(
					FLAGS_outsideDelay), gen(rd()), dis(0, 1), phase(phase), count(
					0), dummy(1) {
		reset();
	}

	void run(double *throughput) {

		reset();

		while (*phase == WARMUP) {
			work();
		}

		reset();

		while (*phase == MEASURE) {
			work();
		}
		size_t endTime = util_get_time();
		*throughput = double(count) / (endTime - startTime);
		while (*phase == WRAPUP) {
			work();
		}

	}

	size_t get_count() {
		return count.load(memory_order_relaxed);
	}

	virtual void work() {

		for (int i = 0; i < 100; i++) {

			for (size_t j = 0; j < outsideDelay; j++) {
				dummy = dummy + dummy;
			}

			if (dis(gen) < writePercent) {
				write();
			} else {
				read();
			}
			count.fetch_add(1, memory_order_relaxed);
		}
	}

	void read() {
		lock->rdlock();
		workload->read(readDelay);
		lock->rd_unlock();
	}

	void write() {
		lock->wrlock();
		workload->write(writeDelay);
		lock->wr_unlock();
	}

	void reset() {
		count = 0;
		startTime = util_get_time();
	}

};

template<class T>
class PPWorker: public Worker<T> {
protected:
	ppoint_t *ppoint;
public:
//  PPWorker() { }
	virtual ~PPWorker() {
	}
	PPWorker(int id, T *lock, WorkLoad *workload, std::atomic<int> *phase,
			ppoint_t *ppoint) :
			Worker<T>(id, lock, workload, phase), ppoint(ppoint) {
	}
	virtual void work() {
		for (int i = 0; i < 100; i++) {

			for (size_t j = 0; j < this->outsideDelay; j++) {
				this->dummy = this->dummy + this->dummy;
			}

			if (this->dis(this->gen) < this->writePercent) {
				this->write();
			} else {
				this->read();
			}
			this->count++;
		}
		ppoint_tick(ppoint);
	}
};

#endif
