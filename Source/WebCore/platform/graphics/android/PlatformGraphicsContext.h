/*
 * Copyright 2006, The Android Open Source Project
 * Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef platform_graphics_context_h
#define platform_graphics_context_h

#include "IntRect.h"
#include "RenderSkinAndroid.h"
#include "RenderSkinButton.h"
#include "SkCanvas.h"
#include "SkPicture.h"
#include "SkTDArray.h"

class SkCanvas;

class Container {
public:
    Container(WebCore::Node* node, const WebCore::IntRect& r)
        : m_node(node), m_rect(r), m_state(WebCore::RenderSkinAndroid::kDisabled)
    {
        m_picture = new SkPicture;
    }

    ~Container()
    {
        m_picture->unref();
    }

    Container& operator=(const Container& src)
    {
        if (this != &src) {
            m_node = src.m_node;
            if (m_picture)
                m_picture->unref();
            m_picture = src.m_picture;
            m_picture->ref();
            m_rect = src.m_rect;
            m_state = WebCore::RenderSkinAndroid::kDisabled;
        }
        return *this;
    }

    Container(const Container& src)
    {
        m_node = src.m_node;
        m_picture = src.m_picture;
        m_picture->ref();
        m_rect = src.m_rect;
        m_state = WebCore::RenderSkinAndroid::kDisabled;
    }

    // m_picture has a ref count of 1 to begin with.  It will increase each time
    // m_picture is referenced by another picture.  When the other pictures are
    // deleted, the ref count gets decremented.  If the ref count is one, then
    // no other pictures reference this one, so the button is no longer being
    // used, and therefore can be removed.
    bool canBeRemoved()
    {
        return m_picture->getRefCnt() == 1;
    }

    bool matches(const WebCore::Node* match) { return m_node == match; }

    const WebCore::Node* node() const { return m_node; }

    // Provide a pointer to our SkPicture.
    SkPicture* picture() { return m_picture; }

    WebCore::IntRect rect() { return m_rect; }

    // Update the rectangle with a new rectangle, as the positioning of this
    // button may have changed due to a new layout.  If it is a new rectangle,
    // set its state to disabled, so that it will be redrawn when we cycle
    // through the list of buttons.
    void setRect(WebCore::IntRect r)
    {
        if (m_rect != r) {
            m_rect = r;
            m_state = WebCore::RenderSkinAndroid::kDisabled;
        }
    }

    // Update the focus state of this button, depending on whether it
    // corresponds to the focused node passed in.  If its state has changed,
    // re-record to the subpicture, so the master picture will reflect the
    // change.
    void updateFocusState(WebCore::RenderSkinAndroid::State state,
                          WebCore::RenderSkinButton* buttonSkin)
    {
        if (state == m_state)
            return;
        // If this button is being told to draw focused, but it is already in a
        // pressed state, leave it in the pressed state, to show that it is
        // being followed.
        if (m_state == WebCore::RenderSkinAndroid::kPressed &&
                state == WebCore::RenderSkinAndroid::kFocused)
            return;
        m_state = state;
        SkCanvas* canvas = m_picture->beginRecording(m_rect.maxX(), m_rect.maxY());
        buttonSkin->draw(canvas, m_rect, state);
        m_picture->endRecording();
    }
private:
    // Only used for comparison, since after it is stored it will be transferred
    // to the UI thread.
    WebCore::Node*                      m_node;
    // The rectangle representing the bounds of the button.
    WebCore::IntRect                    m_rect;
    // An SkPicture that, thanks to storeButtonInfo, is pointed to by the master
    // picture, so that we can rerecord this button without rerecording the
    // world.
    SkPicture*                          m_picture;
    // The state of the button - Currently kFocused or kNormal (and kDisabled
    // as an initial value), but could be expanded to include other states.
    WebCore::RenderSkinAndroid::State   m_state;
};

namespace WebCore {

    class GraphicsContext;

class PlatformGraphicsContext {
public:
    PlatformGraphicsContext();
    // Pass in a recording canvas, and an array of button information to be
    // updated.
    PlatformGraphicsContext(SkCanvas* canvas, WTF::Vector<Container>* buttons);
    // Create a recording canvas
    PlatformGraphicsContext(int width, int height);
    ~PlatformGraphicsContext();

    SkCanvas* mCanvas;

    bool deleteUs() const { return m_deleteCanvas; }
    // If our graphicscontext has a button list, add a new container for the
    // nod/rect, and record a new subpicture for this node/button in the current
    // mCanvas
    void storeButtonInfo(Node* node, const IntRect& r);

    void convertToNonRecording();
    void clearRecording();
    SkPicture* getRecordingPicture() const { return m_picture; }

    // When we detect animation we switch to the recording canvas for better
    // performance. If JS tries to access the pixels of the canvas, the
    // recording canvas becomes tainted and must be converted back to a bitmap
    // backed canvas.
    // CanvasState represents a directed acyclic graph:
    // DEFAULT ----> ANIMATION_DETECTED ----> RECORDING ----> DIRTY
    enum CanvasState {
        DEFAULT, // SkBitmap backed
        ANIMATION_DETECTED, // JavaScript clearRect of canvas is occuring at a high enough rate SkBitmap backed
        RECORDING, // SkPicture backed
        DIRTY // A pixel readback occured; convert to SkBitmap backed.
    };

    bool isDefault() const { return m_canvasState == DEFAULT; }
    bool isAnimating() const { return m_canvasState == ANIMATION_DETECTED; }
    bool isRecording() const { return m_canvasState == RECORDING; }
    bool isDirty() const { return m_canvasState == DIRTY; }

    void setIsAnimating();

private:
    bool m_deleteCanvas;
    WTF::Vector<Container>* m_buttons;
    enum CanvasState m_canvasState;

    SkPicture* m_picture;
};

}
#endif
