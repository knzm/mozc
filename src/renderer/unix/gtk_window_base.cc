// Copyright 2010-2012, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "renderer/unix/gtk_window_base.h"
#include "renderer/unix/gtk_wrapper.h"

namespace mozc {
namespace renderer {
namespace gtk {

GtkWindowBase::GtkWindowBase(GtkWrapperInterface *gtk)
    : gtk_(gtk),
      window_(gtk_->GtkWindowNew(GTK_WINDOW_POPUP)),
      canvas_(gtk_->GtkDrawingAreaNew()) {
  gtk_->GSignalConnect(window_, "destroy", G_CALLBACK(OnDestroyThunk), this);
  gtk_->GSignalConnect(canvas_, "expose-event", G_CALLBACK(OnPaintThunk), this);
  gtk_->GtkContainerAdd(window_, canvas_);
}

GtkWindowBase::~GtkWindowBase() {
}

void GtkWindowBase::ShowWindow() {
  gtk_->GtkWidgetShowAll(window_);
}

void GtkWindowBase::HideWindow() {
  gtk_->GtkWidgetHideAll(window_);
}

GtkWidget *GtkWindowBase::GetWindowWidget() {
  return window_;
}

GtkWidget *GtkWindowBase::GetCanvasWidget() {
  return canvas_;
}

Point GtkWindowBase::GetWindowPos() {
  Point origin;
  gtk_->GtkWindowGetPosition(window_, &origin.x, &origin.y);
  return origin;
}

Size GtkWindowBase::GetWindowSize() {
  Size size;
  gtk_->GtkWindowGetSize(window_, &size.width, &size.height);
  return size;
}

Rect GtkWindowBase::GetWindowRect() {
  return Rect(GetWindowPos(), GetWindowSize());
}

bool GtkWindowBase::IsActive() {
  return gtk_->GtkWindowIsActive(window_);
}

bool GtkWindowBase::DestroyWindow() {
  // TODO(nona): Implement this
  return false;
}

void GtkWindowBase::Move(const Point &pos) {
  gtk_->GtkWindowMove(window_, pos.x, pos.y);
}

void GtkWindowBase::Resize(const Size &size) {
  gtk_->GtkWindowResize(window_, size.width, size.height);
}

void GtkWindowBase::Redraw() {
  Size size = GetWindowSize();
  gtk_->GtkWidgetQueueDrawArea(window_, 0, 0, size.width, size.height);
}

// Callbacks
bool GtkWindowBase::OnDestroy(GtkWidget *widget) {
  gtk_->GtkMainQuit();
  return true;
}

bool GtkWindowBase::OnPaint(GtkWidget *widget, GdkEventExpose *event) {
  return true;
}

void GtkWindowBase::Initialize() {
  // Do nothing, there are no initialization on GtkWindowBase class.
}

Size GtkWindowBase::Update(const commands::Candidates &candidates) {
  // Do nothing, because this method should be overridden.
  DLOG(FATAL) << "Unexpected function call.";
  return Size(0, 0);
}

Rect GtkWindowBase::GetCandidateColumnInClientCord() const {
  // Do nothing, this method should be overridden and only used by candidate
  // window.
  return Rect(0, 0, 0, 0);
}

}  // namespace gtk
}  // namespace renderer
}  // namespace mozc
