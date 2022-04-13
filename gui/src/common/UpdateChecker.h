#pragma once

#include <QObject>

class UpdateChecker : public QObject {
	Q_OBJECT
  public:
	explicit UpdateChecker(class QMenu *parent = nullptr);

  signals:
};
