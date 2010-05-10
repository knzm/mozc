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

// Interactive composer from a Roman string to a Hiragana string

#include "composer/composer.h"

#include "base/util.h"
#include "converter/character_form_manager.h"
#include "composer/internal/composition.h"
#include "composer/internal/mode_switching_handler.h"
#include "composer/internal/transliterators_ja.h"
#include "composer/table.h"
#include "session/commands.pb.h"
#include "session/config_handler.h"
#include "session/config.pb.h"

namespace mozc {
namespace composer {

namespace {

const TransliteratorInterface *GetTransliterator(
    const transliteration::TransliterationType comp_mode) {
  const TransliteratorInterface *kNullTransliterator = NULL;
  switch (comp_mode) {
    case transliteration::HALF_ASCII:
    case transliteration::HALF_ASCII_UPPER:
    case transliteration::HALF_ASCII_LOWER:
    case transliteration::HALF_ASCII_CAPITALIZED:
      VLOG(2) << "GetTransliterator: GetHalfAsciiTransliterator";
      return TransliteratorsJa::GetHalfAsciiTransliterator();

    case transliteration::FULL_ASCII:
    case transliteration::FULL_ASCII_UPPER:
    case transliteration::FULL_ASCII_LOWER:
    case transliteration::FULL_ASCII_CAPITALIZED:
      VLOG(2) << "GetTransliterator: GetFullAsciiTransliterator";
      return TransliteratorsJa::GetFullAsciiTransliterator();

    case transliteration::HALF_KATAKANA:
      VLOG(2) << "GetTransliterator: GetHalfKatakanaTransliterator";
      return TransliteratorsJa::GetHalfKatakanaTransliterator();

    case transliteration::FULL_KATAKANA:
      VLOG(2) << "GetTransliterator: GetFullKatakanaTransliterator";
      return TransliteratorsJa::GetFullKatakanaTransliterator();

    case transliteration::HIRAGANA:
      VLOG(2) << "GetTransliterator: kNullTransliterator";
      return TransliteratorsJa::GetHiraganaTransliterator();

    default:
      VLOG(2) << "GetTransliterator: kNullTransliterator";
      LOG(ERROR) << "Unknown TransliterationType: " << comp_mode;
      return kNullTransliterator;
  }
  VLOG(2) << "GetTransliterator: kNullTransliterator ";
  return kNullTransliterator;  // Just in case
}

const transliteration::TransliterationType GetTransliterationType(
    const TransliteratorInterface *transliterator,
    const transliteration::TransliterationType default_type) {
  if (transliterator == TransliteratorsJa::GetHiraganaTransliterator()) {
    return transliteration::HIRAGANA;
  }
  if (transliterator == TransliteratorsJa::GetHalfAsciiTransliterator()) {
    return transliteration::HALF_ASCII;
  }
  if (transliterator == TransliteratorsJa::GetFullAsciiTransliterator()) {
    return transliteration::FULL_ASCII;
  }
  if (transliterator == TransliteratorsJa::GetFullKatakanaTransliterator()) {
    return transliteration::FULL_KATAKANA;
  }
  if (transliterator == TransliteratorsJa::GetHalfKatakanaTransliterator()) {
    return transliteration::HALF_KATAKANA;
  }
  return default_type;
}

void Transliterate(const transliteration::TransliterationType mode,
                   const string &input,
                   string *output) {
  // When the mode is HALF_KATAKANA, Full width ASCII is also
  // transformed.
  if (mode == transliteration::HALF_KATAKANA) {
    string tmp_input;
    Util::HiraganaToKatakana(input, &tmp_input);
    Util::FullWidthToHalfWidth(tmp_input, output);
    return;
  }

  switch (mode) {
    case transliteration::HALF_ASCII:
      Util::FullWidthAsciiToHalfWidthAscii(input, output);
      break;
    case transliteration::HALF_ASCII_UPPER:
      Util::FullWidthAsciiToHalfWidthAscii(input, output);
      Util::UpperString(output);
      break;
    case transliteration::HALF_ASCII_LOWER:
      Util::FullWidthAsciiToHalfWidthAscii(input, output);
      Util::LowerString(output);
      break;
    case transliteration::HALF_ASCII_CAPITALIZED:
      Util::FullWidthAsciiToHalfWidthAscii(input, output);
      Util::CapitalizeString(output);
      break;

    case transliteration::FULL_ASCII:
      Util::HalfWidthAsciiToFullWidthAscii(input, output);
      break;
    case transliteration::FULL_ASCII_UPPER:
      Util::HalfWidthAsciiToFullWidthAscii(input, output);
      Util::UpperString(output);
      break;
    case transliteration::FULL_ASCII_LOWER:
      Util::HalfWidthAsciiToFullWidthAscii(input, output);
      Util::LowerString(output);
      break;
    case transliteration::FULL_ASCII_CAPITALIZED:
      Util::HalfWidthAsciiToFullWidthAscii(input, output);
      Util::CapitalizeString(output);
      break;

    case transliteration::FULL_KATAKANA:
      Util::HiraganaToKatakana(input, output);
      break;
    case transliteration::HIRAGANA:
      *output = input;
      break;
    default:
      LOG(ERROR) << "Unknown TransliterationType: " << mode;
      *output = input;
      break;
  }
}

transliteration::TransliterationType GetTransliterationTypeFromCompositionMode(
    const commands::CompositionMode mode) {
  switch (mode) {
    case commands::HIRAGANA:
      return transliteration::HIRAGANA;
    case commands::FULL_KATAKANA:
      return transliteration::FULL_KATAKANA;
    case commands::HALF_ASCII:
      return transliteration::HALF_ASCII;
    case commands::FULL_ASCII:
      return transliteration::FULL_ASCII;
    case commands::HALF_KATAKANA:
      return transliteration::HALF_KATAKANA;
    default:
      // commands::DIRECT or invalid mode.
      LOG(ERROR) << "Invalid CompositionMode: " << mode;
      return transliteration::HIRAGANA;
  }
}

}  // namespace

static const size_t kMaxPreeditLength = 256;
Composer::Composer(CompositionInterface *composition)
    : position_(0),
      input_mode_(transliteration::HIRAGANA),
      output_mode_(transliteration::HIRAGANA),
      comeback_input_mode_(transliteration::HIRAGANA),
      capital_sequence_count_(0),
      max_length_(kMaxPreeditLength) {
  composition_.reset(composition);
  SetInputMode(transliteration::HIRAGANA);
  Reset();
}

Composer::~Composer() {}

void Composer::Reset() {
  EditErase();
  SetInputMode(comeback_input_mode_);
}

void Composer::ReloadConfig() {
  // Do nothing at this moment.
}

bool Composer::Empty() const {
  return (GetLength() == 0);
}

void Composer::SetTable(const Table *table) {
  composition_->SetTable(table);
}

void Composer::SetInputMode(transliteration::TransliterationType mode) {
  comeback_input_mode_ = mode;
  input_mode_ = mode;
  capital_sequence_count_ = 0;
  composition_->SetInputMode(GetTransliterator(mode));
}

void Composer::SetTemporaryInputMode(
    transliteration::TransliterationType mode) {
  // Set comeback_input_mode_ to revert back the current input mode.
  comeback_input_mode_ = input_mode_;
  input_mode_ = mode;
  capital_sequence_count_ = 0;
  composition_->SetInputMode(GetTransliterator(mode));
}

void Composer::UpdateInputMode() {
  if (position_ == 0) {
    // Set the default input mode.
    SetInputMode(comeback_input_mode_);
    return;
  }

  if (position_ == composition_->GetLength()) {
    // Do nothing, if the cursor is in the end of the composition.
    return;
  }

  const TransliteratorInterface *lhs_t12r =
      composition_->GetTransliterator(position_);
  const TransliteratorInterface *rhs_t12r =
      composition_->GetTransliterator(position_ + 1);
  if (lhs_t12r == rhs_t12r) {
    // If the current cursor is between the same character type like
    // "A|B" and "あ|い", the input mode follows the character type.
    input_mode_ = GetTransliterationType(lhs_t12r, comeback_input_mode_);
    capital_sequence_count_ = 0;
    composition_->SetInputMode(GetTransliterator(input_mode_));
    return;
  }

  // Set the default input mode.
  SetInputMode(comeback_input_mode_);
}

transliteration::TransliterationType Composer::GetInputMode() const {
  return input_mode_;
}

transliteration::TransliterationType Composer::GetOutputMode() const {
  return output_mode_;
}

void Composer::ToggleInputMode() {
  if (input_mode_ == transliteration::HIRAGANA) {
    // TODO(komatsu): Refer user's perference.
    SetInputMode(transliteration::HALF_ASCII);
  } else {
    SetInputMode(transliteration::HIRAGANA);
  }
}

void Composer::SetOutputMode(transliteration::TransliterationType mode) {
  output_mode_ = mode;
  composition_->SetTransliterator(
      0, composition_->GetLength(), GetTransliterator(mode));
  MoveCursorToEnd();
}

void Composer::InsertCharacterAt(size_t pos, const string &input) {
  if (!EnableInsert()) {
    return;
  }
  position_ = composition_->InsertAt(pos, input);
}

void Composer::InsertCharacter(const string &input) {
  if (!EnableInsert()) {
    return;
  }
  position_ = composition_->InsertAt(position_, input);
}

void Composer::InsertCharacterPreedit(const string &input) {
  InsertCharacterKeyAndPreedit(input, input);
}

void Composer::InsertCharacterKeyAndPreedit(const string &key,
                                       const string &preedit) {
  if (!EnableInsert()) {
    return;
  }
  position_ = composition_->InsertKeyAndPreeditAt(position_, key, preedit);
}

bool Composer::InsertCharacterKeyEvent(const commands::KeyEvent &key) {
  if (key.has_mode()) {
    const transliteration::TransliterationType new_input_mode =
        GetTransliterationTypeFromCompositionMode(key.mode());
    if (new_input_mode != input_mode_) {
      // Only when the new input mode is different from the current
      // input mode, SetInputMode is called.  Otherwise the value of
      // comeback_input_mode_ is lost.
      SetInputMode(new_input_mode);
    }
  }

  if (!key.has_key_code()) {
    // If only SHIFT is pressed, this is used to revert back to the
    // previous input mode.
    for (size_t i = 0; key.modifier_keys_size(); ++i) {
      if (key.modifier_keys(i) == commands::KeyEvent::SHIFT) {
        // TODO(komatsu): Enable to customize the behavior.
        SetInputMode(comeback_input_mode_);
        return true;
      }
    }
    LOG(WARNING) << "key_code is empty";
    return false;
  }

  const uint32 key_code = key.key_code();
  if ((key_code >> 8) > 0) {
    LOG(ERROR) << "key_code is not ASCII: " << key_code;
    return false;
  }
  const char key_char = static_cast<char>(key_code);
  char input[2] = " ";
  input[0] = key_char;

  if (key.has_key_string()) {
    if (key.input_style() == commands::KeyEvent::AS_IS ||
        key.input_style() == commands::KeyEvent::DIRECT_INPUT) {
      composition_->SetInputMode(
          Transliterators::GetConversionStringSelector());
      InsertCharacterKeyAndPreedit(input, key.key_string());
      SetInputMode(comeback_input_mode_);
    } else {
      // Kana input usually has key_string.  Note that, the existence of
      // key_string never determine if the input mode is Kana or Romaji.
      InsertCharacterKeyAndPreedit(input, key.key_string());
    }
  } else {
    // Romaji input usually does not has key_string.  Note that, the
    // existence of key_string never determine if the input mode is
    // Kana or Romaji.
    if ('A' <= key_char && key_char <= 'Z') {
      if (GET_CONFIG(shift_key_mode_switch) ==
          config::Config::ASCII_INPUT_MODE) {
        if (input_mode_ == transliteration::HALF_ASCII ||
            input_mode_ == transliteration::FULL_ASCII) {
          // Do nothing.
        } else {
          SetTemporaryInputMode(transliteration::HALF_ASCII);
        }
      } else if (GET_CONFIG(shift_key_mode_switch) ==
                 config::Config::KATAKANA_INPUT_MODE) {
        if (input_mode_ == transliteration::FULL_KATAKANA) {
          // Do nothing.
        } else {
          SetTemporaryInputMode(transliteration::FULL_KATAKANA);
        }
      }
      ++capital_sequence_count_;
    }
    // When capital input continues, the next lower input is the end
    // of temporary half-width Ascii input.
    if ('a' <= key_char && key_char <= 'z') {
      if (capital_sequence_count_ > 1 &&
          GET_CONFIG(shift_key_mode_switch) ==
          config::Config::ASCII_INPUT_MODE) {
        SetInputMode(comeback_input_mode_);
      }
      if (GET_CONFIG(shift_key_mode_switch) ==
          config::Config::KATAKANA_INPUT_MODE) {
        SetInputMode(comeback_input_mode_);
      }
      capital_sequence_count_ = 0;
    }
    InsertCharacter(input);
  }

  if (comeback_input_mode_ == input_mode_) {
    AutoSwitchMode();
  }
  return true;
}

void Composer::DeleteAt(size_t pos) {
  position_ = composition_->DeleteAt(pos);
  UpdateInputMode();
}

void Composer::Delete() {
  position_ = composition_->DeleteAt(position_);
  UpdateInputMode();
}

void Composer::EditErase() {
  composition_->Erase();
  position_ = 0;
  UpdateInputMode();
}

void Composer::BackspaceAt(size_t pos) {
  if (pos == 0) {
    return;
  }
  position_ = composition_->DeleteAt(pos - 1);
  UpdateInputMode();
}

void Composer::Backspace() {
  BackspaceAt(position_);
  UpdateInputMode();
}

void Composer::MoveCursorLeft() {
  if (position_ > 0) {
    --position_;
    UpdateInputMode();
  }
}

void Composer::MoveCursorRight() {
  if (position_ < composition_->GetLength()) {
    ++position_;
    UpdateInputMode();
  }
}

void Composer::MoveCursorToBeginning() {
  if (position_ != 0) {
    position_ = 0;
    UpdateInputMode();
  }
}

void Composer::MoveCursorToEnd() {
  const size_t cursor_end = composition_->GetLength();
  if (position_ != cursor_end) {
    position_ = cursor_end;
    UpdateInputMode();
  }
}

void Composer::GetPreedit(string *left, string *focused, string *right) const {
  DCHECK(left);
  DCHECK(focused);
  DCHECK(right);
  composition_->GetPreedit(position_, left, focused, right);

  // TODO(komatsu): This function can be obsolete.
  string preedit = *left + *focused + *right;
  if (TransformCharactersForNumbers(&preedit)) {
    const size_t left_size = Util::CharsLen(*left);
    const size_t focused_size = Util::CharsLen(*focused);
    *left = Util::SubString(preedit, 0, left_size);
    *focused = Util::SubString(preedit, left_size, focused_size);
    *right = Util::SubString(preedit, left_size + focused_size, string::npos);
  }
}

void Composer::GetStringForPreedit(string *output) const {
  composition_->GetString(output);
  TransformCharactersForNumbers(output);
}

void Composer::GetStringForSubmission(string *output) const {
  // TODO(komatsu): We should make sure if we can integrate this
  // function to GetStringForPreedit after a while.
  GetStringForPreedit(output);
}

void Composer::GetQueryForConversion(string *output) const {
  string base_output;
  composition_->GetStringWithTrimMode(FIX, &base_output);
  TransformCharactersForNumbers(&base_output);
  Util::FullWidthAsciiToHalfWidthAscii(base_output, output);
}

namespace {
// Determine which query is suitable for a prediction query and return
// its pointer.
// Exmaple:
// = Romanji Input =
// ("もz", "も") -> "も"  // a part of romanji should be trimed.
// ("z", "") -> "z"      // ditto.
// ("もzky", "もz") -> "もzky"  // a user might intentionally typed them.
// = Kana Input =
// ("か", "") -> "か"  // a part of kana (it can be "が") should not be trimed.
string *GetBaseQueryForPrediction(string *asis_query,
                                  string *trimed_query) {
  // If the sizes are equal, there is no matter.
  if (asis_query->size() == trimed_query->size()) {
    return asis_query;
  }

  // Get the different part between asis_query and trimed_query.  For
  // example, "ky" is the different part where asis_query is "もzky"
  // and trimed_query is "もz".
  DCHECK_GT(asis_query->size(), trimed_query->size());
  const string asis_tail = asis_query->substr(trimed_query->size());
  DCHECK(!asis_tail.empty());

  // If the different part is not an alphabet, asis_query is used.
  // This check is mainly used for Kana Input.
  const Util::ScriptType asis_tail_type = Util::GetScriptType(asis_tail);
  if (asis_tail_type != Util::ALPHABET) {
    return asis_query;
  }

  // If the trimed_query is emapy and asis_query is alphabet, an empty
  // string is used because the query is probably a part of romanji.
  if (trimed_query->empty()) {  // alphabet???
    return trimed_query;
  }

  // Now there are two patterns: ("もzk", "もz") and ("もずk", "もず").
  // We assume "もzk" is user's intentional query, but "もずk" is not.
  // So our results are:
  // ("もzk", "もz") => "もzk" and ("もずk", "もず") => "もず".
  const string trimed_tail = Util::SubString(*trimed_query,
                                             Util::CharsLen(*trimed_query) - 1,
                                             string::npos);
  DCHECK(!trimed_tail.empty());
  const Util::ScriptType trimed_tail_type = Util::GetScriptType(trimed_tail);
  if (trimed_tail_type == Util::ALPHABET) {
    return asis_query;
  } else {
    return trimed_query;
  }
}
}  // anonymous namespace

void Composer::GetQueryForPrediction(string *output) const {
  string asis_query;
  composition_->GetStringWithTrimMode(ASIS, &asis_query);

  string trimed_query;
  composition_->GetStringWithTrimMode(TRIM, &trimed_query);

  // NOTE(komatsu): This is a hack to go around the difference
  // expectation between Romanji-Input and Kana-Input.  "かn" in
  // Romaji-Input should be "か" while "あか" in Kana-Input should be
  // "あか", although "かn" and "あか" have the same properties.  An
  // ideal solution is to expand the ambguity and pass all of them to
  // the converter. (e.g. "かn" -> ["かな",..."かの", "かん", ...] /
  // "あか" -> ["あか", "あが"])
  string *base_query = GetBaseQueryForPrediction(&asis_query, &trimed_query);
  TransformCharactersForNumbers(base_query);
  Util::FullWidthAsciiToHalfWidthAscii(*base_query, output);
}

size_t Composer::GetLength() const {
  return composition_->GetLength();
}

size_t Composer::GetCursor() const {
  return position_;
}


void Composer::GetTransliterations(
    transliteration::Transliterations *t13ns) const {
  GetSubTransliterations(0, GetLength(), t13ns);
}

void Composer::GetSubTransliteration(
    const transliteration::TransliterationType type,
    const size_t position,
    const size_t size,
    string *transliteration) const {
  const TransliteratorInterface *t12r = GetTransliterator(type);
  const TransliteratorInterface *kNullT12r = NULL;

  string full_base;
  composition_->GetStringWithTransliterator(t12r, &full_base);

  const size_t t13n_start =
    composition_->ConvertPosition(position, kNullT12r, t12r);
  const size_t t13n_end =
    composition_->ConvertPosition(position + size, kNullT12r, t12r);
  const size_t t13n_size = t13n_end - t13n_start;

  const string sub_base = Util::SubString(full_base, t13n_start, t13n_size);
  transliteration->clear();
  Transliterate(type, sub_base, transliteration);
}

void Composer::GetSubTransliterations(
    const size_t position,
    const size_t size,
    transliteration::Transliterations *transliterations) const {
  string t13n;
  for (size_t i = 0; i < transliteration::NUM_T13N_TYPES; ++i) {
    const transliteration::TransliterationType t13n_type =
      transliteration::TransliterationTypeArray[i];
    GetSubTransliteration(t13n_type, position, size, &t13n);
    transliterations->push_back(t13n);
  }
}

bool Composer::EnableInsert() const {
  if (GetLength() >= max_length_) {
    // do not accept long chars to prevent DOS attack.
    LOG(WARNING) << "The length is too long.";
    return false;
  }
  return true;
}

void Composer::set_max_length(size_t length) {
  max_length_ = length;
}

size_t Composer::max_length() const {
  return max_length_;
}

void Composer::AutoSwitchMode() {
  if (!GET_CONFIG(use_auto_ime_turn_off)) {
    return;
  }

  // AutoSwitchMode is only available on Roma input
  if (GET_CONFIG(preedit_method) != config::Config::ROMAN) {
    return;
  }

  string key;
  // Key should be in half-width alphanumeric.
  composition_->GetStringWithTransliterator(
      GetTransliterator(transliteration::HALF_ASCII), &key);

  ModeSwitchingHandler::ModeSwitching display_mode =
      ModeSwitchingHandler::NO_CHANGE;
  ModeSwitchingHandler::ModeSwitching input_mode =
      ModeSwitchingHandler::NO_CHANGE;
  if (!ModeSwitchingHandler::GetModeSwitchingHandler()->GetModeSwitchingRule(
          key, &display_mode, &input_mode)) {
    // If the key is not a pattern of mode switch rule, the procedure
    // stops here.
    return;
  }

  // |display_mode| affects the existing composition the user typed.
  switch (display_mode) {
    case ModeSwitchingHandler::NO_CHANGE:
      // Do nothing.
      break;
    case ModeSwitchingHandler::REVERT_TO_PREVIOUS_MODE:
      // Invalid value for display_mode
      LOG(ERROR) << "REVERT_TO_PREVIOUS_MODE is an invalid value "
                 << "for display_mode.";
      break;
    case ModeSwitchingHandler::PREFERRED_ALPHANUMERIC:
      if (input_mode_ == transliteration::FULL_ASCII) {
        SetOutputMode(transliteration::FULL_ASCII);
      } else {
        SetOutputMode(transliteration::HALF_ASCII);
      }
      break;
    case ModeSwitchingHandler::HALF_ALPHANUMERIC:
      SetOutputMode(transliteration::HALF_ASCII);
      break;
    case ModeSwitchingHandler::FULL_ALPHANUMERIC:
      SetOutputMode(transliteration::FULL_ASCII);
      break;
    default:
      LOG(ERROR) << "Unkown value: " << display_mode;
      break;
  }

  // |input_mode| affects the current input mode used for the user's
  // new typing.
  switch (input_mode) {
    case ModeSwitchingHandler::NO_CHANGE:
      // Do nothing.
      break;
    case ModeSwitchingHandler::REVERT_TO_PREVIOUS_MODE:
      SetInputMode(comeback_input_mode_);
      break;
    case ModeSwitchingHandler::PREFERRED_ALPHANUMERIC:
      if (input_mode_ != transliteration::HALF_ASCII &&
          input_mode_ != transliteration::FULL_ASCII) {
        SetTemporaryInputMode(transliteration::HALF_ASCII);
      }
      break;
    case ModeSwitchingHandler::HALF_ALPHANUMERIC:
      if (input_mode_ != transliteration::HALF_ASCII) {
        SetTemporaryInputMode(transliteration::HALF_ASCII);
      }
      break;
    case ModeSwitchingHandler::FULL_ALPHANUMERIC:
      if (input_mode_ != transliteration::FULL_ASCII) {
        SetTemporaryInputMode(transliteration::FULL_ASCII);
      }
      break;
    default:
      LOG(ERROR) << "Unkown value: " << display_mode;
      break;
  }
}

// static
Composer *Composer::Create(const Table *table) {
  Composition *composition = new Composition;
  composition->SetTable(table);
  Composer *composer = new Composer(composition);
  return composer;
}

namespace {
enum Script {
  ALPHANUM,     // 0 - 9, "０" - "９", alphabet characters.
  JA_HYPHEN,  // "ー"
  JA_COMMA,   // "、"
  JA_PERIOD,  // "。"
  OTHERS,
};

static const char *kNumberSymbols[] = {
  "+", "*", "/", "=", "(", ")", "<", ">",
  // "＋", "＊", "／", "＝",
  "\xEF\xBC\x8B", "\xEF\xBC\x8A", "\xEF\xBC\x8F", "\xEF\xBC\x9D",
  // "（", "）", "＜", "＞",
  "\xEF\xBC\x88", "\xEF\xBC\x89", "\xEF\xBC\x9C", "\xEF\xBC\x9E",
};
}  // anonymous namespace

// static
bool Composer::TransformCharactersForNumbers(string *query) {
  if (query == NULL) {
    LOG(ERROR) << "query is NULL";
    return false;
  }

  // Create a vector of scripts of query characters to avoid
  // processing query string many times.
  const size_t chars_len = Util::CharsLen(*query);
  vector<Script> char_scripts;
  // flags to determine whether continue to the next step.
  bool has_symbols = false;
  bool has_numbers = false;
  for (size_t i = 0; i < chars_len; ++i) {
    Script script = OTHERS;
    const string one_char = Util::SubString(*query, i, 1);
    if (one_char == "\xE3\x83\xBC") {  // "ー"
      has_symbols = true;
      script = JA_HYPHEN;
    } else if (one_char == "\xE3\x80\x81") {  // "、"
      has_symbols = true;
      script = JA_COMMA;
    } else if (one_char == "\xE3\x80\x82") {  // "。"
      has_symbols = true;
      script = JA_PERIOD;
    } else if (Util::IsScriptType(one_char, Util::NUMBER)) {
      has_numbers = true;
      script = ALPHANUM;
    } else if (Util::IsScriptType(one_char, Util::ALPHABET)) {
      has_numbers = true;
      script = ALPHANUM;
    } else {
      for (size_t j = 0; j < arraysize(kNumberSymbols); ++j) {
        if (one_char == kNumberSymbols[j]) {
          script = ALPHANUM;
          break;
        }
      }
    }
    char_scripts.push_back(script);
  }
  DCHECK_EQ(chars_len, char_scripts.size());
  if (!has_numbers || !has_symbols) {
    VLOG(1) << "The query does not contain number, symbol or neither.";
    return false;
  }

  string transformed_query;
  bool transformed = false;
  for (size_t i = 0; i < chars_len; ++i) {
    const Script script = char_scripts[i];
    if (script == OTHERS || script == ALPHANUM) {
      // Append one character.
      Util::SubString(*query, i, 1, &transformed_query);
      continue;
    }

    // "ー" should be "−" if the previous character is a digit, or the
    // previous character is empty and the next character is a digit.
    if (script == JA_HYPHEN) {
      bool check = false;
      // If there is no previous char, the next character decides the
      // transform.
      if (i == 0) {
        // Next char should exist and be a number.
        if (i+1 < chars_len && char_scripts[i+1] == ALPHANUM) {
          check = true;
        } else if (i+2 < chars_len &&
                   char_scripts[i+1] == JA_HYPHEN &&
                   char_scripts[i+2] == ALPHANUM) {
          // Otherwise, the next character should also be JA_HYPHEN
          // and the i+2 th characters should be an alphanumeric (eg. 1--2).
          check = true;
        }
      } else {
        // If previous character(s) is an Alphanumeric or sequence of
        // JA_HYPHEN terminated with an Alphanumeric, the target
        // JA_HYPHEN should be transformed (ex. 0-, 0-----, etc).
        for (size_t j = i; j > 0; --j) {
          if (char_scripts[j - 1] == JA_HYPHEN) {
            continue;
          }
          if (char_scripts[j - 1] == ALPHANUM) {
            check = true;
          }
          break;
        }
      }

      // JA_HYPHEN should be transformed to MINUS.
      if (check) {
        string append_char;
        CharacterFormManager::GetCharacterFormManager()->
            ConvertPreeditString("\xE2\x88\x92", &append_char);  // "−"
        transformed_query.append(append_char);
        transformed = true;
      } else {
        // Append one character.
        Util::SubString(*query, i, 1, &transformed_query);
      }
      continue;
    }

    // "、" should be "，" if the previous character and the next
    // character are both digits.
    if (script == JA_COMMA) {
      // Previous char should exist and be a number.
      const bool lhs_check = (i > 0 && char_scripts[i-1] == ALPHANUM);
      // Next three characters should exist and be all digits.
      const bool rhs_check = (i+1 < chars_len && char_scripts[i+1] == ALPHANUM);
      // JA_COMMA should be transformed to COMMA.
      if (lhs_check && rhs_check) {
        string append_char;
        CharacterFormManager::GetCharacterFormManager()->
            ConvertPreeditString("\xEF\xBC\x8C", &append_char);  // "，"
        transformed_query.append(append_char);
        transformed = true;
      } else {
        // Append one character.
        Util::SubString(*query, i, 1, &transformed_query);
      }
      continue;
    }

    // "。" should be "．" if the previous character and the next
    // character are both digits.
    if (script == JA_PERIOD) {
      // Previous char should exist and be a number.
      const bool lhs_check = (i > 0 && char_scripts[i-1] == ALPHANUM);
      // Next three characters should exist and be a digit.
      const bool rhs_check = (i+1 < chars_len && char_scripts[i+1] == ALPHANUM);

      // JA_PRERIOD should be transformed to PRERIOD.
      if (lhs_check && rhs_check) {
        string append_char;
        CharacterFormManager::GetCharacterFormManager()->
            ConvertPreeditString("\xEF\xBC\x8E", &append_char);  // "．"
        transformed_query.append(append_char);
        transformed = true;
      } else {
        // Append one character.
        Util::SubString(*query, i, 1, &transformed_query);
      }
      continue;
    }
    DCHECK(false) << "Should not come here.";
  }
  if (!transformed) {
    return false;
  }

  // It is possible that the query's size in byte differs from the
  // orig_query's size in byte.
  DCHECK_EQ(Util::CharsLen(*query), Util::CharsLen(transformed_query));
  *query = transformed_query;
  return true;
}

}  // namespace composer
}  // namespace mozc
