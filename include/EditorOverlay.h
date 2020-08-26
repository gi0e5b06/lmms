/*
 * EditorOverlay.h -
 *
 */

#ifndef EDITOR_OVERLAY_H
#define EDITOR_OVERLAY_H

#include "Editor.h"
#include "Widget.h"

//#include <QEvent>
//#include <QHash>
//#include <QObject>
#include <QWidget>

class EditorOverlay : public Widget
{
    Q_OBJECT

  public:
    EditorOverlay(QWidget* _parent, Editor* _editor);

    static void
            drawModeCursor(QPainter& _p, QWidget& _w, Editor::EditMode _mode);

  public slots:
    void check();
    void verticalLine(int _xs, int _xe);
    void horizontalLine(int _ys, int _ye);

  protected:
    void drawWidget(QPainter& _p) override;
    void drawModeCursor(QPainter& _p, Editor::EditMode _mode);
    void drawVerticalLine(QPainter& p);
    void drawHorizontalLine(QPainter& p);
    // void paintEvent(QPaintEvent* _pe) override;

    Editor* m_editor;
    int     m_vlxs, m_vlxe;
    int     m_hlys, m_hlye;
};

#endif
