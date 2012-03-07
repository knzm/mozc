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

#include "rewriter/number_rewriter.h"

#include <stdio.h>
#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "base/util.h"
#include "dictionary/pos_matcher.h"
#include "config/config_handler.h"
#include "config/config.pb.h"
#include "converter/segments.h"
#include "session/commands.pb.h"

namespace mozc {
namespace {

// If top candidate is Kanji numeric, we want to expand at least
// 5 candidates apart from base candidate.
// http://b/issue?id=2872048
const int kArabicNumericOffset = 5;

// Rewrite type
enum RewriteType {
  NO_REWRITE = 0,
  ARABIC_FIRST,  // arabic candidates first ordering
  KANJI_FIRST,  // kanji candidates first ordering
};

void PushBackCandidate(const string &value, const string &desc, uint16 style,
                       vector<Segment::Candidate> *results) {
  bool found = false;
  for (vector<Segment::Candidate>::const_iterator it = results->begin();
       it != results->end(); ++it) {
    if (it->value == value) {
      found = true;
      break;
    }
  }
  if (!found) {
    Segment::Candidate cand;
    cand.value = value;
    cand.description = desc;
    cand.style = style;
    results->push_back(cand);
  }
}

bool IsNumber(uint16 lid) {
  // Number candidates sometimes categorized as general noun.
  // TODO(toshiyuki): It's better if we can rewrite
  // from general noun POS to number POS
  // TODO(toshiyuki): We can remove general noun check if we can set
  // correct POS.
  return (POSMatcher::IsNumber(lid) || POSMatcher::IsKanjiNumber(lid) ||
          POSMatcher::IsGeneralNoun(lid));
}

// Returns rewrite type for the given segment and base candidate information.
// *base_candidate_pos: candidate index of starting insertion.
// *arabic_candidate: arabic candidate using numeric style conversion.
// POS information, cost, etc will be copied from base candidate.
RewriteType GetRewriteTypeAndBase(const Segment &seg,
                                  int *base_candidate_pos,
                                  Segment::Candidate *arabic_candidate) {
  DCHECK(base_candidate_pos);
  DCHECK(arabic_candidate);
  for (size_t i = 0; i < seg.candidates_size(); ++i) {
    const Segment::Candidate &c = seg.candidate(i);
    if (!IsNumber(c.lid)) {
      continue;
    }

    if (Util::GetScriptType(c.content_value) == Util::NUMBER) {
      *base_candidate_pos = i;
      arabic_candidate->CopyFrom(c);
      return ARABIC_FIRST;
    }

    string kanji_number, arabic_number, half_width_new_content_value;
    Util::FullWidthToHalfWidth(c.content_key, &half_width_new_content_value);
    // Try to get normalized kanji_number and arabic_number.
    // If it failed, do nothing.
    // Retain suffix for later use.
    string number_suffix;
    if (!Util::NormalizeNumbersWithSuffix(c.content_value,
                                          true,  // trim_reading_zeros
                                          &kanji_number,
                                          &arabic_number,
                                          &number_suffix) ||
        arabic_number == half_width_new_content_value) {
      return NO_REWRITE;
    }
    const string suffix = c.value.substr(
        c.content_value.size(), c.value.size() - c.content_value.size());
    arabic_candidate->Init();
    arabic_candidate->value = arabic_number + number_suffix + suffix;
    arabic_candidate->content_value = arabic_number + number_suffix;
    arabic_candidate->key = c.key;
    arabic_candidate->content_key = c.content_key;
    arabic_candidate->cost = c.cost;
    arabic_candidate->structure_cost = c.structure_cost;
    arabic_candidate->lid = c.lid;
    arabic_candidate->rid = c.rid;
    *base_candidate_pos = i;
    return KANJI_FIRST;
  }

  return NO_REWRITE;
}

void SetCandidatesInfo(const Segment::Candidate &arabic_cand,
                       vector<Segment::Candidate> *candidates) {
  const string suffix =
      arabic_cand.value.substr(arabic_cand.content_value.size(),
                               arabic_cand.value.size() -
                               arabic_cand.content_value.size());

  for (vector<Segment::Candidate>::iterator it = candidates->begin();
       it != candidates->end(); ++it) {
    it->content_value.assign(it->value);
    it->value.append(suffix);
  }
}

class CheckValueOperator {
 public:
  explicit CheckValueOperator(const string &v) : find_value_(v) {}
  bool operator() (const Segment::Candidate &cand) const {
    return (cand.value == find_value_);
  }

 private:
  const string &find_value_;
};

// If we have the candidates to be inserted before the base candidate,
// delete them.
// TODO(toshiyuki): Delete candidates between base pos and insert pos
// if necessary.
void EraseExistingCandidates(const vector<Segment::Candidate> &results,
                             int *base_candidate_pos,
                             int *insert_pos, Segment *seg) {
  DCHECK(base_candidate_pos);
  DCHECK(insert_pos);
  DCHECK(seg);
  // Remember base candidate value
  const string &base_value = seg->candidate(*base_candidate_pos).value;
  size_t pos = 0;
  while (pos < seg->candidates_size()) {
    const string &value = seg->candidate(pos).value;
    if (value == base_value) {
      break;
    }
    // Simple liner search. |results| size is small. (at most 10 or so)
    const vector<Segment::Candidate>::const_iterator itr =
        find_if(results.begin(), results.end(), CheckValueOperator(value));
    if (itr == results.end()) {
      ++pos;
      continue;
    }
    seg->erase_candidate(pos);
    --(*base_candidate_pos);
    --(*insert_pos);
  }
}

// This is a utility function for InsertCandidate and UpdateCandidate.
// Do not use this function directly.
void MergeCandidateInfoInternal(const Segment::Candidate &base_cand,
                                const Segment::Candidate &result_cand,
                                Segment::Candidate *cand) {
  DCHECK(cand);
  cand->lid = base_cand.lid;
  cand->rid = base_cand.rid;
  cand->cost = base_cand.cost;
  cand->value = result_cand.value;
  cand->content_value = result_cand.content_value;
  cand->key = base_cand.key;
  cand->content_key = base_cand.content_key;
  cand->style = result_cand.style;
  cand->description = result_cand.description;
  // Don't want to have FULL_WIDTH form for Hex/Oct/BIN..etc.
  if (cand->style == Segment::Candidate::NUMBER_HEX ||
      cand->style == Segment::Candidate::NUMBER_OCT ||
      cand->style == Segment::Candidate::NUMBER_BIN) {
    cand->attributes |= Segment::Candidate::NO_VARIANTS_EXPANSION;
  }
}

void InsertCandidate(Segment *segment,
                     int32 insert_position,
                     const Segment::Candidate &base_cand,
                     const Segment::Candidate &result_cand) {
  DCHECK(segment);
  Segment::Candidate *c = segment->insert_candidate(insert_position);
  c->Init();
  MergeCandidateInfoInternal(base_cand, result_cand, c);
}

void UpdateCandidate(Segment *segment,
                     int32 update_position,
                     const Segment::Candidate &base_cand,
                     const Segment::Candidate &result_cand) {
  DCHECK(segment);
  Segment::Candidate *c = segment->mutable_candidate(update_position);
  // Do not call |c->Init()| for an existing candidate.
  // There are two major reasons.
  // 1) Future design change may introduce another field into
  //    Segment::Candidate. In such situation, simply calling |c->Init()|
  //    for an existing candidate may result in unexpeced data loss.
  // 2) In order to preserve existing attribute information such as
  //    Segment::Candidate::USER_DICTIONARY bit in |c|, we cannot not call
  //    |c->Init()|. Note that neither |base_cand| nor |result[0]| has
  //    valid value in its |attributes|.
  MergeCandidateInfoInternal(base_cand, result_cand, c);
}

void InsertConvertedCandidates(const vector<Segment::Candidate> &results,
                               const Segment::Candidate &base_cand,
                               int base_candidate_pos,
                               int insert_pos, Segment *seg) {
  if (results.empty()) {
    return;
  }
  if (base_candidate_pos >= seg->candidates_size()) {
    LOG(WARNING) << "Invalid base candidate pos";
    return;
  }
  // First, insert top candidate
  // If we find the base candidate is equal to the converted
  // special form candidates, we will rewrite it.
  // Otherwise, we will insert top candidate just below the base.
  // Sometimes original base candidate is different from converted candidate
  // For example, "千万" v.s. "一千万", or "一二三" v.s. "百二十三".
  // We don't want to rewrite "千万" to "一千万".
  {
    const string &base_value = seg->candidate(base_candidate_pos).value;
    vector<Segment::Candidate>::const_iterator itr =
        find_if(results.begin(), results.end(), CheckValueOperator(base_value));
    if (itr != results.end() &&
        itr->style != Segment::Candidate::NUMBER_KANJI &&
        itr->style != Segment::Candidate::NUMBER_KANJI_ARABIC) {
      // Update exsisting base candidate
      UpdateCandidate(seg, base_candidate_pos, base_cand, results[0]);
    } else {
      // Insert candidate just below the base candidate
      InsertCandidate(seg, base_candidate_pos + 1, base_cand, results[0]);
      ++insert_pos;
    }
  }

  // Insert others
  for (size_t i = 1; i < results.size(); ++i) {
    InsertCandidate(seg, insert_pos++, base_cand, results[i]);
  }
}

int GetInsertPos(int base_pos, const Segment &segment, RewriteType type) {
  if (type == ARABIC_FIRST) {
    // +2 for arabic half_width full_width expansion
    return min(base_pos + 2, static_cast<int>(segment.candidates_size()));
  } else {
    return min(base_pos + kArabicNumericOffset,
               static_cast<int>(segment.candidates_size()));
  }
}

void InsertHalfArabic(const string &half_arabic,
                      vector<Util::NumberString> *output) {
  output->push_back(Util::NumberString(half_arabic, "",
                                       Util::NumberString::DEFAULT_STYLE));
}

void GetNumbers(RewriteType type, const Segments &segments,
                const string &arabic_content_value,
                vector<Util::NumberString> *output) {
  DCHECK(output);
  if (type == ARABIC_FIRST) {
    InsertHalfArabic(arabic_content_value, output);
    Util::ArabicToWideArabic(arabic_content_value, output);
    Util::ArabicToSeparatedArabic(arabic_content_value, output);
    Util::ArabicToKanji(arabic_content_value, output);
    Util::ArabicToOtherForms(arabic_content_value, output);
  } else if (type == KANJI_FIRST) {
    Util::ArabicToKanji(arabic_content_value, output);
    InsertHalfArabic(arabic_content_value, output);
    Util::ArabicToWideArabic(arabic_content_value, output);
    Util::ArabicToSeparatedArabic(arabic_content_value, output);
    Util::ArabicToOtherForms(arabic_content_value, output);
  }

  // Radix conversion is done only for conversion mode.
  // Showing radix candidates is annoying for an user.
  if (segments.conversion_segments_size() == 1 &&
      segments.request_type() == Segments::CONVERSION) {
    Util::ArabicToOtherRadixes(arabic_content_value, output);
  }
}
}  // namespace

NumberRewriter::NumberRewriter() {}
NumberRewriter::~NumberRewriter() {}

int NumberRewriter::capability() const {
  return RewriterInterface::CONVERSION;
}

bool NumberRewriter::Rewrite(Segments *segments) const {
  if (!GET_CONFIG(use_number_conversion)) {
    VLOG(2) << "no use_number_conversion";
    return false;
  }

  bool modified = false;
  for (size_t i = 0; i < segments->conversion_segments_size(); ++i) {
    Segment *seg = segments->mutable_conversion_segment(i);
    DCHECK(seg);
    int base_candidate_pos = 0;
    Segment::Candidate arabic_cand;
    RewriteType type = GetRewriteTypeAndBase(
        segments->conversion_segment(i), &base_candidate_pos, &arabic_cand);
    if (type == NO_REWRITE) {
      continue;
    }
    modified = true;

    if (arabic_cand.content_value.size() > arabic_cand.value.size()) {
      LOG(ERROR) << "Invalid content_value/value: ";
      continue;
    }

    string arabic_content_value;
    Util::FullWidthToHalfWidth(
        arabic_cand.content_value, &arabic_content_value);
    if (Util::GetScriptType(arabic_content_value) != Util::NUMBER) {
      if (Util::GetFirstScriptType(arabic_content_value) == Util::NUMBER) {
        // Rewrite for number suffix
        const int insert_pos = min(base_candidate_pos + 1,
                                   static_cast<int>(seg->candidates_size()));
        InsertCandidate(seg, insert_pos, arabic_cand, arabic_cand);
        continue;  // It's normal for a candidate to have a suffix.
      }
      LOG(ERROR) << "arabic_content_value is not number: "
                 << arabic_content_value;
      continue;
    }
    vector<Util::NumberString> output;
    GetNumbers(type, *segments, arabic_content_value, &output);
    vector<Segment::Candidate> converted_numbers;
    for (int j = 0; j < output.size(); j++) {
      PushBackCandidate(output[j].value, output[j].description, output[j].style,
                        &converted_numbers);
    }
    SetCandidatesInfo(arabic_cand, &converted_numbers);
    int insert_pos = GetInsertPos(base_candidate_pos, *seg, type);
    EraseExistingCandidates(
        converted_numbers, &base_candidate_pos, &insert_pos, seg);
    DCHECK_LT(base_candidate_pos, insert_pos);
    InsertConvertedCandidates(converted_numbers, arabic_cand,
                              base_candidate_pos,
                              insert_pos, seg);
  }

  return modified;
}
}  // namespace mozc
