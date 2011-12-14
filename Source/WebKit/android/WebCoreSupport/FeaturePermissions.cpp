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

#include "config.h"
#include "FeaturePermissions.h"
#include "Frame.h"
#include "WebViewCore.h"
#include "android/log.h"

namespace android {

FeaturePermissions::FeaturePermissions(WebViewCore* webViewCore, Frame* mainFrame)
    : m_webViewCore(webViewCore)
    , m_mainFrame(mainFrame)
{
    ASSERT(m_webViewCore);
}

void FeaturePermissions::handleRequestPermission(Frame* frame, Vector<String>& features, void* context, void (*callback)(void*, bool))
{
    String appid = frame->document()->url().string();
    Vector<void *> tmpContext;
    tmpContext.append(context);
    Vector<void (*)(void*, bool)> tmpCallbacks;
    tmpCallbacks.append(callback);

    PermissionRequest tmpPermReq = {appid, features, tmpContext, tmpCallbacks};
    if (m_queuedPermissionRequests.isEmpty()) {
        m_queuedPermissionRequests.append(tmpPermReq);
        m_webViewCore->navigatorPermissionsShowPrompt(features,appid);
        return;
    } else {
        unsigned int i=0;
        for(; i<m_queuedPermissionRequests.size(); i++) {
            if((m_queuedPermissionRequests[i].appid == appid) && (areListsEqual(m_queuedPermissionRequests[i].features, features))) {
                m_queuedPermissionRequests[i].callbacks.append(callback);
                m_queuedPermissionRequests[i].contexts.append(context);
                break;
            }
        }
        if(i == m_queuedPermissionRequests.size())
            m_queuedPermissionRequests.append(tmpPermReq);
    }
}

void FeaturePermissions::handleRequestPermissionResponse(const Vector<String>& features, String appid, bool permission, bool remember)
{
    if(!m_queuedPermissionRequests.isEmpty()) {
        if (appid != m_queuedPermissionRequests[0].appid)
            return; // We should do something better here
        if(!areListsEqual(m_queuedPermissionRequests[0].features, features)) {
           return;
        }
    }

    maybeCallbackFrames(appid, m_queuedPermissionRequests[0].callbacks, m_queuedPermissionRequests[0].contexts, permission);
    ASSERT(!m_queuedPermissionRequests.isEmpty());
    m_queuedPermissionRequests.remove(0);

    // If there are other requests queued, start the next one.
    if (!m_queuedPermissionRequests.isEmpty()) {
        m_webViewCore->navigatorPermissionsShowPrompt( m_queuedPermissionRequests[0].features,
                                                       m_queuedPermissionRequests[0].appid);
    }
}

void FeaturePermissions::maybeCallbackFrames(String& appid, Vector<void(*)(void*,bool)>& callbacks, Vector<void *>& contexts, bool permission)
{
    for (Frame* frame = m_mainFrame; frame; frame = frame->tree()->traverseNext()) {
        if (appid == frame->document()->url().string()) {
            for(unsigned int i=0; i<callbacks.size(); i++) {
                callbacks[i](contexts[i],permission);
            }
        }
    }
}

void FeaturePermissions::resetTemporaryPermissionStates()
{
    m_webViewCore->navigatorPermissionsHidePrompt();
}

bool FeaturePermissions::areListsEqual(const Vector<String>& list1, const Vector<String>& list2) const
{
    if(list1.size() != list2.size()) {
        return false;
    }

    for(unsigned int i=0; i<list1.size(); i++) {
        if(list2.find(list1[i]) == WTF::notFound) {
            return false;
        }
    }
    return true;
}

} //namespace
