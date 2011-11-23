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

#ifndef StyleCacheManager_h
#define StyleCacheManager_h

#include <wtf/HashMap.h>
#include <wtf/Noncopyable.h>
#include <wtf/text/StringHash.h>

namespace WebCore {

class SSCNode;
class Element;

class StyleCacheManager {
    WTF_MAKE_NONCOPYABLE(StyleCacheManager); WTF_MAKE_FAST_ALLOCATED;
    public:
        friend StyleCacheManager* styleCache();

        void add(const String&, PassRefPtr<SSCNode>, bool pFrame); // Prunes if capacity() is exceeded.
        void remove(const String&);

        PassRefPtr<SSCNode> getSSCTree(const String& url) const;

        int styleCacheCount() const { return m_size; }

        void clear();

    private:
        typedef HashMap<String, RefPtr<SSCNode> > StyleCacheMap;
        StyleCacheMap m_styleCache;

        StyleCacheManager();
        ~StyleCacheManager(); // Not implemented to make sure nobody accidentally calls delete -- WebCore does not delete singletons.

        void prune();

        int m_size;

     };

// Function to obtain the global style cache.
StyleCacheManager* styleCache();

} // namespace WebCore

#endif // StyleCache_h
