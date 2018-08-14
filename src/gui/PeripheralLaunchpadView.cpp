/*
 * PeripheralLaunchpadView.cpp - implementation of an interactive launchpad
 * widget (like the APCmini) used in the instrument track window for testing +
 * according model class
 *
 * Copyright (c) 2018 gi0e5b06 (on github.com)
 * Copyright (c) 2004-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

/** \brief A launchpad to play notes on in the instrument plugin window.
 */

/*
 * \mainpage Instrument plugin keyboard display classes
 *
 * \section introduction Introduction
 *
 * \todo fill this out
 * \todo write isWhite inline function and replace throughout
 */

#include "PeripheralLaunchpadView.h"

//#include <QCursor>
#include <QKeyEvent>
#include <QPainter>
#include <QVBoxLayout>

//#include "CaptionMenu.h"
#include "InstrumentTrack.h"
//#include "Knob.h"
//#include "MainWindow.h"
#include "Piano.h"
#include "StringPairDrag.h"

//#include "embed.h"
//#include "gui_templates.h"

#include <cmath>

// QPixmap * PeripheralLaunchpadView::s_padPm = NULL;           /*!< A pad
// released
// */ QPixmap * PeripheralLaunchpadView::s_padPressedPm = NULL;    /*!< A pad
// pressed */

// const int PIANO_BASE = 11;          /*!< The height of the root note
// display */ const int PW_WHITE_KEY_WIDTH = 10;  /*!< The width of a white
// key
// */ const int PW_WHITE_KEY_HEIGHT = 57; /*!< The height of a white key */
// const int LABEL_TEXT_SIZE = 7;      /*!< The height of the key label text
// */

const int PW_PAD_WIDTH  = 12;
const int PW_PAD_HEIGHT = 12;
const int PW_GAP_WIDTH  = 1;
const int PW_GAP_HEIGHT = 1;

/*! \brief Create a new keyboard display view
 *
 *  \param _parent the parent instrument plugin window
 *  \todo are the descriptions of the m_startkey and m_lastkey properties
 * correct?
 */
PeripheralLaunchpadView::PeripheralLaunchpadView(QWidget* _parent) :
      PeripheralView(_parent), /*!< Our parent */
      //m_startKey(0),
      m_lastKey(-1)
{
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setFocusPolicy(Qt::StrongFocus);
    setMaximumWidth(250);  // NumKeys * PW_WHITE_KEY_WIDTH );

    /*
    // create scrollbar at the bottom
    m_pianoScroll = new QScrollBar(Qt::Horizontal, this);
    m_pianoScroll->setSingleStep(16);
    m_pianoScroll->setPageStep(16);
    m_pianoScroll->setValue(36);  // Octave_3 * WhiteKeysPerOctave );

    // and connect it to this widget
    connect(m_pianoScroll, SIGNAL(valueChanged(int)), this,
            SLOT(pianoScrolled(int)));

    // create a layout for ourselves
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addSpacing(68);  // PIANO_BASE+PW_WHITE_KEY_HEIGHT );
    layout->addWidget(m_pianoScroll);
    */
}

/*! \brief Destroy this piano display view
 *
 */
PeripheralLaunchpadView::~PeripheralLaunchpadView()
{
}

/*! \brief Register a change to this piano display view
 *
 */
/*
void PeripheralLaunchpadView::modelChanged()
{
        m_piano = castModel<Piano>();
        if( m_piano != NULL )
        {
                connect( m_piano->instrumentTrack()->baseNoteModel(), SIGNAL(
dataChanged() ), this, SLOT( update() ) );
        }

}
*/

// gets the key from the given mouse-position
/*! \brief Get the key from the mouse position in the piano display
 *
 *  First we determine it roughly by the position of the point given in
 *  white key widths from our start.  We then add in any black keys that
 *  might have been skipped over (they take a key number, but no 'white
 *  key' space).  We then add in our starting key number.
 *
 *  We then determine whether it was a black key that was pressed by
 *  checking whether it was within the vertical range of black keys.
 *  Black keys sit exactly between white keys on this keyboard, so
 *  we then shift the note down or up if we were in the left or right
 *  half of the white note.  We only do this, of course, if the white
 *  note has a black key on that side, so to speak.
 *
 *  This function returns const because there is a linear mapping from
 *  the point given to the key returned that never changes.
 *
 *  \param _p The point that the mouse was pressed.
 */
int PeripheralLaunchpadView::getKeyFromMouse(const QPoint& _p) const
{
    for(int k = 0; k < NumKeys; ++k)
        if(getPadRect(k).contains(_p))
            return k;
    return -100;
}

// handler for scrolling-event
/*! \brief Handle the scrolling on the piano display view
 *
 *  We need to update our start key position based on the new position.
 *
 *  \param _new_pos the new key position.
 */
/*
void PeripheralLaunchpadView::pianoScrolled(int _newPos)
{
    // m_startKey = WhiteKeys[_new_pos % WhiteKeysPerOctave]+
    //		( _new_pos / WhiteKeysPerOctave ) * KeysPerOctave;
    // m_startKey=qBound(0,m_startKey,NumKeys-1);

      m_startKey = qBound(4, (_newPos / 16) * 16 + 4, 84);
      qInfo("PeripheralLaunchpadView::pianoScrolled np=%d sk=%d", _newPos,
      m_startKey);
      update();
}
*/

/*! \brief Handle a context menu selection on the piano display view
 *
 *  \param _me the ContextMenuEvent to handle.
 *  \todo Is this right, or does this create the context menu?
 */
void PeripheralLaunchpadView::contextMenuEvent(QContextMenuEvent* _me)
{
    {
        PeripheralView::contextMenuEvent(_me);
    }
}

// handler for mouse-click-event
/*! \brief Handle a mouse click on this piano display view
 *
 *  We first determine the key number using the getKeyFromMouse() method.
 *
 *  If we're below the 'root key selection' area,
 *  we set the volume of the note to be proportional to the vertical
 *  position on the keyboard - lower down the key is louder, within the
 *  boundaries of the (white or black) key pressed.  We then tell the
 *  instrument to play that note, scaling for MIDI max loudness = 127.
 *
 *  If we're in the 'root key selection' area, of course, we set the
 *  root key to be that key.
 *
 *  We finally update ourselves to show the key press
 *
 *  \param _me the mouse click to handle.
 */
void PeripheralLaunchpadView::mousePressEvent(QMouseEvent* _me)
{
    if(_me->button() == Qt::LeftButton)
    {
        // get pressed key
        int keyNum = getKeyFromMouse(_me->pos());
        if(keyNum >= 0)
        {
            if(_me->modifiers() & Qt::ShiftModifier)
            {
                m_piano->instrumentTrack()->baseNoteModel()->setInitValue(
                        (float)keyNum);

                emit baseNoteChanged();
                update();
            }
            else if(_me->modifiers() & Qt::ControlModifier)
            {
                new StringPairDrag("automatable_model",
                                   QString::number(m_piano->instrumentTrack()
                                                           ->baseNoteModel()
                                                           ->id()),
                                   QPixmap(), this);
                _me->accept();
            }
            else if(m_piano != NULL)
            {
                int velocity = m_piano->instrumentTrack()
                                       ->midiPort()
                                       ->baseVelocity();
                // set note on
                m_piano->midiEventProcessor()->processInEvent(
                        MidiEvent(MidiNoteOn, -1, keyNum, velocity));
                m_piano->setKeyState(keyNum, true);
                m_lastKey = keyNum;

                emit keyPressed(keyNum);
            }
        }
    }
}

// handler for mouse-release-event
/*! \brief Handle a mouse release event on the piano display view
 *
 *  If a key was pressed by the in the mousePressEvent() function, we
 *  turn the note off.
 *
 *  \param _me the mousePressEvent to handle.
 */
void PeripheralLaunchpadView::mouseReleaseEvent(QMouseEvent*)
{
    if(m_lastKey != -1)
    {
        if(m_piano != NULL)
        {
            m_piano->midiEventProcessor()->processInEvent(
                    MidiEvent(MidiNoteOff, -1, m_lastKey, 0));
            m_piano->setKeyState(m_lastKey, false);
        }

        // and let the user see that he released a key... :)
        update();

        m_lastKey = -1;
    }
}

// handler for mouse-move-event
/*! \brief Handle a mouse move event on the piano display view
 *
 *  This handles the user dragging the mouse across the keys.  It uses
 *  code from mousePressEvent() and mouseReleaseEvent(), also correcting
 *  for if the mouse movement has stayed within one key and if the mouse
 *  has moved outside the vertical area of the keyboard (which is still
 *  allowed but won't make the volume go up to 11).
 *
 *  \param _me the ContextMenuEvent to handle.
 *  \todo Paul Wayper thinks that this code should be refactored to
 *  reduce or remove the duplication between this, the mousePressEvent()
 *  and mouseReleaseEvent() methods.
 */
void PeripheralLaunchpadView::mouseMoveEvent(QMouseEvent* _me)
{
    if(m_piano == NULL)
    {
        return;
    }

    /*
    int keyNum = getKeyFromMouse( _me->pos() );
    int y_diff = _me->pos().y() - PIANO_BASE;
    int velocity = (int)( (float) y_diff /
            ( Piano::isWhiteKey( keyNum ) ?
                    PW_WHITE_KEY_HEIGHT : PW_BLACK_KEY_HEIGHT ) *
                                            (float)
    m_piano->instrumentTrack()->midiPort()->baseVelocity() );
    // maybe the user moved the mouse-cursor above or under the
    // piano-widget while holding left button so check that and
    // correct volume if necessary
    if( y_diff < 0 )
    {
            velocity = 0;
    }
    else if( y_diff >
            ( Piano::isWhiteKey( keyNum ) ?
                            PW_WHITE_KEY_HEIGHT : PW_BLACK_KEY_HEIGHT ) )
    {
            velocity = m_piano->instrumentTrack()->midiPort()->baseVelocity();
    }

    // is the calculated key different from current key? (could be the
    // user just moved the cursor one pixel left but on the same key)
    if( keyNum != m_lastKey )
    {
            if( m_lastKey != -1 )
            {
                    m_piano->midiEventProcessor()->processInEvent( MidiEvent(
    MidiNoteOff, -1, m_lastKey, 0 ) ); m_piano->setKeyState( m_lastKey, false
    ); m_lastKey = -1;
            }
            if( _me->buttons() & Qt::LeftButton )
            {
                    if( _me->pos().y() > PIANO_BASE )
                    {
                            m_piano->midiEventProcessor()->processInEvent(
    MidiEvent( MidiNoteOn, -1, keyNum, velocity ) ); m_piano->setKeyState(
    keyNum, true ); m_lastKey = keyNum;
                    }
                    else
                    {
                            m_piano->instrumentTrack()->baseNoteModel()->setInitValue(
    (float) keyNum );
                    }
            }
            // and let the user see that he pressed a key... :)
            update();
    }
    else if( m_piano->isKeyPressed( keyNum ) )
    {
            m_piano->midiEventProcessor()->processInEvent( MidiEvent(
    MidiKeyPressure, -1, keyNum, velocity ) );
    }
    */
}

/*! \brief Handle a key press event on the piano display view
 *
 *  We determine our key number from the getKeyFromKeyEvent() method,
 *  and pass the event on to the piano's handleKeyPress() method if
 *  auto-repeat is off.
 *
 *  \param _ke the KeyEvent to handle.
 */
void PeripheralLaunchpadView::keyPressEvent(QKeyEvent* _ke)
{
    const int keyNum
            = getKeyFromKeyEvent(_ke) + (DefaultOctave - 1) * KeysPerOctave;

    if(_ke->isAutoRepeat() == false && keyNum > -1)
    {
        if(m_piano != NULL)
        {
            m_piano->handleKeyPress(keyNum);
            _ke->accept();
            update();
        }
    }
    else
    {
        _ke->ignore();
    }
}

/*! \brief Handle a key release event on the piano display view
 *
 *  The same logic as the keyPressEvent() method.
 *
 *  \param _ke the KeyEvent to handle.
 */
void PeripheralLaunchpadView::keyReleaseEvent(QKeyEvent* _ke)
{
    const int keyNum
            = getKeyFromKeyEvent(_ke) + (DefaultOctave - 1) * KeysPerOctave;
    if(_ke->isAutoRepeat() == false && keyNum > -1)
    {
        if(m_piano != NULL)
        {
            m_piano->handleKeyRelease(keyNum);
            _ke->accept();
            update();
        }
    }
    else
    {
        _ke->ignore();
    }
}

/*! \brief Handle the focus leaving the piano display view
 *
 *  Turn off all notes if we lose focus.
 *
 *  \todo Is there supposed to be a parameter given here?
 */
void PeripheralLaunchpadView::focusOutEvent(QFocusEvent*)
{
    if(m_piano == NULL)
    {
        return;
    }

    // focus just switched to another control inside the instrument track
    // window we live in?
    if(parentWidget()->parentWidget()->focusWidget() != this
       && parentWidget()->parentWidget()->focusWidget() != NULL
       && !(parentWidget()->parentWidget()->focusWidget()->inherits(
                    "QLineEdit")
            || parentWidget()->parentWidget()->focusWidget()->inherits(
                       "QPlainTextEdit")))
    {
        // then reclaim keyboard focus!
        setFocus();
        return;
    }

    // if we loose focus, we HAVE to note off all running notes because
    // we don't receive key-release-events anymore and so the notes would
    // hang otherwise
    for(int i = 0; i < NumKeys; ++i)
    {
        m_piano->midiEventProcessor()->processInEvent(
                MidiEvent(MidiNoteOff, -1, i, 0));
        m_piano->setKeyState(i, false);
    }
    update();
}

/*! \brief update scrollbar range after resize
 *
 *  After resizing we need to adjust range of scrollbar for not allowing
 *  to scroll too far to the right.
 *
 *  \param _event resize-event object (unused)
 */
void PeripheralLaunchpadView::resizeEvent(QResizeEvent* _event)
{
    QWidget::resizeEvent(_event);
    /*
    // 75 (number of white keys) should be computed.
    m_pianoScroll->setRange( 0,
    (int)ceil((75*PW_WHITE_KEY_WIDTH-width())/(float)PW_WHITE_KEY_WIDTH));
    */
}

/*! \brief Paint the piano display view in response to an event
 *
 *  This method draws the piano and the 'root note' base.  It draws
 *  the base first, then all the white keys, then all the black keys.
 *
 *  \todo Is there supposed to be a parameter given here?
 */
void PeripheralLaunchpadView::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    // p.fillRect( QRect( 0, 0, width(), height() ), Qt::red );
    p.fillRect(QRect(0, 0, width(), height()), p.background());
    p.setPen(Qt::white);

    const int baseKey
            = (m_piano != NULL)
                      ? m_piano->instrumentTrack()->baseNoteModel()->value()
                      : 0;

    for(int k = 0; k < 64; k++)
    {
        QColor padfg(255, 255, 255);
        QColor padbg(40, 40, 40);
        QColor padbd(0, 0, 0);
        if(m_piano && m_piano->isKeyPressed(k))
        {
            padbd = QColor(0, 255, 0);
            padbg = QColor(128, 255, 128);
            // qInfo("PPV::paintEvent k=%d pressed",k);
        }

        QRect r = getPadRect(k);
        p.fillRect(r, padbg);

        if(k == baseKey)
            p.fillRect(r.adjusted(+1, +1, -1, -1), Qt::white);
    }
}

QRect PeripheralLaunchpadView::getPadRect(int _k) const
{
    if((_k < 0) || (_k > 63))
        return QRect();

    const int padi = _k % 8;
    const int padj = 7 - (_k / 8);

    int x = 45 + padi * 20;
    int y = 1 + padj * 10;
    int w = 18;
    int h = 8;
    return QRect(x, y, w, h);
}
