// Copyright 2010, Google Inc.
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

#ifndef IME_MOZC_CONVERTER_CANDIDATE_FILTER_H_
#define IME_MOZC_CONVERTER_CANDIDATE_FILTER_H_

#include <set>
#include <string>
#include <utility>
#include "converter/segments.h"

namespace mozc {

class CandidateFilter {
 public:
  explicit CandidateFilter();
  virtual ~CandidateFilter();

  enum ResultType {
    GOOD_CANDIDATE,    // Can insert the candidate into the list
    BAD_CANDIDATE,     // Do not insert the candidate
    STOP_ENUMERATION,  // Do not insert and stop enumurations
  };

  // return ResultType
  ResultType FilterCandidate(const Segment::Candidate *candidate);

  // reset internal state
  void Reset();

 private:
  ResultType FilterCandidateInternal(
      const Segment::Candidate *candidate);

  set<string> seen_;
  const Segment::Candidate *top_candidate_;
  set<pair<int, int> > svm_no_filter_ids_;

  DISALLOW_COPY_AND_ASSIGN(CandidateFilter);
};
}  // namespace mozc

#endif  // IME_MOZC_CONVERTER_CANDIDATE_FILTER_H_
