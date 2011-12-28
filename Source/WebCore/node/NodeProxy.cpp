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
#include "V8Proxy.h"
#include "JavaSharedClient.h"
#include "JNIUtility.h"
#include "MainThread.h"
#include "text/CString.h"
#include "HTMLVideoElement.h"
#include "V8HTMLVideoElement.h"
#include "Page.h"
#include "Chrome.h"
#include "WebViewCore.h"
#include "DOMWindow.h"
#include "Navigator.h"
#include "NodeProxy.h"
#include <vector>

namespace WebCore {
using namespace v8;
using WTF::String;

String* NodeProxy::s_moduleRootPath = 0;
bool NodeProxy::s_nodeInitialized = false;

void NodeProxy::handleRegisterPrivilegedFeaturesEvent(node::NodeEvent *ev) {
  NODE_ASSERT(ev->type == node::NODE_EVENT_FP_REGISTER_PRIVILEGED_FEATURES);
  node::RegisterPrivilegedFeaturesEvent *e = &ev->u.RegisterPrivilegedFeaturesEvent_;

  Vector<String> featurelist;
  Vector<std::string>::const_iterator filesEnd = e->features->end();

  for (Vector<std::string>::const_iterator fileIterator = e->features->begin(); fileIterator != filesEnd; ++fileIterator) {
    NODE_LOGI("feature vector data - %s", fileIterator->c_str());
    featurelist.append(fileIterator->c_str());
  }
}

void NodeProxy::handleRequestPermission(node::NodeEvent *ev) {
  NODE_ASSERT(ev->type == node::NODE_EVENT_FP_REQUEST_PERMISSION);
  node::RequestPermissionEvent *e = &ev->u.RequestPermissionEvent_;

  Vector<String> featurelist;
  Vector<std::string>::const_iterator filesEnd = e->features->end();

  for (Vector<std::string>::const_iterator fileIterator = e->features->begin(); fileIterator != filesEnd; ++fileIterator) {
    NODE_LOGI("feature vector data - %s", fileIterator->c_str());
    featurelist.append(fileIterator->c_str());
  }

  //String appid = m_frame->document()->url().string();
  if(m_frame->page()) {
      m_frame->page()->chrome()->handleRequestPermission(m_frame, featurelist, e->context, e->callback);
  }

  return;
}

NodeProxy::NodeProxy(Frame* frame)
  : m_frame(frame) {
  NODE_LOGF();
  NODE_ASSERT(frame);

  // Initialize the node event system on demand
  if (!s_nodeInitialized) {
    NODE_ASSERT(s_moduleRootPath);
    NODE_LOGI("%s, rootModulePath(%s)", __FUNCTION__, s_moduleRootPath->latin1().data());
    node::Node::Initialize(true, s_moduleRootPath->latin1().data());
    s_nodeInitialized = true;
  }

  // load the node instance
  NODE_LOGI("%s(tid=%d), frame(%p)", __FUNCTION__, gettid(), m_frame);
  m_node = new node::Node(this);
}

std::string NodeProxy::url() {
  NODE_ASSERT(m_frame);
  return m_frame->document()->url().string().latin1().data();
}

class NodeProxyTimer : public TimerBase {
  public:
    NodeProxyTimer(node::NodeEvent event) : m_event(event) {}

  private:
    virtual void fired();
    node::NodeEvent m_event;
};

void NodeProxyTimer::fired() {
  NODE_LOGM("%s(tid=%d)", __FUNCTION__, gettid());

  v8::HandleScope scope;
  if (m_event.type == node::NODE_EVENT_LIBEV_INVOKE_PENDING){
    if (node::Node::InvokePending()) {
      // atleast one node is done processing
      node::Node::CheckTestStatus(false);
    }
  } else if (m_event.type == node::NODE_EVENT_LIBEV_DONE) {
    node::Node::CheckTestStatus(true);
  } else {
    NODE_LOGE("%s, unexpected event type : %d", __PRETTY_FUNCTION__, m_event.type);
    NODE_ASSERT(0);
  }
  delete this;
}

// we lower the priority of the nodejs event handling by running
// it in a timer, this for e.g. allows the painting thread to run
void NodeProxy::handleNodeEventMainThread(void* data) {
  NODE_LOGM("%s", __FUNCTION__);
  NodeProxyTimer *timer = static_cast<NodeProxyTimer*>(data);
  timer->startOneShot(0.01); // 10ms
}

void NodeProxy::handleNodeEventEvThread(node::NodeEvent *ev) {
  NODE_LOGM("%s(tid=%d)", __FUNCTION__, gettid());
  NODE_ASSERT(ev->type ==  node::NODE_EVENT_LIBEV_DONE || ev->type == node::NODE_EVENT_LIBEV_INVOKE_PENDING);
  NodeProxyTimer *timer = new NodeProxyTimer(*ev);
  ::android::JavaSharedClient::EnqueueFunctionPtr(handleNodeEventMainThread, timer);
  NODE_LOGM("%s(tid=%d), enqueued event on to main thread", __FUNCTION__, gettid());
}

void NodeProxy::HandleNodeEvent(node::NodeEvent *ev) {
  // libev callbacks are invoked on the libev thread
  if (ev->type == node::NODE_EVENT_LIBEV_DONE || ev->type == node::NODE_EVENT_LIBEV_INVOKE_PENDING) {
    NODE_ASSERT(!isMainThread());
    return handleNodeEventEvThread(ev);
  }

  // All other callbacks should be called on the webcore thread
  NODE_LOGI("Node Bridge event type: %d", ev->type);
  NODE_ASSERT(isMainThread()); // should be called only on the main thread
  switch (ev->type) {
    case node::NODE_EVENT_FP_REGISTER_PRIVILEGED_FEATURES:
      handleRegisterPrivilegedFeaturesEvent(ev);
      break;

    case node::NODE_EVENT_FP_REQUEST_PERMISSION:
      handleRequestPermission(ev);
      break;

    default:
      NODE_ASSERT(0);
      break;
  }
}

void NodeProxy::OnTestDone() {
  NODE_LOGF();
}

void NodeProxy::OnDelete() {
  NODE_LOGF();

  // FIXME: we should reset loadModule stub here..
  m_node = 0;
}

NodeProxy::~NodeProxy() {
  NODE_LOGF();
  disconnectFrame();
}

// Tie the lifecycle of node instance to the page
void NodeProxy::disconnectFrame() {
  NODE_LOGF();
  m_frame = 0;
  if (m_node) {
    delete m_node;
    m_node = 0;
  }
}

// Handle browser focusin/out events
void NodeProxy::pause() {
  NODE_LOGI("%s", __PRETTY_FUNCTION__);
  if (m_node) {
    node::WebKitEvent e;
    e.type = node::WEBKIT_EVENT_PAUSE;
    m_node->HandleWebKitEvent(&e);
  }
}

// Handle browser focusin/out events
void NodeProxy::resume() {
  NODE_LOGI("%s", __PRETTY_FUNCTION__);
  if (m_node) {
    node::WebKitEvent e;
    e.type = node::WEBKIT_EVENT_RESUME;
    m_node->HandleWebKitEvent(&e);
  }
}

// This is the app path on the device to install app specific files
// node modules will be installed in a sub directory under this
// so each app gets its own set of node modules
void NodeProxy::setAppDataPath(const String& dataPath) {
  NODE_LOGI("%s, module path: %s", __FUNCTION__, dataPath.latin1().data());
  s_moduleRootPath = new String(dataPath);
}

}
