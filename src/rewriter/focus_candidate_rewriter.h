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

#ifndef MOZC_REWRITER_FOCUS_CANDIDATE_REWRITER_H_
#define MOZC_REWRITER_FOCUS_CANDIDATE_REWRITER_H_

#include "rewriter/rewriter_interface.h"

namespace mozc {

class Segments;

class FocusCandidateRewriter: public RewriterInterface  {
 public:
  FocusCandidateRewriter();
  virtual ~FocusCandidateRewriter();

  // Changed the focus of "segment_index"-th segment to be "candidate_index".
  // The segments will be written according to pre-defined "actions".
  // Currently, FocusSegmentValue() finds bracket/parentheses matching, e.g,
  // When user chooses "(" in some candidate, corresponding close bracket ")"
  // is automatically placed at the top.
  bool Focus(Segments *segments,
             size_t segment_index,
             int candidate_index) const;

  bool Rewrite(Segments *segments) const {
    return false;
  }
};
}  // mozc
#endif  // MOZC_REWRITER_FOCUS_CANDIDATE_REWRITER_H_
