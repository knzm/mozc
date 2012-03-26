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

#include "languages/pinyin/session_converter.h"

#include <algorithm>
#include <string>
#include <vector>

#include "base/util.h"
#include "languages/pinyin/pinyin_context_interface.h"
#include "session/commands.pb.h"
#include "session/key_event_util.h"

namespace mozc {
namespace pinyin {
namespace {
// TODO(hsumita): Calculate this value by the platform-specific APIs.
const size_t kCandidatesPerPage = 5;
}  // namespace

SessionConverter::SessionConverter(PinyinContextInterface *context)
    : context_(context) {
}

SessionConverter::~SessionConverter() {
}

bool SessionConverter::IsConverterActive() const {
  return !context_->input_text().empty();
}

bool SessionConverter::IsCandidateListVisible() const {
  return (context_->candidates_size() != 0 ||
          !context_->auxiliary_text().empty());
}

bool SessionConverter::IsConversionTextVisible() const {
  return !(context_->selected_text().empty() &&
           context_->conversion_text().empty() &&
           context_->rest_text().empty());
}

bool SessionConverter::Insert(const commands::KeyEvent &key_event) {
  if (!key_event.has_key_code()) {
    return false;
  }

  const uint32 key_code = key_event.key_code();
  const uint32 modifiers = KeyEventUtil::GetModifiers(key_event);
  DCHECK(!KeyEventUtil::HasCaps(modifiers));

  char insert_character = key_code;
  if (KeyEventUtil::IsShift(modifiers)) {
    if (isupper(key_code)) {
      insert_character = tolower(key_code);
    } else if (islower(key_code)) {
      insert_character = toupper(key_code);
    }
  } else if (modifiers != 0) {
    return false;
  }

  return context_->Insert(insert_character);
}

void SessionConverter::Clear() {
  context_->Clear();
}

void SessionConverter::Commit() {
  context_->Commit();
}

void SessionConverter::CommitPreedit() {
  context_->CommitPreedit();
}

bool SessionConverter::SelectCandidateOnPage(size_t index) {
  size_t absolute_index;
  if (!GetAbsoluteIndex(index, &absolute_index)) {
    return false;
  }
  return context_->SelectCandidate(absolute_index);
}

bool SessionConverter::SelectFocusedCandidate() {
  if (context_->candidates_size() == 0) {
    context_->Commit();
    return true;
  }
  return context_->SelectCandidate(context_->focused_candidate_index());
}

bool SessionConverter::FocusCandidate(size_t index) {
  if (index >= context_->candidates_size()) {
    return false;
  }
  return context_->FocusCandidate(index);
}

bool SessionConverter::FocusCandidateOnPage(size_t index) {
  size_t absolute_index;
  if (!GetAbsoluteIndex(index, &absolute_index)) {
    return false;
  }
  return FocusCandidate(absolute_index);
}

bool SessionConverter::FocusCandidateNext() {
  return context_->FocusCandidateNext();
}

bool SessionConverter::FocusCandidateNextPage() {
  DCHECK_LT(0, context_->candidates_size());

  const size_t current_page =
      context_->focused_candidate_index() / kCandidatesPerPage;
  const size_t last_page =
      (context_->candidates_size() - 1) / kCandidatesPerPage;

  if (current_page == last_page) {
    return false;
  }

  const size_t index = min(context_->candidates_size() - 1,
                           context_->focused_candidate_index()
                           + kCandidatesPerPage);
  return context_->FocusCandidate(index);
}

bool SessionConverter::FocusCandidatePrev() {
  return context_->FocusCandidatePrev();
}

bool SessionConverter::FocusCandidatePrevPage() {
  if (context_->focused_candidate_index() < kCandidatesPerPage) {
    return false;
  }
  const size_t index =
      context_->focused_candidate_index() - kCandidatesPerPage;
  return context_->FocusCandidate(index);
}

bool SessionConverter::ClearCandidateFromHistory(size_t index) {
  size_t absolute_index;
  if (!GetAbsoluteIndex(index, &absolute_index)) {
    return false;
  }
  return context_->ClearCandidateFromHistory(absolute_index);
}

bool SessionConverter::RemoveCharBefore() {
  return context_->RemoveCharBefore();
}

bool SessionConverter::RemoveCharAfter() {
  return context_->RemoveCharAfter();
}

bool SessionConverter::RemoveWordBefore() {
  return context_->RemoveWordBefore();
}

bool SessionConverter::RemoveWordAfter() {
  return context_->RemoveWordAfter();
}

bool SessionConverter::MoveCursorRight() {
  return context_->MoveCursorRight();
}

bool SessionConverter::MoveCursorLeft() {
  return context_->MoveCursorLeft();
}

bool SessionConverter::MoveCursorRightByWord() {
  return context_->MoveCursorRightByWord();
}

bool SessionConverter::MoveCursorLeftByWord() {
  return context_->MoveCursorLeftByWord();
}

bool SessionConverter::MoveCursorToBeginning() {
  return context_->MoveCursorToBeginning();
}

bool SessionConverter::MoveCursorToEnd() {
  return context_->MoveCursorToEnd();
}

void SessionConverter::FillOutput(commands::Output *output) const {
  DCHECK(output);

  if (!context_->commit_text().empty()) {
    FillResult(output->mutable_result());
  }

  if (IsConversionTextVisible()) {
    FillConversion(output->mutable_preedit());
  }

  if (IsCandidateListVisible()) {
    FillCandidates(output->mutable_candidates());
  }
}

void SessionConverter::PopOutput(commands::Output *output) {
  DCHECK(output);
  FillOutput(output);
  context_->ClearCommitText();
}

void SessionConverter::FillConversion(commands::Preedit *preedit) const {
  DCHECK(preedit);
  DCHECK(IsConversionTextVisible());
  preedit->Clear();

  const string texts[3] = {
    context_->selected_text(),
    context_->conversion_text(),
    context_->rest_text(),
  };
  const size_t kConversionIndex = 1;

  // Add segments
  size_t segment_num = 0;
  size_t total_size = 0;
  for (int i = 0; i < 3; ++i) {
    if (texts[i].empty()) {
      continue;
    }

    commands::Preedit::Segment *segment = preedit->add_segment();
    segment->set_value(texts[i]);
    const size_t text_size = Util::CharsLen(texts[i]);
    segment->set_value_length(text_size);
    total_size += text_size;

    if (i == kConversionIndex) {
      segment->set_annotation(commands::Preedit::Segment::HIGHLIGHT);
      preedit->set_highlighted_position(segment_num);
    } else {
      segment->set_annotation(commands::Preedit::Segment::UNDERLINE);
    }

    ++segment_num;
  }

  preedit->set_cursor(Util::CharsLen(context_->selected_text()));
}

void SessionConverter::FillResult(commands::Result *result) const {
  DCHECK(result);
  DCHECK(!context_->commit_text().empty());
  result->Clear();

  result->set_value(context_->commit_text());
  result->set_type(commands::Result::STRING);
}

void SessionConverter::FillCandidates(commands::Candidates *candidates) const {
  DCHECK(candidates);
  DCHECK(IsCandidateListVisible());
  candidates->Clear();

  vector<string> candidates_list;
  context_->GetCandidates(&candidates_list);
  candidates->set_size(context_->candidates_size());

  if (!candidates_list.empty()) {
    const size_t focused_index = context_->focused_candidate_index();
    const size_t candidates_begin =
        focused_index - focused_index % kCandidatesPerPage;
    const size_t candidates_end = min(context_->candidates_size(),
                                      candidates_begin + kCandidatesPerPage);

    for (size_t i = candidates_begin; i < candidates_end; ++i) {
      commands::Candidates::Candidate *new_candidate =
          candidates->add_candidate();
      new_candidate->set_id(i);
      new_candidate->set_index(i);
      new_candidate->set_value(candidates_list[i]);
    }

    {
      const string kDigits = "1234567890";

      // Logic here is copied from SessionOutput::FillShortcuts.  We
      // can't reuse this at this time because SessionOutput depends on
      // converter/segments.cc, which depends on the Japanese language
      // model.
      // TODO(hsumita): extract FillShortcuts() method to another library.
      const size_t num_loop = candidates_end - candidates_begin;
      for (size_t i = 0; i < num_loop; ++i) {
        const string shortcut = kDigits.substr(i, 1);
        candidates->mutable_candidate(i)->mutable_annotation()->
            set_shortcut(shortcut);
      }
    }

    candidates->set_focused_index(context_->focused_candidate_index());
  }

  if (!context_->auxiliary_text().empty()) {
    commands::Footer *footer = candidates->mutable_footer();
    footer->set_label(context_->auxiliary_text());
  }

  candidates->set_direction(commands::Candidates::HORIZONTAL);
  candidates->set_display_type(commands::MAIN);
  candidates->set_position(Util::CharsLen(context_->selected_text()));
}

bool SessionConverter::GetAbsoluteIndex(size_t relative_index,
                                        size_t *absolute_index) const {
  DCHECK(absolute_index);

  if (relative_index >= kCandidatesPerPage) {
    return false;
  }

  const size_t focused_index = context_->focused_candidate_index();
  const size_t current_page = focused_index / kCandidatesPerPage;
  const size_t index = current_page * kCandidatesPerPage + relative_index;

  if (index >= context_->candidates_size()) {
    return false;
  }

  *absolute_index = index;
  return true;
}

void SessionConverter::ReloadConfig() {
  context_->ReloadConfig();
}

}  // namespace pinyin
}  // namespace mozc
