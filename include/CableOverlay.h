/*
 * CableOverlay.h -
 *
 */

#ifndef CABLE_OVERLAY_H
#define CABLE_OVERLAY_H

#include "Model.h"
#include "ModelView.h"

#include <QObject>
#include <QEvent>
#include <QHash>
#include <QWidget>

class CableOverlay : public QWidget
{
  public:
    CableOverlay(QWidget* _parent);

    void collect(QWidget* _w, QHash<ModelView*, Model*>& _table);

    bool eventFilter(QObject *obj, QEvent *event) override;
    void paintEvent(QPaintEvent* _pe) override;
};

#endif
