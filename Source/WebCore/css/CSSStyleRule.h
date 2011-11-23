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

#ifndef CSSStyleRule_h
#define CSSStyleRule_h

#include "CSSMutableStyleDeclaration.h"
#include "CSSRule.h"
#include "CSSSelectorList.h"
#include <wtf/PassRefPtr.h>
#include <wtf/RefPtr.h>

namespace WebCore {

class CSSMutableStyleDeclaration;
class CSSSelector;

class CSSStyleRule : public CSSRule {
public:
    static PassRefPtr<CSSStyleRule> create(CSSStyleSheet* parent, int sourceLine)
    {
        return adoptRef(new CSSStyleRule(parent, sourceLine));
    }
    virtual ~CSSStyleRule();

    virtual String selectorText() const;
    void setSelectorText(const String&);

    CSSMutableStyleDeclaration* style() const { return m_style.get(); }

    virtual String cssText() const;

    // Not part of the CSSOM
    virtual bool parseString(const String&, bool = false);

    void adoptSelectorVector(Vector<OwnPtr<CSSParserSelector> >& selectors) { m_selectorList.adoptSelectorVector(selectors); }
    void setDeclaration(PassRefPtr<CSSMutableStyleDeclaration>);

    const CSSSelectorList& selectorList() const { return m_selectorList; }
    CSSMutableStyleDeclaration* declaration() { return m_style.get(); }

    virtual void addSubresourceStyleURLs(ListHashSet<KURL>& urls);

    int sourceLine() { return m_sourceLine; }

    virtual bool operator==(const CSSRule& o);

protected:
    CSSStyleRule(CSSStyleSheet* parent, int sourceLine);

private:
    virtual bool isStyleRule() { return true; }

    // Inherited from CSSRule
    virtual unsigned short type() const { return STYLE_RULE; }

    RefPtr<CSSMutableStyleDeclaration> m_style;
    CSSSelectorList m_selectorList;
    int m_sourceLine;
};

inline bool CSSStyleRule::operator==(const CSSRule& o)
{
    if (type() != o.type())
        return false;

    const CSSStyleRule* rule = static_cast<const CSSStyleRule*>(&o);

    if ((m_style && !rule->m_style) || (!m_style && rule->m_style) || (!(*(m_style) == *(rule->m_style))))
        return false;

    CSSSelector* s1 = selectorList().first();
    CSSSelector* s2 = rule->selectorList().first();

    if (s1 && !s2)
        return false;

    if (s2 && !s1)
        return false;

    while (s1 && s2) {
        if (!(*s1 == *s2))
            return false;
        s1 = CSSSelectorList::next(s1);
        s2 = CSSSelectorList::next(s2);
    }

    return true;
}

} // namespace WebCore

#endif // CSSStyleRule_h
