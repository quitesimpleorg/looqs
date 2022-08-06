#ifndef LIMITQUEUE_H
#define LIMITQUEUE_H
#include <QQueue>

template <class T> class LimitQueue
{
  protected:
	QQueue<T> queue;
	unsigned int limit = 0;

  public:
	LimitQueue();
	LimitQueue(unsigned int limit)
	{
		this->limit = limit;
	}

	void enqueue(const T &t)
	{
		if(queue.size() == limit)
		{
			queue.dequeue();
		}
		queue.enqueue(t);
	}

	int size()
	{
		return queue.size();
	}

	T dequeue()
	{
		return queue.dequeue();
	}

	void setLimit(unsigned int limit)
	{
		this->limit = limit;
	}

	void clear()
	{
		queue.clear();
	}
};

#endif // LIMITQUEUE_H
