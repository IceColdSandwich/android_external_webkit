/*
 * (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * (C) 2002-2003 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2002, 2006, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2011, Code Aurora Forum, All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef CSSFontFaceRule_h
#define CSSFontFaceRule_h

#include "CSSMutableStyleDeclaration.h"
#include "CSSRule.h"
#include <wtf/PassRefPtr.h>
#include <wtf/RefPtr.h>

namespace WebCore {

class CSSMutableStyleDeclaration;

class CSSFontFaceRule : public CSSRule {
public:
    static PassRefPtr<CSSFontFaceRule> create()
    {
        return adoptRef(new CSSFontFaceRule(0));
    }
    static PassRefPtr<CSSFontFaceRule> create(CSSStyleSheet* parent)
    {
        return adoptRef(new CSSFontFaceRule(parent));
    }

    virtual ~CSSFontFaceRule();

    CSSMutableStyleDeclaration* style() const { return m_style.get(); }

    virtual String cssText() const;

    void setDeclaration(PassRefPtr<CSSMutableStyleDeclaration>);

    virtual void addSubresourceStyleURLs(ListHashSet<KURL>& urls);

    virtual bool operator==(const CSSRule& o);

private:
    CSSFontFaceRule(CSSStyleSheet* parent);

    virtual bool isFontFaceRule() { return true; }

    // Inherited from CSSRule
    virtual unsigned short type() const { return FONT_FACE_RULE; }

    RefPtr<CSSMutableStyleDeclaration> m_style;
};

inline bool CSSFontFaceRule::operator==(const CSSRule& o)
{
    if (type() != o.type())
        return false;

    const CSSFontFaceRule* rule = static_cast<const CSSFontFaceRule*>(&o);

    if (m_style && rule->m_style) {
        CSSMutableStyleDeclarationConstIterator c1 = m_style->begin();
        CSSMutableStyleDeclarationConstIterator c2 = rule->m_style->begin();
        while (c1 != m_style->end()) {
            bool found = false;
            while (c2 != rule->m_style->end()) {
                if (*c1 == *c2) {
                    found = true;
                    break;
                }
                ++c2;
            }
            if(!found)
                return false;
            ++c1;
        }
    }

    return true;
}


} // namespace WebCore

#endif // CSSFontFaceRule_h
