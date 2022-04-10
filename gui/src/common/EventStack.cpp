#include "EventStack.h"
#include <QList>

EventStack::EventStack(int size) : list(QList<void *>(size, NULL)) {
}

void EventStack::set(unsigned int index, void *data) {
	Q_ASSERT(data != NULL);

	mutex.lock();
	list[index] = data;
	mutex.unlock();

	cond.wakeAll();
}

void *EventStack::pick(unsigned int index) {
	while (true) {
		mutex.lock();
		auto ret = list.at(index);
		if (!ret) {
			QDeadlineTimer deadline(10 * 1000);
			cond.wait(&mutex, deadline);
		}
		mutex.unlock();

		if (ret) {
			return ret;
		}
	}
}
