/*
 * Copyright (c) 2011, Code Aurora Forum. All rights reserved.
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

#ifndef FeaturePermissions_h
#define FeaturePermissions_h

#include "PlatformString.h"
// for String hash types instantiated there.
#include <wtf/text/StringHash.h>
#include <wtf/RefCounted.h>
#include <wtf/Vector.h>


namespace WebCore {
    class Frame;
}

namespace android {

    class WebViewCore;
    class FeaturePermissions : public RefCounted<FeaturePermissions> {
        public:
            FeaturePermissions(WebViewCore*, WebCore::Frame*);
            void handleRequestPermission(WebCore::Frame*, Vector<String>&, void*, void (*)(void*, bool));
            void handleRequestPermissionResponse(const Vector<String>&, String, bool, bool);
            void resetTemporaryPermissionStates();

        private:
            bool areListsEqual(const Vector<String>&, const Vector<String>&) const;
            void maybeCallbackFrames(String&, Vector<void(*)(void*,bool)>&, Vector<void *>&, bool);
            WebViewCore* m_webViewCore;
            WebCore::Frame* m_mainFrame;
            struct PermissionRequest {
                String appid;
                Vector<String> features;
                Vector<void *> contexts;
                Vector<void(*)(void*,bool)>  callbacks;
            };
            typedef Vector<PermissionRequest> PermissionRequestVector;
            PermissionRequestVector m_queuedPermissionRequests;
    };
}
#endif
