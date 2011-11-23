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

#ifndef SSCNode_h
#define SSCNode_h

#include "CSSRuleList.h"
#include "HTMLNames.h"
#include "QualifiedName.h"
#include "RenderStyle.h"
#include "TreeShared.h"
#include <wtf/text/AtomicString.h>

namespace WebCore {

class Element;

class SSCNode : public TreeShared<SSCNode> {
public:

    static PassRefPtr<SSCNode> create(const Element* element, PassRefPtr<RenderStyle> style, PassRefPtr<CSSRuleList> ruleList);

    ~SSCNode(void);

    void setRuleList(PassRefPtr<CSSRuleList> ruleList) { m_ruleList = ruleList; }

    const PassRefPtr<RenderStyle> style() const { return m_style; }
    PassRefPtr<CSSRuleList> ruleList() const { return m_ruleList; }

    const Vector<RefPtr<SSCNode> > children() const { return m_children; }

    bool attributesMatch(Element* element, PassRefPtr<CSSRuleList> ruleList, const QualifiedName& tagName, const AtomicString& className, const AtomicString& id);
    PassRefPtr<SSCNode> attributesMatchChildren(Element* element, PassRefPtr<CSSRuleList> ruleList) const;

    void appendChild(PassRefPtr<SSCNode> newChild);
    void removeChild(PassRefPtr<SSCNode> child);

    void setStyle(PassRefPtr<RenderStyle> style);
    void appendRuleList(PassRefPtr<CSSRuleList> ruleList);

private:

    SSCNode(const QualifiedName& tagName, const AtomicString& className, const AtomicString& id, PassRefPtr<RenderStyle> style, PassRefPtr<CSSRuleList> ruleList);

    QualifiedName m_tagName;
    AtomicString m_className;
    AtomicString m_id;
    RefPtr<RenderStyle> m_style;
    RefPtr<CSSRuleList> m_ruleList;
    // Vector of ref pointers, since we own the children
    // SSCNodes should be removed only when StyleCache Manager deletes the root SSCNode
    Vector<RefPtr<SSCNode> > m_children;
    Vector<RefPtr<CSSRuleList> > m_ruleLists;

};

}
#endif
