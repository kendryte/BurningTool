#pragma once

#include <QList>
#include <QMutex>
#include <QWaitCondition>

class EventStack {
	QWaitCondition cond;
	QMutex mutex;
	QList<void *> list;

  public:
    EventStack(int result_count);
    void set(unsigned int index, void *data);
    void *pick(unsigned int index);
};
