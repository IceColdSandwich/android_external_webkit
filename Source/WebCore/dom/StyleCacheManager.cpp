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
#include "StyleCacheManager.h"

#include "SSCNode.h"

static const int s_maxCapacity = 30;

namespace WebCore {

StyleCacheManager* styleCache()
{
    static StyleCacheManager* staticStyleCache = new StyleCacheManager;
    return staticStyleCache;
}

StyleCacheManager::StyleCacheManager()
    : m_size(0)
{
}

void StyleCacheManager::add(const String& url, PassRefPtr<SSCNode> sscNode, bool pFrame)
{
    ASSERT(!url.isEmpty());
    ASSERT(sscNode);
    prune();
    pair<StyleCacheMap::iterator, bool> result = m_styleCache.set(url, sscNode.get());
    if((result.second)&& pFrame)
        ++m_size;
}

void StyleCacheManager::remove(const String& url)
{
    StyleCacheMap::iterator it = m_styleCache.find(url);
    if (it == m_styleCache.end())
        return;

    m_styleCache.remove(url);
    --m_size;
}

void StyleCacheManager::prune()
{
    if (m_size > s_maxCapacity) {
        StyleCacheMap::iterator end = m_styleCache.end();
        for (StyleCacheMap::iterator it = m_styleCache.begin(); it != end; ++it) {
            if (it->second->refCount() == 1) {
                m_styleCache.remove(it);
                m_size--;
                break;
            }
        }
    }

}

PassRefPtr<SSCNode> StyleCacheManager::getSSCTree(const String& url) const
{
    StyleCacheMap::const_iterator end = m_styleCache.end();
    for (StyleCacheMap::const_iterator it = m_styleCache.begin(); it != end; ++it) {
        if (it->first == url)
            return it->second;
    }
    return 0;
}

void StyleCacheManager::clear()
{
    m_styleCache.clear();
    m_size = 0;
}

} // namespace WebCore
