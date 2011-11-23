/*
* Copyright (C) 2011, Code Aurora Forum. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above
*       copyright notice, this list of conditions and the following
*       disclaimer in the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of Code Aurora Forum, Inc. nor the names of its
*       contributors may be used to endorse or promote products derived
*       from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include "SSCNode.h"

#include "CSSRule.h"
#include "CSSStyleSelector.h"
#include "Element.h"
#include "Logging.h"
#include "NotImplemented.h"

#include <wtf/CurrentTime.h>
#include <wtf/text/CString.h>

namespace WebCore {

using namespace HTMLNames;

SSCNode::SSCNode(const QualifiedName& tagName, const AtomicString& className, const AtomicString& id, PassRefPtr<RenderStyle> style, PassRefPtr<CSSRuleList> ruleList)
: m_tagName(tagName),
  m_className(className),
  m_id(id)
{
    ASSERT(!tagName.toString().isEmpty());
    ASSERT(style);

    m_style = style;

    RefPtr<CSSRuleList> list = ruleList;
    if (list)
        m_ruleLists.append(list);
}

PassRefPtr<SSCNode> SSCNode::create(const Element* element, PassRefPtr<RenderStyle> style, PassRefPtr<CSSRuleList> ruleList)
{
    const QualifiedName& tagName = element->tagQName();
    const AtomicString& className = element->getAttribute(HTMLNames::classAttr);
    const AtomicString& id = element->getIdAttribute();

    ASSERT(!tagName.toString().isEmpty());
    ASSERT(style);
    return adoptRef(new SSCNode(tagName, className, id, style, ruleList));
}

SSCNode::~SSCNode()
{
}

inline bool SSCNode::attributesMatch(Element* element, PassRefPtr<CSSRuleList> ruleList, const QualifiedName& tagName, const AtomicString& className, const AtomicString& id)
{
    ASSERT(element);
    ASSERT(element->document());
    ASSERT(element->document()->hasStyleSelector());

    if ((tagName == bodyTag) && (m_tagName == tagName)) {
        if (m_className != className)
            return false;

        if (m_id != id)
            return false;

        if (!ruleList)
            return true;

        if (ruleList && !m_ruleLists.size()) {
            m_style = 0;
            return true;
        }

        bool found = false;
        for (int ix = m_ruleLists.size()-1; ix >= 0; ix--) {
            RefPtr<CSSRuleList> ruleList1 = m_ruleLists[ix];
            if (!ruleList1)
                continue;
            if (*(ruleList) <= *(ruleList1)) {
                found = true;
                break;
            }
        }

        if (!found)
            m_style = 0;
        return true;
    }

    if (m_tagName != tagName)
        return false;

    if (element->hasClass() && (m_className != className))
        return false;

    if (element->hasID() && m_id != id)
        return false;

    if (!ruleList)
        return true;

    if (ruleList && !m_ruleLists.size()) {
        m_style = 0;
        return true;
    }

    bool found = false;
    for (int ix = m_ruleLists.size()-1; ix >= 0; ix--) {
        RefPtr<CSSRuleList> ruleList1 = m_ruleLists[ix];
        if (!ruleList1)
            continue;
        if (*(ruleList) <= *(ruleList1)) {
            found = true;
            break;
        }
    }

    if (!found)
        m_style = 0;

    return true;
}

PassRefPtr<SSCNode> SSCNode::attributesMatchChildren(Element* element, PassRefPtr<CSSRuleList> ruleList) const
{
    RefPtr<CSSRuleList> ruleList1 = ruleList;
    const QualifiedName& tagName = element->tagQName();
    const AtomicString& className = element->getAttribute(HTMLNames::classAttr);
    const AtomicString& id = element->getIdAttribute();

    for (size_t i = 0; i < m_children.size(); i++) {
        RefPtr<SSCNode> sscNode = m_children[i];
        if (sscNode->attributesMatch(element, ruleList1, tagName, className, id))
            return sscNode;
    }
    return 0;
}

void SSCNode::appendChild(PassRefPtr<SSCNode> newChild)
{
    ASSERT(newChild);
    ASSERT(!newChild->parent());
    RefPtr<SSCNode> child = newChild;
    child->setParent(this);
    m_children.append(child);
}

void SSCNode::removeChild(PassRefPtr<SSCNode> child)
{
    ASSERT(child);
    RefPtr<SSCNode> childNode = child;
    size_t index = m_children.find(childNode);
    ASSERT(index);
    m_children.remove(index);
}

void SSCNode::setStyle(PassRefPtr<RenderStyle> style)
{
    ASSERT(style);
    RefPtr<RenderStyle> renderStyle = style;
    m_style = renderStyle;

}

void SSCNode::appendRuleList(PassRefPtr<CSSRuleList> ruleList)
{
    ASSERT(ruleList);
    RefPtr<CSSRuleList> cssRuleList = ruleList;
    m_ruleLists.append(cssRuleList);
}

} // namespace WebCore
