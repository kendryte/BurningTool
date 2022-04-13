#pragma once

#include <QFile>
#include <QTextEdit>

class LoggerWindow : public QTextEdit {
	Q_OBJECT

	bool autoScroll = true;
	QFile logfile;
	bool trace_visible;

  public slots:
    void append(bool isTrace, const QString &text);
    void toggleAutoScroll() { autoScroll = !autoScroll; };
    void clearScreen() { this->clear(); };
    void scrollToBottom();
    void resizeEvent(QResizeEvent *);
    void setTraceLevelVisible(bool visible) { trace_visible = visible; };

  protected:
    void contextMenuEvent(QContextMenuEvent *event);

  public:
    explicit LoggerWindow(QWidget *parent = nullptr);
    ~LoggerWindow();

  private:
};
