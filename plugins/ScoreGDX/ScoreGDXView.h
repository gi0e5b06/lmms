/*
 */

#ifndef SCOREGDX_VIEW_H
#define SCOREGDX_VIEW_H

#include "ToolPluginView.h"

#include <QVector>
#include <QWidget>

class ScoreGDXView : public ToolPluginView
{
    Q_OBJECT

  public:
    ScoreGDXView(ToolPlugin* plugin);
    ~ScoreGDXView() override;

  public slots:
    // void update();

  protected:
    bool drawStave(QPainter& p, int& x, int& y, int w, int h);
    bool drawTempo(QPainter& p, int& x, int& y, int w, int h, bpm_t _tempo);
    bool drawBar(QPainter& p, int& x, int& y, int w, int h, int _bar);
    bool drawClef(QPainter& p, int& x, int& y, int w, int h, int _clef);
    bool drawSignature(
            QPainter& p, int& x, int& y, int w, int h, int _n, int _d);
    bool drawNote(
            QPainter& p, int& x, int& y, int w, int h, int _key, int _ticks);
    bool drawChord(QPainter&    p,
                   int&         x,
                   int&         y,
                   int          w,
                   int          h,
                   QVector<int> _keys,
                   int          _ticks);
    bool drawSilence(QPainter& p, int& x, int& y, int w, int h, int _ticks);

    virtual void paintEvent(QPaintEvent* pe);
};

#endif
