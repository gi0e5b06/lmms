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

#include "AutomatableModelView.h" // REQUIRED
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

class EXPORT Knob : public Widget, public RealModelView
{
    Q_OBJECT
    Q_ENUMS(knobTypes)

  public:
    Q_PROPERTY(real_t innerRadius READ innerRadius WRITE setInnerRadius)
    Q_PROPERTY(real_t outerRadius READ outerRadius WRITE setOuterRadius)

    Q_PROPERTY(real_t centerPointX READ centerPointX WRITE setCenterPointX)
    Q_PROPERTY(real_t centerPointY READ centerPointY WRITE setCenterPointY)

    Q_PROPERTY(real_t lineWidth READ lineWidth WRITE setLineWidth)
    Q_PROPERTY(knobTypes knobNum READ knobNum WRITE setknobNum)

    // Unfortunately, the gradient syntax doesn't create our gradient
    // correctly so we need to do this:
    Q_PROPERTY(QColor outerColor READ outerColor WRITE setOuterColor)
    Q_PROPERTY(QColor lineColor READ lineColor WRITE setlineColor)
    Q_PROPERTY(QColor arcColor READ arcColor WRITE setarcColor)
    Q_PROPERTY(QColor pointColor READ pointColor WRITE setPointColor)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)

    // mapPropertyFromModel(bool, isVolumeKnob, setVolumeKnob, m_volumeKnob);
    // mapPropertyFromModel(real_t, volumeRatio, setVolumeRatio,
    // m_volumeRatio);

    Knob(RealModel* _model,
         QWidget*   _parent,
         knobTypes  _knobNum = knobBright_26);
    Knob(QWidget* _parent = nullptr, const QString& _name = "[knob]");

    // obsolete
    Knob(knobTypes      _knobNum,
         QWidget*       _parent = nullptr,
         const QString& _name   = "[knob]");

    virtual ~Knob();

    void initUi();            //!< to be called by ctors
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
    INLINE void setHintText(const QString& _txt_before,
                            const QString& _txt_after)
    {
        setDescription(_txt_before);
        setUnit(_txt_after);
    }

    // virtual void setModel(Model* _m, bool isOldModelValid = true);

    void setLabel(const QString& txt);  // deprecated

    QString text() const;
    void    setText(const QString& _s);

    // bool isInteractive() const;
    // void setInteractive(bool _b);

    void setTotalAngle(real_t angle);

    // Begin styled knob accessors
    real_t innerRadius() const;
    void   setInnerRadius(real_t r);

    real_t outerRadius() const;
    void   setOuterRadius(real_t r);

    knobTypes knobNum() const;
    void      setknobNum(knobTypes k);

    QPointF centerPoint() const;
    real_t  centerPointX() const;
    void    setCenterPointX(real_t c);
    real_t  centerPointY() const;
    void    setCenterPointY(real_t c);

    real_t lineWidth() const;
    void   setLineWidth(real_t w);

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
    void sliderMoved(real_t value);

  public slots:
    virtual void modelChanged();
    virtual void enterValue();
    virtual void editRandomization();
    virtual void update();
    virtual void friendlyUpdate();

    void displayHelp();
    void toggleScale();

  protected:
    void contextMenuEvent(QContextMenuEvent* _ce) override;
    void dragEnterEvent(QDragEnterEvent* _dee) override;
    void dropEvent(QDropEvent* _de) override;
    void focusOutEvent(QFocusEvent* _fe) override;
    void mousePressEvent(QMouseEvent* _me) override;
    void mouseReleaseEvent(QMouseEvent* _me) override;
    void mouseMoveEvent(QMouseEvent* _me) override;
    void mouseDoubleClickEvent(QMouseEvent* _me) override;
    void wheelEvent(QWheelEvent* _we) override;

    void doConnections() override;
    void undoConnections() override;

    // virtual real_t getValue( const QPoint & _p );
    virtual void convert(const QPoint& _p, real_t& value_, real_t& dist_);
    virtual void setPosition(const QPoint& _p, bool _shift);

    virtual QString displayValue() const;

  private:
    void onTextUpdated();

    QLineF calculateLine(const QPointF& _mid,
                         real_t         _radius,
                         real_t         _innerRadius = 1) const;

    QColor statusColor();

    // void clearCache();
    void   drawWidget(QPainter& _p);
    void   drawKnob(QPainter& _p);
    void   drawText(QPainter& _p);
    bool   updateAngle();
    real_t angleFromValue(real_t value,
                          real_t minValue,
                          real_t maxValue,
                          real_t totalAngle) const;

    /*
    INLINE real_t pageSize() const
    {
            return ( model()->maxValue() - model()->minValue() ) / 100.;
    }
    */

    static TextFloat* s_textFloat;

    real_t m_pressValue;  // model value when left button pressed
    QPoint m_pressPos;    // mouse pos when left button pressed
    bool   m_pressLeft;   // true when left button pressed

    QString m_text;
    // TODO: Qt::AnchorPoint m_textAnchor;
    // bool    m_interactive;

    QPixmap* m_knobPixmap;

    // QPoint m_mouseOffset;
    // QPoint m_origMousePos;
    // real_t m_leftOver;
    // bool m_buttonPressed;

    real_t m_totalAngle;
    real_t m_angle;
    // QPixmap* m_cache;

    // Styled knob stuff, could break out
    QPointF m_centerPoint;
    real_t  m_innerRadius;
    real_t  m_outerRadius;
    real_t  m_lineWidth;
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
    mapPropertyFromModel(real_t, volumeRatio, setVolumeRatio, m_volumeRatio);

    VolumeKnob(knobTypes _knob_num, QWidget* _parent);
    VolumeKnob(QWidget* _parent);

    virtual QString displayValue() const;

  public slots:
    virtual void enterValue();

  private:
    BoolModel m_volumeKnob;
    RealModel m_volumeRatio;
};

#endif
