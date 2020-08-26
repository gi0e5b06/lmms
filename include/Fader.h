/*
 * Fader.h - fader-widget used in FX-mixer - partly taken from Hydrogen
 *
 * Copyright (c) 2008-2012 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 *
 * This file is part of LMMS - https://lmms.io
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 */

/*
 * Hydrogen
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 *
 * http://www.hydrogen-music.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef FADER_H
#define FADER_H

#include <QTime>
//#include <QWidget>
#include "AutomatableModelView.h"  // REQUIRED
#include "Widget.h"

#include <QPixmap>

class TextFloat;

class EXPORT Fader : public Widget, public RealModelView
{
    Q_OBJECT

  public:
    Q_PROPERTY(QColor peakGreen READ peakGreen WRITE setPeakGreen)
    Q_PROPERTY(QColor peakRed READ peakRed WRITE setPeakRed)
    Q_PROPERTY(QColor peakYellow READ peakYellow WRITE setPeakYellow)
    Q_PROPERTY(bool levelsDisplayedInDBFS READ areLevelsDisplayedInDBFS WRITE
                       setLevelsDisplayedInDBFS)

    Fader(RealModel* _model, QWidget* _parent);
    Fader(QWidget* _parent = nullptr, const QString& _name = "[fader]");

    // obsolete
    Fader(RealModel* _model, const QString& _name, QWidget* _parent);
    Fader(RealModel*     _model,
          const QString& _name,
          QWidget*       _parent,
          const QPixmap& back,
          const QPixmap& leds,
          const QPixmap& knob);

    virtual ~Fader();

    // INLINE bool needsUpdate() const { return m_needsUpdate; }

    INLINE real_t getPeak_L() const
    {
        return m_fPeakValue_L;
    }
    void setPeak_L(real_t fPeak);

    INLINE real_t getPeak_R() const
    {
        return m_fPeakValue_R;
    }
    void setPeak_R(real_t fPeak);

    INLINE real_t getMinPeak() const
    {
        return m_fMinPeak;
    }

    INLINE void setMinPeak(real_t minPeak)
    {
        m_fMinPeak = minPeak;
    }

    INLINE real_t getMaxPeak() const
    {
        return m_fMaxPeak;
    }

    INLINE void setMaxPeak(real_t maxPeak)
    {
        m_fMaxPeak = maxPeak;
    }

    QColor const& peakGreen() const;
    void          setPeakGreen(const QColor& c);

    QColor const& peakRed() const;
    void          setPeakRed(const QColor& c);

    QColor const& peakYellow() const;
    void          setPeakYellow(const QColor& c);

    // INLINE bool getLevelsDisplayedInDBFS() const { return
    // m_levelsDisplayedInDBFS; }
    INLINE bool areLevelsDisplayedInDBFS() const
    {
        return m_levelsDisplayedInDBFS;
    }
    void setLevelsDisplayedInDBFS(bool _b)
    {
        m_levelsDisplayedInDBFS = _b;
    }

    void setDisplayConversion(bool b)
    {
        m_displayConversion = b;
    }

    INLINE void setHintText(const QString& _txt_before,
                            const QString& _txt_after)
    {
        setDescription(_txt_before);
        setUnit(_txt_after);
    }

  public slots:
    virtual void modelChanged();
    virtual void enterValue();
    // virtual void editRandomization();
    virtual void update();
    virtual void friendlyUpdate();

    // void displayHelp();
    // void toggleScale();

  protected:
    void doConnections() override;
    void undoConnections() override;

    virtual void initUi();
    virtual void drawWidget(QPainter& _p);
    virtual void drawDBFSLevels(QPainter& painter);
    virtual void drawLinearLevels(QPainter& painter);

    void contextMenuEvent(QContextMenuEvent* _me) override;
    void mousePressEvent(QMouseEvent* ev) override;
    void mouseDoubleClickEvent(QMouseEvent* mouseEvent) override;
    void mouseMoveEvent(QMouseEvent* ev) override;
    void mouseReleaseEvent(QMouseEvent* _me) override;
    void wheelEvent(QWheelEvent* ev) override;
    // void paintEvent( QPaintEvent *ev ) override;

  private:
    INLINE bool isClipping(const real_t _value) const
    {
        return _value > 1.;
    }

    // void paintDBFSLevels(QPaintEvent *ev, QPainter & painter);
    // void paintLinearLevels(QPaintEvent *ev, QPainter & painter);

    int knobPosY() const
    {
        real_t fRange  = model()->maxValue() - model()->minValue();
        real_t realVal = model()->value() - model()->minValue();

        return height() - ((height() - m_knob.height()) * (realVal / fRange));
    }

    void setPeak(real_t  fPeak,
                 real_t& targetPeak,
                 real_t& persistentPeak,
                 QTime&  lastPeakTime);
    int  calculateDisplayPeak(real_t fPeak);

    void updateTextFloat();

    // bool  m_needsUpdate;

    real_t m_fPeakValue_L;
    real_t m_fPeakValue_R;
    real_t m_persistentPeak_L;
    real_t m_persistentPeak_R;
    real_t m_fMinPeak;
    real_t m_fMaxPeak;

    QTime m_lastPeakTime_L;
    QTime m_lastPeakTime_R;

    QPixmap m_back;
    QPixmap m_leds;
    QPixmap m_knob;

    bool m_displayConversion;
    bool m_levelsDisplayedInDBFS;

    int    m_moveStartPoint;
    real_t m_startValue;

    static TextFloat* s_textFloat;

    QColor m_peakGreen;
    QColor m_peakRed;
    QColor m_peakYellow;
};

#endif
