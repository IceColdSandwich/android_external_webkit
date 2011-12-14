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
#ifndef NODE_PROXY_H
#define NODE_PROXY_H

#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>

#include "Frame.h"
#include <node.h>

namespace WebCore {
  // NodeProxy is the webcore side object for a node instance
  // its a 1-1 mapping to node instance in that page (one per context/frame)
  // Its owned by the DOMWindow and gets destroyed when the DOMWindow is detached
  // from frame (atleast currently)
  class NodeProxy: public RefCounted<NodeProxy>, public node::NodeClient {
    public:
      static PassRefPtr<NodeProxy> create(Frame *frame){
        return adoptRef(new NodeProxy(frame));
      }

      // releases node resources
      ~NodeProxy();

      // handle app focus in/out events
      void pause();
      void resume();

      // called by the webkit engine when the frame is detached from window
      // we release node resources here
      void disconnectFrame();

      // this the app data path that will be used as the root 
      // for installing and looking up node modules
      static void setAppDataPath(const String&);

      // NodeClient interface, handles events from node
      void HandleNodeEvent(node::NodeEvent *ev);
      void OnDelete();
      void OnTestDone();
      std::string url();

      // accessors
      Frame *frame() const { return m_frame; }
      node::Node *node() const { return m_node; }

    private:
      void handleNodeEventEvThread(node::NodeEvent* e);

      // This needs to be static since javashared client expects a C pointer
      static void handleNodeEventMainThread(void* data);
      void handleRegisterPrivilegedFeaturesEvent(node::NodeEvent *ev);
      void handleCameraPreviewFrameEvent(node::NodeEvent *ev);
      void handleRequestPermission(node::NodeEvent *ev);

      NodeProxy(Frame*);
      Frame *m_frame;
      node::Node *m_node;
      static String* s_moduleRootPath;
      static bool s_nodeInitialized;
  };
}

#endif

