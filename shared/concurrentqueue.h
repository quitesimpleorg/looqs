#ifndef CONCURRENTQUEUE_H
#define CONCURRENTQUEUE_H
#include <QList>
#include <QMutex>
#include <QSemaphore>
#define QUEUE_SIZE 10000
template <class T> class ConcurrentQueue : protected QList<T>
{
  protected:
	QMutex mutex;

	QSemaphore avail{QUEUE_SIZE};

  public:
	void enqueue(const T &t)
	{
		avail.acquire(1);
		QMutexLocker locker(&mutex);
		QList<T>::append(t);
	}

	QVector<T> dequeue(int batchsize)
	{
		avail.release(batchsize);
		// TODO: this sucks
		QVector<T> result;
		QMutexLocker locker(&mutex);
		for(int i = 0; i < batchsize; i++)
		{
			result.append(QList<T>::takeFirst());
		}
		return result;
	}

	void enqueue(const QVector<T> &t)
	{
		QList<T> tmp(t.begin(), t.end());
		avail.acquire(t.size());
		QMutexLocker locker(&mutex);
		QList<T>::append(tmp);
	}

	unsigned int remaining()
	{
		return QUEUE_SIZE - avail.available();
	}

	void clear()
	{
		QMutexLocker locker(&mutex);
		QList<T>::clear();
		avail.release(QUEUE_SIZE);
	}

	bool dequeue(T &result)
	{
		QMutexLocker locker(&mutex);
		if(QList<T>::isEmpty())
			return false;
		avail.release(1);
		result = QList<T>::takeFirst();
		return true;
	}
};

#endif // CONCURRENTQUEUE_H
