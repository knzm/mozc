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

// A class handling the converter on the session layer.

#ifndef MOZC_SESSION_SESSION_CONVERTER_INTERFACE_H_
#define MOZC_SESSION_SESSION_CONVERTER_INTERFACE_H_

#include <string>

#include "base/base.h"
#include "converter/segments.h"
#include "composer/composer.h"
#include "session/commands.pb.h"

namespace mozc {
class ConverterInterface;

namespace config {
class Config;
}

namespace session {
struct ConversionPreferences {
  bool use_history;
  int max_history_size;
};

struct OperationPreferences {
  bool use_cascading_window;
  string candidate_shortcuts;
};

// Class handling ConverterInterface with a session state.  This class
// support stateful operations related with the converter.
class SessionConverterInterface {
 public:
  SessionConverterInterface() {}
  virtual ~SessionConverterInterface() {}

  // Reload the global configuration variables.
  virtual void ReloadConfig() ABSTRACT;
  // Update the configuration.
  virtual void UpdateConfig(const config::Config &config) ABSTRACT;

  typedef int States;
  enum State {
    NO_STATE = 0,
    COMPOSITION = 1,
    SUGGESTION = 2,
    PREDICTION = 4,
    CONVERSION = 8,
  };

  // Check if the current state is in the state bitmap.
  virtual bool CheckState(States) const ABSTRACT;

  // Indicate if the conversion session is active or not.  In general,
  // Convert functions make it active and Cancel, Reset and Commit
  // functions make it deactive.
  virtual bool IsActive() const ABSTRACT;

  // Return the default conversion preferences to be used for custom
  // conversion.
  virtual const ConversionPreferences &conversion_preferences() const ABSTRACT;

  // Send a conversion request to the converter.
  virtual bool Convert(const composer::Composer *composer) ABSTRACT;
  virtual bool ConvertWithPreferences(
      const composer::Composer *composer,
      const ConversionPreferences &preferences) ABSTRACT;

  // Send a transliteration request to the converter.
  virtual bool ConvertToTransliteration(
      const composer::Composer *composer,
      transliteration::TransliterationType type) ABSTRACT;

  // Convert the current composition to half-width characters.
  // NOTE(komatsu): This function might be merged to ConvertToTransliteration.
  virtual bool ConvertToHalfWidth(const composer::Composer *composer) ABSTRACT;

  // Switch the composition to Hiragana, full-width Katakana or
  // half-width Katakana by rotation.
  virtual bool SwitchKanaType(const composer::Composer *composer) ABSTRACT;

  // Send a suggestion request to the converter.
  virtual bool Suggest(const composer::Composer *composer) ABSTRACT;
  virtual bool SuggestWithPreferences(
      const composer::Composer *composer,
      const ConversionPreferences &preferences) ABSTRACT;

  // Send a prediction request to the converter.
  virtual bool Predict(const composer::Composer *composer) ABSTRACT;
  virtual bool PredictWithPreferences(
      const composer::Composer *composer,
      const ConversionPreferences &preferences) ABSTRACT;



  // Clear conversion segments, but keep the context.
  virtual void Cancel() ABSTRACT;

  // Clear conversion segments and the context.
  virtual void Reset() ABSTRACT;

  // Fix the conversion with the current status.
  virtual void Commit() ABSTRACT;

  // Fix the suggestion candidate.
  virtual void CommitSuggestion(size_t index) ABSTRACT;

  // Fix only the conversion of the first segment, and keep the rest.
  virtual void CommitFirstSegment(composer::Composer *composer) ABSTRACT;

  // Commit the preedit string represented by Composer.
  virtual void CommitPreedit(const composer::Composer &composer) ABSTRACT;

  // Revert the last "Commit" operation
  virtual void Revert() ABSTRACT;

  // Move the focus of segments.
  virtual void SegmentFocusRight() ABSTRACT;
  virtual void SegmentFocusLast() ABSTRACT;
  virtual void SegmentFocusLeft() ABSTRACT;
  virtual void SegmentFocusLeftEdge() ABSTRACT;

  // Resize the focused segment.
  virtual void SegmentWidthExpand() ABSTRACT;
  virtual void SegmentWidthShrink() ABSTRACT;

  // Move the focus of candidates.
  virtual void CandidateNext() ABSTRACT;
  virtual void CandidateNextPage() ABSTRACT;
  virtual void CandidatePrev() ABSTRACT;
  virtual void CandidatePrevPage() ABSTRACT;
  // Move the focus to the candidate represented by the id.
  virtual void CandidateMoveToId(int id) ABSTRACT;
  // Move the focus to the index from the beginning of the current page.
  virtual void CandidateMoveToPageIndex(size_t index) ABSTRACT;
  // Move the focus to the candidate represented by the shortcut.  If
  // the shortcut is not bound with any candidate, false is returned.
  virtual bool CandidateMoveToShortcut(char shortcut) ABSTRACT;

  // Operation for the candidate list.
  virtual bool IsCandidateListVisible() const ABSTRACT;
  virtual void SetCandidateListVisible(bool visible) ABSTRACT;

  // Fill protocol buffers and update internal status.
  virtual void PopOutput(commands::Output *output) ABSTRACT;

  // Fill protocol buffers
  virtual void FillOutput(commands::Output *output) const ABSTRACT;

  virtual const string &GetDefaultResult() const ABSTRACT;

 private:
  DISALLOW_COPY_AND_ASSIGN(SessionConverterInterface);
};
}  // namespace session
}  // namespace mozc
#endif  // MOZC_SESSION_SESSION_CONVERTER_INTERFACE_H_
