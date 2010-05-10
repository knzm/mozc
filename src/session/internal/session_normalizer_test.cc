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

#include <string>
#include "base/base.h"
#include "base/util.h"
#include "session/internal/session_normalizer.h"
#include "testing/base/public/gunit.h"

namespace mozc {
namespace session {

TEST(SessionNormalizerTest, NormalizePreeditText) {
  string output;
  // "めかぶ"
  SessionNormalizer::NormalizePreeditText(
      "\xe3\x82\x81\xe3\x81\x8b\xe3\x81\xb6", &output);
  // "めかぶ"
  EXPECT_EQ("\xe3\x82\x81\xe3\x81\x8b\xe3\x81\xb6", output);

  // "ゔぁいおりん"
  SessionNormalizer::NormalizePreeditText("\xe3\x82\x94\xe3\x81\x81\xe3\x81\x84"
                                          "\xe3\x81\x8a\xe3\x82\x8a\xe3\x82"
                                          "\x93", &output);
  // "ヴぁいおりん"
  EXPECT_EQ("\xe3\x83\xb4\xe3\x81\x81\xe3\x81\x84\xe3\x81\x8a\xe3\x82\x8a\xe3"
            "\x82\x93", output);

  // "ぐ〜ぐる"
  SessionNormalizer::NormalizePreeditText("\xe3\x81\x90\xe3\x80\x9c\xe3\x81\x90"
                                          "\xe3\x82\x8b", &output);
#ifdef OS_WINDOWS
  // "ぐ～ぐる"
  EXPECT_EQ("\xe3\x81\x90\xef\xbd\x9e\xe3\x81\x90\xe3\x82\x8b", output);
#else
  // "ぐ〜ぐる"
  EXPECT_EQ("\xe3\x81\x90\xe3\x80\x9c\xe3\x81\x90\xe3\x82\x8b", output);
#endif
}

TEST(SessionNormalizerTest, NormalizeTransliterationText) {
  string output;
  // "めかぶ"
  SessionNormalizer::NormalizeTransliterationText(
      "\xe3\x82\x81\xe3\x81\x8b\xe3\x81\xb6", &output);
  // "めかぶ"
  EXPECT_EQ("\xe3\x82\x81\xe3\x81\x8b\xe3\x81\xb6", output);

  // "ゔぁいおりん"
  SessionNormalizer::NormalizeTransliterationText(
      "\xe3\x82\x94\xe3\x81\x81\xe3\x81\x84"
      "\xe3\x81\x8a\xe3\x82\x8a\xe3\x82\x93",
      &output);
  // "ヴぁいおりん"
  EXPECT_EQ("\xe3\x83\xb4\xe3\x81\x81\xe3\x81\x84\xe3\x81\x8a\xe3\x82\x8a\xe3"
            "\x82\x93", output);

  // "ぐ〜ぐる"
  SessionNormalizer::NormalizeTransliterationText(
      "\xe3\x81\x90\xe3\x80\x9c\xe3\x81\x90\xe3\x82\x8b", &output);
#ifdef OS_WINDOWS
  // "ぐ～ぐる"
  EXPECT_EQ("\xe3\x81\x90\xef\xbd\x9e\xe3\x81\x90\xe3\x82\x8b", output);
#else
  // "ぐ〜ぐる"
  EXPECT_EQ("\xe3\x81\x90\xe3\x80\x9c\xe3\x81\x90\xe3\x82\x8b", output);
#endif
}

TEST(SessionNormalizerTest, NormalizeCandidateText) {
#ifdef OS_WINDOWS
  string output;
  // "ぐ〜ぐる"
  SessionNormalizer::NormalizeCandidateText("\xe3\x81\x90\xe3\x80\x9c\xe3\x81"
                                            "\x90\xe3\x82\x8b", &output);
  // "ぐ～ぐる"
  EXPECT_EQ("\xe3\x81\x90\xef\xbd\x9e\xe3\x81\x90\xe3\x82\x8b", output);
  // "−"
  SessionNormalizer::NormalizeCandidateText("\xe2\x88\x92", &output);
  // "－"
  EXPECT_EQ("\xef\xbc\x8d", output);
  // "¢"
  SessionNormalizer::NormalizeCandidateText("\xc2\xa2", &output);
  // "￠"
  EXPECT_EQ("\xef\xbf\xa0", output);
  // "‖"
  SessionNormalizer::NormalizeCandidateText("\xe2\x80\x96", &output);
  // "∥"
  EXPECT_EQ("\xe2\x88\xa5", output);
#endif
}
}  // namespace session
}  // namespace mozc
