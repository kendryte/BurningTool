#include "EventStack.h"
#include <QDebug>
#include <QList>

EventStack::EventStack(int size) : list(QList<void *>(size, NULL)) {
}

void EventStack::set(unsigned int index, void *data) {
	Q_ASSERT(data != NULL);

	if (canceled)
		return;

	mutex.lock();
	list[index] = data;
	mutex.unlock();

	cond.wakeAll();
}
void EventStack::cancel() {
	this->canceled = true;
	cond.wakeAll();
}

void *EventStack::pick(unsigned int index) {
	while (true) {
		mutex.lock();
		auto ret = list.at(index);
		if (!ret) {
			if (canceled)
				return NULL;

			QDeadlineTimer deadline(10 * 1000);
			bool success = cond.wait(&mutex, deadline);
			if (!success) {
				qDebug() << "process wait condition timeout.";
				return NULL;
			}
		}
		mutex.unlock();

		if (ret) {
			return ret;
		}
	}
}
