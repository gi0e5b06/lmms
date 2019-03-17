/*
 * Format12Export.h -
 *
 * Copyright (c) 2019 gi0e5b06 (on github.com)
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

#ifndef _FORMAT12_H
#define _FORMAT12_H

#include "ExportFilter.h"

#include <QString>

class Format12Export : public ExportFilter
{
  public:
    Format12Export();
    ~Format12Export();

    virtual PluginView* instantiateView(QWidget*)
    {
        return nullptr;
    }

    virtual bool proceed(const QString& _fileName);

  protected:
    void removeTag(QDomElement& _root, const QString& _tagName);
    void renameTag(QDomElement&   _root,
                   const QString& _oldName,
                   const QString& _newName);
    void removeAttribute(QDomElement&   _root,
                         const QString& _tagName,
                         const QString& _attrName);
    void renameAttribute(QDomElement&   _root,
                         const QString& _tagName,
                         const QString& _attrName,
                         const QString& _newName);

  private:
    void error();
};

#endif
