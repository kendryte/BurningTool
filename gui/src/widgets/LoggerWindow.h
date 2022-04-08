#pragma once

#include <QTextEdit>

class LoggerWindow : public QTextEdit {
	Q_OBJECT

	bool autoScroll;

  public slots:
	void append(const QString &text);
	void toggleAutoScroll() { autoScroll = !autoScroll; };
	void clearScreen() { this->clear(); };
	void scrollToBottom();
	void resizeEvent(QResizeEvent *);

  protected:
	void contextMenuEvent(QContextMenuEvent *event);

  public:
	explicit LoggerWindow(QWidget *parent = nullptr);
	~LoggerWindow();

  private:
};
