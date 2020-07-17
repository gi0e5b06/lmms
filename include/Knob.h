/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Knob.h - powerful knob-widget
 *
 * Copyright (c) 2017-2020 gi0e5b06 (on github.com)
 * Copyright (c) 2004-2008 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 *
 * This file is part of LSMM -
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef KNOB_H
#define KNOB_H

#include "AutomatableModelView.h"
#include "Widget.h"
//#include "PaintCacheable.h"
//#include "templates.h"

#include <QLine>
#include <QPoint>
//#include <QWidget>

class QPixmap;
class TextFloat;

enum knobTypes
{
    knobDark_28,
    knobBright_26,
    knobSmall_17,
    knobVintage_32,
    knobStyled
};

class EXPORT Knob : public Widget, public FloatModelView
{
    Q_OBJECT
    Q_ENUMS(knobTypes)

  public:
    Q_PROPERTY(float innerRadius READ innerRadius WRITE setInnerRadius)
    Q_PROPERTY(float outerRadius READ outerRadius WRITE setOuterRadius)

    Q_PROPERTY(float centerPointX READ centerPointX WRITE setCenterPointX)
    Q_PROPERTY(float centerPointY READ centerPointY WRITE setCenterPointY)

    Q_PROPERTY(float lineWidth READ lineWidth WRITE setLineWidth)
    Q_PROPERTY(knobTypes knobNum READ knobNum WRITE setknobNum)

    // Unfortunately, the gradient syntax doesn't create our gradient
    // correctly so we need to do this:
    Q_PROPERTY(QColor outerColor READ outerColor WRITE setOuterColor)
    Q_PROPERTY(QColor lineColor READ lineColor WRITE setlineColor)
    Q_PROPERTY(QColor arcColor READ arcColor WRITE setarcColor)
    Q_PROPERTY(QColor pointColor READ pointColor WRITE setPointColor)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)

    // mapPropertyFromModel(bool, isVolumeKnob, setVolumeKnob, m_volumeKnob);
    // mapPropertyFromModel(float, volumeRatio, setVolumeRatio,
    // m_volumeRatio);

    Knob(knobTypes      _knob_num,
         QWidget*       _parent = nullptr,
         const QString& _name   = "[knob]");
    Knob(QWidget*       _parent = nullptr,
         const QString& _name   = "[knob]");  //!< default ctor
    virtual ~Knob();

    void initUi(const QString& _name);  //!< to be called by ctors
    void onKnobNumUpdated();  //!< to be called when you updated @a m_knobNum

    // TODO: remove, tmp
    /*
    void setVolumeKnob(bool _b)
    {
        qWarning("Knob: setVolumeKnob is deprecated, use a VolumeKnob
    instead");
    }
    */

    // TODO: remove
    inline void setHintText(const QString& _txt_before,
                            const QString& _txt_after)
    {
        setDescription(_txt_before);
        setUnit(_txt_after);
    }

    virtual void setModel(Model* _m, bool isOldModelValid = true);

    void setLabel(const QString& txt);  // deprecated

    QString text() const;
    void    setText(const QString& _s);

    // bool isInteractive() const;
    // void setInteractive(bool _b);

    void setTotalAngle(float angle);

    // Begin styled knob accessors
    float innerRadius() const;
    void  setInnerRadius(float r);

    float outerRadius() const;
    void  setOuterRadius(float r);

    knobTypes knobNum() const;
    void      setknobNum(knobTypes k);

    QPointF centerPoint() const;
    float   centerPointX() const;
    void    setCenterPointX(float c);
    float   centerPointY() const;
    void    setCenterPointY(float c);

    float lineWidth() const;
    void  setLineWidth(float w);

    QColor outerColor() const;
    void   setOuterColor(const QColor& c);
    QColor lineColor() const;
    void   setlineColor(const QColor& c);
    QColor arcColor() const;
    void   setarcColor(const QColor& c);
    QColor pointColor() const;
    void   setPointColor(const QColor& c);
    QColor textColor() const;
    void   setTextColor(const QColor& c);

    virtual QLine  cableFrom() const;
    virtual QLine  cableTo() const;
    virtual QColor cableColor() const;

  signals:
    void sliderPressed();
    void sliderReleased();
    void sliderMoved(float value);

  public slots:
    virtual void modelChanged();
    virtual void enterValue();
    virtual void editRandomization();
    virtual void update();
    virtual void friendlyUpdate();

    void displayHelp();
    void toggleScale();

  protected:
    virtual void contextMenuEvent(QContextMenuEvent* _ce);
    virtual void dragEnterEvent(QDragEnterEvent* _dee);
    virtual void dropEvent(QDropEvent* _de);
    virtual void focusOutEvent(QFocusEvent* _fe);
    virtual void mousePressEvent(QMouseEvent* _me);
    virtual void mouseReleaseEvent(QMouseEvent* _me);
    virtual void mouseMoveEvent(QMouseEvent* _me);
    virtual void mouseDoubleClickEvent(QMouseEvent* _me);
    virtual void wheelEvent(QWheelEvent* _we);

    // virtual float getValue( const QPoint & _p );
    virtual void convert(const QPoint& _p, float& value_, float& dist_);
    virtual void setPosition(const QPoint& _p, bool _shift);

    virtual QString displayValue() const;

  private:
    void onTextUpdated();

    virtual void doConnections();

    QLineF calculateLine(const QPointF& _mid,
                         float          _radius,
                         float          _innerRadius = 1) const;

    QColor statusColor();

    // void clearCache();
    void  drawWidget(QPainter& _p);
    void  drawKnob(QPainter& _p);
    void  drawText(QPainter& _p);
    bool  updateAngle();
    float angleFromValue(float value,
                         float minValue,
                         float maxValue,
                         float totalAngle) const;

    /*
    inline float pageSize() const
    {
            return ( model()->maxValue() - model()->minValue() ) / 100.0f;
    }
    */

    static TextFloat* s_textFloat;

    float  m_pressValue;  // model value when left button pressed
    QPoint m_pressPos;    // mouse pos when left button pressed
    bool   m_pressLeft;   // true when left button pressed

    QString m_text;
    // TODO: Qt::AnchorPoint m_textAnchor;
    // bool    m_interactive;

    QPixmap* m_knobPixmap;

    // QPoint m_mouseOffset;
    // QPoint m_origMousePos;
    // float m_leftOver;
    // bool m_buttonPressed;

    float m_totalAngle;
    float m_angle;
    // QPixmap* m_cache;

    // Styled knob stuff, could break out
    QPointF m_centerPoint;
    float   m_innerRadius;
    float   m_outerRadius;
    float   m_lineWidth;
    QColor  m_outerColor;
    QColor  m_lineColor;  //!< unused yet
    QColor  m_arcColor;   //!< unused yet
    QColor  m_pointColor;
    QColor  m_statusColor;  //!< used as an indicator
    QColor  m_textColor;

    knobTypes m_knobNum;
};

class EXPORT BalanceKnob : public Knob
{
    Q_OBJECT

  public:
    BalanceKnob(QWidget* _parent);
};

class EXPORT CutoffFrequencyKnob : public Knob
{
    Q_OBJECT

  public:
    CutoffFrequencyKnob(QWidget* _parent);
};

class EXPORT FrequencyKnob : public Knob
{
    Q_OBJECT

  public:
    FrequencyKnob(QWidget* _parent);
};

class EXPORT MixKnob : public Knob
{
    Q_OBJECT

  public:
    MixKnob(QWidget* _parent);
};

class EXPORT ResonanceKnob : public Knob
{
    Q_OBJECT

  public:
    ResonanceKnob(QWidget* _parent);
};

class EXPORT VolumeKnob : public Knob
{
    Q_OBJECT

  public:
    // mapPropertyFromModel(bool, isVolumeKnob, setVolumeKnob, m_volumeKnob);
    mapPropertyFromModel(float, volumeRatio, setVolumeRatio, m_volumeRatio);

    VolumeKnob(knobTypes _knob_num, QWidget* _parent);
    VolumeKnob(QWidget* _parent);

    virtual QString displayValue() const;

  public slots:
    virtual void enterValue();

  private:
    BoolModel  m_volumeKnob;
    FloatModel m_volumeRatio;
};

#endif
