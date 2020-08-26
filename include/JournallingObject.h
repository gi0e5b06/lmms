/*
 * JournallingObject.h - declaration of class JournallingObject
 *
 * Copyright (c) 2006-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef JOURNALLING_OBJECT_H
#define JOURNALLING_OBJECT_H

#include "SerializingObject.h"
#include "lmms_basics.h"

#include <QStack>

class EXPORT JournallingObject : public SerializingObject
{
  public:
    JournallingObject();
    virtual ~JournallingObject();

    virtual jo_id_t id() const final
    {
        return m_id;
    }

    virtual void saveJournallingState(const bool newState) final
    {
        m_journallingStateStack.push(m_journalling);
        m_journalling = newState;
    }

    virtual void restoreJournallingState() final
    {
        if(!isJournallingStateStackEmpty())
        {
            m_journalling = m_journallingStateStack.pop();
        }
    }

    virtual void addJournalCheckPoint() final;

    virtual bool isJournalling() const final;

    virtual void setJournalling(const bool _sr) final
    {
        m_journalling = _sr;
    }

    virtual bool testAndSetJournalling(const bool newState) final
    {
        const bool oldJournalling = m_journalling;
        m_journalling             = newState;
        return oldJournalling;
    }

    virtual bool isJournallingStateStackEmpty() const final
    {
        return m_journallingStateStack.isEmpty();
    }

    virtual QDomElement saveState(QDomDocument& _doc,
                                  QDomElement&  _parent) override;

    virtual void restoreState(const QDomElement& _this) override;

  protected:
    virtual void changeID(jo_id_t _id) final;

  private:
    jo_id_t m_id;
    bool    m_journalling;

    QStack<bool> m_journallingStateStack;
};

#endif
