#pragma once

#include <QPushButton>

class AnimatedButton : public QPushButton {
	class QPropertyAnimation *animate;
	class QGraphicsColorizeEffect *effect;

  public:
	AnimatedButton(QWidget *parent = nullptr);
	~AnimatedButton();
};
