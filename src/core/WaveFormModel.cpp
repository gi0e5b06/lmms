/*
 * WaveFormModel.cpp -
 *
 * Copyright (c) 2018-2019 gi0e5b06 (on github.com)
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

#include "WaveFormModel.h"

WaveFormModel::WaveFormModel(Model*         _parent,
                             const QString& _displayName,
                             const QString& _objectName,
                             bool           _defaultConstructed) :
      Model(_parent, _displayName, _objectName, _defaultConstructed),
      m_wf(WaveFormStandard::SINE)
{
}

WaveFormModel::~WaveFormModel()
{
}

void WaveFormModel::saveSettings(QDomDocument&  doc,
                                 QDomElement&   element,
                                 const QString& name,
                                 const bool     unique)
{
    // if(unique) test

    QDomElement e = doc.createElement(nodeName());
    element.appendChild(e);
    e.setAttribute("name", name);

    const WaveFormStandard* wfs = dynamic_cast<const WaveFormStandard*>(m_wf);
    if(wfs)
    {
        e.setAttribute("type", "standard");
        e.setAttribute("bank", wfs->bank());
        e.setAttribute("index", wfs->index());
    }
}

void WaveFormModel::loadSettings(const QDomElement& element,
                                 const QString&     name,
                                 const bool         required)
{
    const WaveFormStandard* r = nullptr;

    QDomNode node = element.firstChild();
    while(!node.isNull())
    {
        if(node.isElement())
        {
            if(node.nodeName() == nodeName())
            {
                QDomElement e = node.toElement();
                if(e.attribute("name") == name)
                {
                    QString type = e.attribute("type");
                    if(type == "standard")
                    {
                        int bank  = e.attribute("bank").toInt();
                        int index = e.attribute("index").toInt();
                        r         = WaveFormStandard::get(bank, index);
                        break;
                    }
                }
            }
        }
        node = node.nextSibling();
    }

    if(r != nullptr)
        setValue(r);
    // else if(required) error
}

const WaveFormStandard* WaveFormModel::value() const
{
    return m_wf;
}

void WaveFormModel::setValue(const WaveFormStandard* _wf)
{
    const WaveFormStandard* wf
            = (_wf == nullptr ? WaveFormStandard::SINE : _wf);
    if(wf != m_wf)
    {
        m_wf = wf;
        emit dataChanged();
    }
}

// TODO: mv up
QString WaveFormModel::formatNumber(real_t v)
{
#ifdef REAL_IS_FLOAT
    QString r = QString::number(v, 'f', 9);
#endif
#ifdef REAL_IS_DOUBLE
    QString r = QString::number(v, 'f', 17);
#endif
    if(r.indexOf(QChar('.')) >= 0)
    {
        r.replace(QRegExp("0*$"), "");
        r.replace(QRegExp("[.]$"), "");
    }
    return r;
}
