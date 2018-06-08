#include "graph.h"

Graph::Graph(int x, int y, int w, int h, QLabel* l, QString n, QWidget *parent) : QWidget(parent)
{
    this->setGeometry(x, y, w, h);
    this->setMouseTracking(true);
    this->show();

    QPalette palette;
    palette.setBrush(QPalette::Background, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(palette);

    val = l;
    name = n;

    graph = new QPixmap(w, h);
    paint = new QPainter;
    out = new QLabel(this);
    out->setFrameShape(out->Panel);
    out->setFrameShadow(out->Sunken);
    out->setAlignment(Qt::AlignRight);
    out->setGeometry(0, 0, w, h);
    out->setMouseTracking(true);
    out->show();

    arrsize = w * 2;
    values = new int[arrsize];

    for (last = arrsize - 1; last >= 0; last--)
        values[last] = 0;

    percent = (double)h / 100.0;
    spectrate = false;

    count = 0;
    min = 101;
    max = -1;
    avg = 0;
    value = 0;
}
Graph::~Graph()
{

}
void Graph::AddValue(int val)
{
    if (last == arrsize) // с начала
        last = 0;

    values[last] = val;
    count++;

    if (val > max)
        max = val;
    if (val < min)
        min = val;

    value += val;
    avg = value / count;

    x = out->width();
    y = out->height();

    paint->begin(graph);
    paint->eraseRect(0, 0, x, y);
    paint->setPen(QPen(Qt::red, 1));

    for (int i = last; i >= 0; i--) // от last до начала
    {
        int value = values[i];
        if (value > 0)
            paint->drawLine(x, y - percent * value, x--, y);
    }
    for (int i = arrsize; i >= last; i--) // от конца до last
    {
        int value = values[i];
        if (value > 0)
            paint->drawLine(x, y - percent * value, x--, y);
    }
    last++;

    paint->end();
    out->setPixmap(*graph);
}
void Graph::rs(int w, int h)
{
    this->resize(w, h);
    graph = new QPixmap(w, h);
    out->resize(w, h);
}

void Graph::mouseMoveEvent(QMouseEvent* e)
{
    spectrate = true;

    if (e->x() < 0 || e->x() > out->width() || e->y() < 0 || e->y() > out->height())
    {
        spectrate = false;
        return;
    }

    int pos = out->width() - e->x();
    if (pos > count)
        return;

    if (pos <= last)
    {
        val->setText(name + QString::number(values[last - pos]));
    }
    else
    {
        val->setText(name + QString::number(values[arrsize - last - pos]));
    }
}

