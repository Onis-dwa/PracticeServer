#ifndef GRAPH_H
#define GRAPH_H

#include <QObject>
#include <QLabel>
#include <QtGui>
#include <QMouseEvent>

class Graph : public QWidget
{
public:
    Graph(int x, int y, int w, int h, QLabel* l, QString n, QWidget *parent = nullptr);
    ~Graph();

    void AddValue(int);
    void rs(int w, int h);
    bool spectrate;

private:
    int* values;
    int arrsize;
    int last;
    long count;
    int x;
    int y;
    double percent;

    QLabel* val;
    QString name;
    QLabel* out;
    QPixmap* graph;
    QPainter* paint;

protected:
    virtual void mouseMoveEvent(QMouseEvent*);
};

#endif // GRAPH_H
