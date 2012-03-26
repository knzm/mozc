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

#include <string>
#include <vector>
#include "base/base.h"
#include "base/util.h"
#include "composer/composer.h"
#include "composer/table.h"
#include "config/config.pb.h"
#include "config/config_handler.h"
#include "converter/converter.h"
#include "converter/converter_interface.h"
#include "converter/immutable_converter_interface.h"
#include "converter/node_allocator.h"
#include "converter/segments.h"
#include "converter/user_data_manager_interface.h"
#include "dictionary/pos_matcher.h"
#include "dictionary/suffix_dictionary.h"
#include "dictionary/suppression_dictionary.h"
#include "prediction/predictor.h"
#include "testing/base/public/gunit.h"
#include "transliteration/transliteration.h"
#include "rewriter/rewriter_interface.h"

#ifdef MOZC_USE_SEPARATE_CONNECTION_DATA
#include "converter/connection_data_injected_environment.h"
namespace {
const ::testing::Environment *kConnectionDataInjectedEnvironment =
    ::testing::AddGlobalTestEnvironment(
        new ::mozc::ConnectionDataInjectedEnvironment());
}  // namespace
#endif  // MOZC_USE_SEPARATE_CONNECTION_DATA

#ifdef MOZC_USE_SEPARATE_DICTIONARY
#include "dictionary/dictionary_data_injected_environment.h"
namespace {
const ::testing::Environment *kDictionaryDataInjectedEnvironment =
    ::testing::AddGlobalTestEnvironment(
        new ::mozc::DictionaryDataInjectedEnvironment());
}  // namespace
#endif  // MOZC_USE_SEPARATE_DICTIONARY

DECLARE_string(test_tmpdir);

namespace mozc {

class ConverterTest : public testing::Test {
 public:
  virtual void SetUp() {

    // set default user profile directory
    Util::SetUserProfileDirectory(FLAGS_test_tmpdir);
    config::Config config;
    config::ConfigHandler::GetDefaultConfig(&config);
    config::ConfigHandler::SetConfig(config);
  }

  virtual void TearDown() {

    // just in case, reset the config in test_tmpdir
    config::Config config;
    config::ConfigHandler::GetDefaultConfig(&config);
    config::ConfigHandler::SetConfig(config);
    SuffixDictionaryFactory::SetSuffixDictionary(NULL);

    PredictorFactory::SetPredictor(NULL);
    RewriterFactory::SetRewriter(NULL);
  }

};

// test for issue:2209644
// just checking whether this causes segmentation fault or not.
// TODO(toshiyuki): make dictionary mock and test strictly.
TEST_F(ConverterTest, CanConvertTest) {
  ConverterInterface *converter = ConverterFactory::GetConverter();
  CHECK(converter);
  {
    Segments segments;
    EXPECT_TRUE(converter->StartConversion(&segments, string("-")));
  }
  {
    Segments segments;
    // "おきておきて"
    EXPECT_TRUE(converter->StartConversion(
        &segments,
        string("\xe3\x81\x8a\xe3\x81\x8d\xe3\x81\xa6\xe3\x81\x8a"
               "\xe3\x81\x8d\xe3\x81\xa6")));
  }
}

namespace {
string ContextAwareConvert(const string &first_key,
                           const string &first_value,
                           const string &second_key) {
  ConverterInterface *converter = ConverterFactory::GetConverter();
  CHECK(converter);
  converter->GetUserDataManager()->ClearUserHistory();

  Segments segments;
  EXPECT_TRUE(converter->StartConversion(&segments, first_key));

  int position = -1;
  for (size_t i = 0; i < segments.segment(0).candidates_size(); ++i) {
    if (first_value == segments.segment(0).candidate(i).value) {
      position = static_cast<int>(i);
      break;
    }
  }
  EXPECT_GE(position, 0) << first_value;

  EXPECT_TRUE(converter->CommitSegmentValue(&segments, 0, position))
      << first_value;
  EXPECT_TRUE(converter->FinishConversion(&segments));

  EXPECT_TRUE(converter->StartConversion(&segments, second_key));
  EXPECT_EQ(2, segments.segments_size());

  return segments.segment(1).candidate(0).value;
}
}  // anonymous namespace

TEST_F(ConverterTest, ContextAwareConversionTest) {
  // EXPECT_EQ("一髪", ContextAwareConvert("きき", "危機", "いっぱつ"));
  // EXPECT_EQ("大", ContextAwareConvert("きょうと", "京都", "だい"));
  // EXPECT_EQ("点", ContextAwareConvert("もんだい", "問題", "てん"));
  // EXPECT_EQ("陽水", ContextAwareConvert("いのうえ", "井上", "ようすい"));
  // EXPECT_EQ("々", ContextAwareConvert("ねん", "年", "ねん"));
  // EXPECT_EQ("々", ContextAwareConvert("ひと", "人", "びと"));

  EXPECT_EQ("\xE4\xB8\x80\xE9\xAB\xAA",
            ContextAwareConvert(
                "\xE3\x81\x8D\xE3\x81\x8D",
                "\xE5\x8D\xB1\xE6\xA9\x9F",
                "\xE3\x81\x84\xE3\x81\xA3\xE3\x81\xB1\xE3\x81\xA4"));
  EXPECT_EQ("\xE5\xA4\xA7",
            ContextAwareConvert(
                "\xE3\x81\x8D\xE3\x82\x87\xE3\x81\x86\xE3\x81\xA8",
                "\xE4\xBA\xAC\xE9\x83\xBD",
                "\xE3\x81\xA0\xE3\x81\x84"));
  EXPECT_EQ("\xE7\x82\xB9",
            ContextAwareConvert(
                "\xE3\x82\x82\xE3\x82\x93\xE3\x81\xA0\xE3\x81\x84",
                "\xE5\x95\x8F\xE9\xA1\x8C",
                "\xE3\x81\xA6\xE3\x82\x93"));
  EXPECT_EQ("\xE9\x99\xBD\xE6\xB0\xB4",
            ContextAwareConvert(
                "\xE3\x81\x84\xE3\x81\xAE\xE3\x81\x86\xE3\x81\x88",
                "\xE4\xBA\x95\xE4\xB8\x8A",
                "\xE3\x82\x88\xE3\x81\x86\xE3\x81\x99\xE3\x81\x84"));
  EXPECT_EQ("\xE3\x80\x85",
            ContextAwareConvert(
                "\xE3\x81\xAD\xE3\x82\x93",
                "\xE5\xB9\xB4",
                "\xE3\x81\xAD\xE3\x82\x93"));
  EXPECT_EQ("\xE3\x80\x85",
            ContextAwareConvert(
                "\xE3\x81\xB2\xE3\x81\xA8",
                "\xE4\xBA\xBA",
                "\xE3\x81\xB3\xE3\x81\xA8"));
}

TEST_F(ConverterTest, CommitSegmentValue) {
  ConverterInterface *converter = ConverterFactory::GetConverter();
  CHECK(converter);
  Segments segments;

  {
    // Prepare a segment, with candidates "1" and "2";
    Segment *segment = segments.add_segment();
    segment->add_candidate()->value = "1";
    segment->add_candidate()->value = "2";
  }
  {
    // Prepare a segment, with candidates "3" and "4";
    Segment *segment = segments.add_segment();
    segment->add_candidate()->value = "3";
    segment->add_candidate()->value = "4";
  }
  {
    // Commit the candidate whose value is "2".
    EXPECT_TRUE(converter->CommitSegmentValue(&segments, 0, 1));
    EXPECT_EQ(2, segments.segments_size());
    EXPECT_EQ(0, segments.history_segments_size());
    EXPECT_EQ(2, segments.conversion_segments_size());
    const Segment &segment = segments.conversion_segment(0);
    EXPECT_EQ(Segment::FIXED_VALUE, segment.segment_type());
    EXPECT_EQ("2", segment.candidate(0).value);
    EXPECT_TRUE(
        segment.candidate(0).attributes & Segment::Candidate::RERANKED);
  }
  {
    // Make the segment SUBMITTED
    segments.mutable_conversion_segment(0)->
        set_segment_type(Segment::SUBMITTED);
    EXPECT_EQ(2, segments.segments_size());
    EXPECT_EQ(1, segments.history_segments_size());
    EXPECT_EQ(1, segments.conversion_segments_size());
  }
  {
    // Commit the candidate whose value is "3".
    EXPECT_TRUE(converter->CommitSegmentValue(&segments, 0, 0));
    EXPECT_EQ(2, segments.segments_size());
    EXPECT_EQ(1, segments.history_segments_size());
    EXPECT_EQ(1, segments.conversion_segments_size());
    const Segment &segment = segments.conversion_segment(0);
    EXPECT_EQ(Segment::FIXED_VALUE, segment.segment_type());
    EXPECT_EQ("3", segment.candidate(0).value);
    EXPECT_FALSE(
        segment.candidate(0).attributes & Segment::Candidate::RERANKED);
  }
}

TEST_F(ConverterTest, CommitPartialSuggestionSegmentValue) {
  ConverterInterface *converter = ConverterFactory::GetConverter();
  CHECK(converter);
  Segments segments;

  {
    // Prepare a segment, with candidates "1" and "2";
    Segment *segment = segments.add_segment();
    segment->add_candidate()->value = "1";
    segment->add_candidate()->value = "2";
  }
  {
    // Prepare a segment, with candidates "3" and "4";
    Segment *segment = segments.add_segment();
    segment->add_candidate()->value = "3";
    segment->add_candidate()->value = "4";
  }
  {
    // Commit the candidate whose value is "2".
    EXPECT_TRUE(converter->CommitPartialSuggestionSegmentValue(
                &segments, 0, 1, "left2", "right2"));
    EXPECT_EQ(3, segments.segments_size());
    EXPECT_EQ(1, segments.history_segments_size());
    EXPECT_EQ(2, segments.conversion_segments_size());
    {
      // The tail segment of the history segments uses
      // CommitPartialSuggestionSegmentValue's |current_segment_key| parameter
      // and contains original value.
      const Segment &segment =
          segments.history_segment(segments.history_segments_size() - 1);
      EXPECT_EQ(Segment::SUBMITTED, segment.segment_type());
      EXPECT_EQ("2", segment.candidate(0).value);
      EXPECT_EQ("left2", segment.key());
      EXPECT_TRUE(
          segment.candidate(0).attributes & Segment::Candidate::RERANKED);
    }
    {
      // The head segment of the conversion segments uses |new_segment_key|.
      const Segment &segment = segments.conversion_segment(0);
      EXPECT_EQ(Segment::FREE, segment.segment_type());
      EXPECT_EQ("right2", segment.key());
    }
  }
}

TEST_F(ConverterTest, CandidateKeyTest) {
  ConverterInterface *converter = ConverterFactory::GetConverter();
  CHECK(converter);
  Segments segments;
  // "わたしは"
  EXPECT_TRUE(converter->StartConversion(
      &segments,
      string("\xE3\x82\x8F\xE3\x81\x9F\xE3\x81\x97\xE3\x81\xAF")));
  EXPECT_EQ(1, segments.segments_size());
  // "わたしは"
  EXPECT_EQ("\xE3\x82\x8F\xE3\x81\x9F\xE3\x81\x97\xE3\x81\xAF",
            segments.segment(0).candidate(0).key);
  // "わたし"
  EXPECT_EQ("\xE3\x82\x8F\xE3\x81\x9F\xE3\x81\x97",
            segments.segment(0).candidate(0).content_key);
}

TEST_F(ConverterTest, QueryOfDeathTest) {
  ConverterInterface *converter = ConverterFactory::GetConverter();
  CHECK(converter);
  Segments segments;
  // "りゅきゅけmぽ"
  EXPECT_TRUE(converter->StartConversion(
      &segments,
      "\xE3\x82\x8A\xE3\x82\x85"
      "\xE3\x81\x8D\xE3\x82\x85"
      "\xE3\x81\x91"
      "m"
      "\xE3\x81\xBD"));
}

TEST_F(ConverterTest, Regression3323108) {
  ConverterInterface *converter = ConverterFactory::GetConverter();
  Segments segments;

  // "ここではきものをぬぐ"
  EXPECT_TRUE(converter->StartConversion(
      &segments,
      "\xE3\x81\x93\xE3\x81\x93\xE3\x81\xA7"
      "\xE3\x81\xAF\xE3\x81\x8D\xE3\x82\x82"
      "\xE3\x81\xAE\xE3\x82\x92\xE3\x81\xAC"
      "\xE3\x81\x90"));
  EXPECT_EQ(3, segments.conversion_segments_size());
  EXPECT_TRUE(converter->ResizeSegment(
      &segments, ConversionRequest(), 1, 2));
  EXPECT_EQ(2, segments.conversion_segments_size());

  // "きものをぬぐ"
  EXPECT_EQ("\xE3\x81\x8D\xE3\x82\x82"
            "\xE3\x81\xAE\xE3\x82\x92"
            "\xE3\x81\xAC\xE3\x81\x90",
            segments.conversion_segment(1).key());
}

TEST_F(ConverterTest, Regression3437022) {
  ConverterInterface *converter = ConverterFactory::GetConverter();
  Segments segments;

  // "けいたい"
  const string kKey1 = "\xe3\x81\x91\xe3\x81\x84\xe3\x81\x9f\xe3\x81\x84";
  // "でんわ"
  const string kKey2 = "\xe3\x81\xa7\xe3\x82\x93\xe3\x82\x8f";

  // "携帯"
  const string kValue1 = "\xe6\x90\xba\xe5\xb8\xaf";
  // "電話"
  const string kValue2 = "\xe9\x9b\xbb\xe8\xa9\xb1";

  {
    // Make sure converte result is one segment
    EXPECT_TRUE(converter->StartConversion(
        &segments, kKey1 + kKey2));
    EXPECT_EQ(1, segments.conversion_segments_size());
    EXPECT_EQ(kValue1 + kValue2,
              segments.conversion_segment(0).candidate(0).value);
  }
  {
    // Make sure we can convert first part
    segments.Clear();
    EXPECT_TRUE(converter->StartConversion(
        &segments, kKey1));
    EXPECT_EQ(1, segments.conversion_segments_size());
    EXPECT_EQ(kValue1, segments.conversion_segment(0).candidate(0).value);
  }
  {
    // Make sure we can convert last part
    segments.Clear();
    EXPECT_TRUE(converter->StartConversion(
        &segments, kKey2));
    EXPECT_EQ(1, segments.conversion_segments_size());
    EXPECT_EQ(kValue2, segments.conversion_segment(0).candidate(0).value);
  }

  // Add compound entry to suppression dictioanry
  segments.Clear();

  SuppressionDictionary *dic
      = SuppressionDictionary::GetSuppressionDictionary();
  dic->Lock();
  dic->AddEntry(kKey1 + kKey2, kValue1 + kValue2);
  dic->UnLock();

  EXPECT_TRUE(converter->StartConversion(
      &segments, kKey1 + kKey2));

  EXPECT_NE(1, segments.conversion_segments_size());

  int rest_size = 0;
  for (size_t i = 1; i < segments.conversion_segments_size(); ++i) {
    rest_size += Util::CharsLen(
        segments.conversion_segment(i).candidate(0).key);
  }

  // Expand segment so that the entire part will become one segment
  EXPECT_TRUE(converter->ResizeSegment(
      &segments, ConversionRequest(), 0, rest_size));

  EXPECT_EQ(1, segments.conversion_segments_size());
  EXPECT_NE(kValue1 + kValue2,
            segments.conversion_segment(0).candidate(0).value);

  dic->Lock();
  dic->Clear();
  dic->UnLock();
}

TEST_F(ConverterTest, CompletePOSIds) {
  const char *kTestKeys[] = {
    // "きょうと",
    "\xE3\x81\x8D\xE3\x82\x87\xE3\x81\x86\xE3\x81\xA8",
    // "いきます",
    "\xE3\x81\x84\xE3\x81\x8D\xE3\x81\xBE\xE3\x81\x99",
    // "うつくしい",
    "\xE3\x81\x86\xE3\x81\xA4\xE3\x81\x8F\xE3\x81\x97\xE3\x81\x84",
    // "おおきな",
    "\xE3\x81\x8A\xE3\x81\x8A\xE3\x81\x8D\xE3\x81\xAA",
    // "いっちゃわない"
    "\xE3\x81\x84\xE3\x81\xA3\xE3\x81\xA1\xE3\x82\x83\xE3\x82\x8F"
    "\xE3\x81\xAA\xE3\x81\x84\xE3\x81\xAD",
    // "わたしのなまえはなかのです"
    "\xE3\x82\x8F\xE3\x81\x9F\xE3\x81\x97\xE3\x81\xAE\xE3\x81\xAA"
    "\xE3\x81\xBE\xE3\x81\x88\xE3\x81\xAF\xE3\x81\xAA\xE3\x81\x8B"
    "\xE3\x81\xAE\xE3\x81\xA7\xE3\x81\x99"
  };

  scoped_ptr<ConverterImpl> converter(new ConverterImpl);
  for (size_t i = 0; i < arraysize(kTestKeys); ++i) {
    Segments segments;
    segments.set_request_type(Segments::PREDICTION);
    mozc::Segment *seg = segments.add_segment();
    seg->set_key(kTestKeys[i]);
    seg->set_segment_type(mozc::Segment::FREE);
    segments.set_max_prediction_candidates_size(20);
    CHECK(ImmutableConverterFactory::GetImmutableConverter()->
          Convert(&segments));
    const int lid = segments.segment(0).candidate(0).lid;
    const int rid = segments.segment(0).candidate(0).rid;
    Segment::Candidate candidate;
    candidate.value = segments.segment(0).candidate(0).value;
    candidate.key = segments.segment(0).candidate(0).key;
    candidate.lid = 0;
    candidate.rid = 0;
    converter->CompletePOSIds(&candidate);
    EXPECT_EQ(lid, candidate.lid);
    EXPECT_EQ(rid, candidate.rid);
    EXPECT_NE(candidate.lid, 0);
    EXPECT_NE(candidate.rid, 0);
  }

  {
    Segment::Candidate candidate;
    candidate.key = "test";
    candidate.value = "test";
    candidate.lid = 10;
    candidate.rid = 11;
    converter->CompletePOSIds(&candidate);
    EXPECT_EQ(10, candidate.lid);
    EXPECT_EQ(11, candidate.rid);
  }
}

TEST_F(ConverterTest, SetupHistorySegmentsFromPrecedingText) {
  scoped_ptr<ConverterImpl> converter(new ConverterImpl);

  // Test for short preceding text.
  {
    Segments segments;
    segments.set_max_history_segments_size(4);
    converter->SetupHistorySegmentsFromPrecedingText(
        "\xE7\xA7\x81\xE3\x81\xAF\xE9\x88\xB4\xE6\x9C\xA8",  // "私は鈴木"
        &segments);
    EXPECT_EQ(2, segments.history_segments_size());

    // Check the first segment
    EXPECT_EQ(Segment::HISTORY, segments.segment(0).segment_type());
    EXPECT_EQ(1, segments.segment(0).candidates_size());
    EXPECT_EQ("\xE3\x82\x8F\xE3\x81\x9F\xE3\x81\x97\xE3\x81\xAF",  // "わたしは"
              segments.segment(0).candidate(0).key);
    EXPECT_EQ("\xE7\xA7\x81\xE3\x81\xAF",  // "私は"
              segments.segment(0).candidate(0).value);

    // Check the second segment
    EXPECT_EQ(Segment::HISTORY, segments.segment(1).segment_type());
    EXPECT_EQ(1, segments.segment(1).candidates_size());
    EXPECT_EQ("\xE3\x81\x99\xE3\x81\x9A\xE3\x81\x8D",  // "すずき"
              segments.segment(1).candidate(0).key);
    EXPECT_EQ("\xE9\x88\xB4\xE6\x9C\xA8",  // "鈴木"
              segments.segment(1).candidate(0).value);
  }
  // Test for long preceding text having 6 segments. The results should have 4
  // history segments.
  {
    Segments segments;
    segments.set_max_history_segments_size(4);
    converter->SetupHistorySegmentsFromPrecedingText(
        // "私は鈴木私は鈴木私は鈴木"
        "\xE7\xA7\x81\xE3\x81\xAF\xE9\x88\xB4\xE6\x9C\xA8"
        "\xE7\xA7\x81\xE3\x81\xAF\xE9\x88\xB4\xE6\x9C\xA8"
        "\xE7\xA7\x81\xE3\x81\xAF\xE9\x88\xB4\xE6\x9C\xA8",
        &segments);
    EXPECT_EQ(4, segments.history_segments_size());

    for (int i = 0; i < 4; ++i) {
      EXPECT_EQ(Segment::HISTORY, segments.segment(i).segment_type());
      // Check the first and third segments
      if (i % 2 == 0) {
        EXPECT_EQ(1, segments.segment(i).candidates_size());
        EXPECT_EQ(
            "\xE3\x82\x8F\xE3\x81\x9F\xE3\x81\x97\xE3\x81\xAF",  // "わたしは"
            segments.segment(i).candidate(0).key);
        EXPECT_EQ("\xE7\xA7\x81\xE3\x81\xAF",  // "私は"
                  segments.segment(i).candidate(0).value);
      } else {
        // Check the second and fourth segments
        EXPECT_EQ(1, segments.segment(i).candidates_size());
        EXPECT_EQ("\xE3\x81\x99\xE3\x81\x9A\xE3\x81\x8D",  // "すずき"
                  segments.segment(i).candidate(0).key);
        EXPECT_EQ("\xE9\x88\xB4\xE6\x9C\xA8",  // "鈴木"
                  segments.segment(i).candidate(0).value);
      }
    }
  }
}

TEST_F(ConverterTest, ConvertUsingPrecedingText_KikiIppatsu) {
  ConverterInterface *converter = ConverterFactory::GetConverter();
  // To see preceding text helps conversion, consider the case where user
  // converts "いっぱつ".
  {
    // Without preceding text, test dictionary converts "いっぱつ" to "一発".
    Segments segments;
    mozc::composer::Composer composer;
    composer.InsertCharacter(
        "\xE3\x81\x84\xE3\x81\xA3\xE3\x81\xB1\xE3\x81\xA4");  // "いっぱつ"
    mozc::ConversionRequest request(&composer);
    converter->StartConversionForRequest(request, &segments);

    EXPECT_EQ(1, segments.conversion_segments_size());
    EXPECT_EQ("\xE4\xB8\x80\xE7\x99\xBA",  // "一発"
              segments.conversion_segment(0).candidate(0).value);
  }
  {
    // However, with preceding text "危機", test dictionary converts "いっぱつ"
    // to "一髪".
    Segments segments;
    mozc::composer::Composer composer;
    composer.InsertCharacter(
        "\xE3\x81\x84\xE3\x81\xA3\xE3\x81\xB1\xE3\x81\xA4");  // "いっぱつ"
    mozc::ConversionRequest request(&composer);
    request.set_preceding_text("\xE5\x8D\xB1\xE6\xA9\x9F");  // "危機"
    converter->StartConversionForRequest(request, &segments);

    EXPECT_EQ(1, segments.conversion_segments_size());
    EXPECT_EQ("\xE4\xB8\x80\xE9\xAB\xAA",  // "一髪"
              segments.conversion_segment(0).candidate(0).value);
  }
}

TEST_F(ConverterTest, ConvertUsingPrecedingText_Jyosushi) {
  ConverterInterface *converter = ConverterFactory::GetConverter();
  // To see preceding text helps conversion after number characters, consider
  // the case where user converts "ひき".
  {
    // Without preceding text, test dictionary converts "ひき" to "引き".
    Segments segments;
    mozc::composer::Composer composer;
    composer.InsertCharacter("\xE3\x81\xB2\xE3\x81\x8D");  // "ひき"
    mozc::ConversionRequest request(&composer);
    converter->StartConversionForRequest(request, &segments);

    EXPECT_EQ(1, segments.conversion_segments_size());
    EXPECT_EQ("\xE5\xBC\x95\xE3\x81\x8D",  // "引き"
              segments.conversion_segment(0).candidate(0).value);
  }
  {
    // However, if providing "猫が5" as preceding text, "ひき" is converted to
    // "匹" with test dictionary.
    Segments segments;
    mozc::composer::Composer composer;
    composer.InsertCharacter("\xE3\x81\xB2\xE3\x81\x8D");  // "ひき"
    mozc::ConversionRequest request(&composer);
    request.set_preceding_text("\xE7\x8C\xAB\xE3\x81\x8C\x35");  // "猫が5"
    converter->StartConversionForRequest(request, &segments);

    EXPECT_EQ(1, segments.conversion_segments_size());
    EXPECT_EQ("\xE5\x8C\xB9",  // "匹"
              segments.conversion_segment(0).candidate(0).value);
  }
}

TEST_F(ConverterTest, Regression3046266) {
  // Shouldn't correct nodes at the beginning of a sentence.
  ConverterInterface *converter = ConverterFactory::GetConverter();
  Segments segments;

  // Can be any string that has "ん" at the end
  // "かん"
  const char kKey1[] = "\xE3\x81\x8B\xE3\x82\x93";

  // Can be any string that has a vowel at the beginning
  // "あか"
  const char kKey2[] = "\xE3\x81\x82\xE3\x81\x8B";

  // "中"
  const char kValueNotExpected[] = "\xE4\xB8\xAD";

  EXPECT_TRUE(converter->StartConversion(
      &segments, kKey1));
  EXPECT_EQ(1, segments.conversion_segments_size());
  EXPECT_TRUE(converter->CommitSegmentValue(
      &segments, 0, 0));
  EXPECT_TRUE(converter->FinishConversion(&segments));

  EXPECT_TRUE(converter->StartConversion(
      &segments, kKey2));
  EXPECT_EQ(1, segments.conversion_segments_size());
  const Segment &segment = segments.conversion_segment(0);
  for (size_t i = 0; i < segment.candidates_size(); ++i) {
    EXPECT_NE(segment.candidate(i).value, kValueNotExpected);
  }
}

TEST_F(ConverterTest, Regression5502496) {
  // Make sure key correction works for the first word of a sentence.
  ConverterInterface *converter = ConverterFactory::GetConverter();
  Segments segments;

  // "みんあ"
  const char kKey[] = "\xE3\x81\xBF\xE3\x82\x93\xE3\x81\x82";

  // "みんな"
  const char kValueExpected[] = "\xE3\x81\xBF\xE3\x82\x93\xE3\x81\xAA";

  EXPECT_TRUE(converter->StartConversion(&segments, kKey));
  EXPECT_EQ(1, segments.conversion_segments_size());
  const Segment &segment = segments.conversion_segment(0);
  bool found = false;
  for (size_t i = 0; i < segment.candidates_size(); ++i) {
    if (segment.candidate(i).value == kValueExpected) {
      found = true;
    }
  }
  EXPECT_TRUE(found);
}

TEST_F(ConverterTest, EmoticonsAboveSymbols) {
  ConverterInterface *converter = ConverterFactory::GetConverter();
  Segments segments;

  // "かおもじ"
  const char kKey[] = "\xE3\x81\x8B\xE3\x81\x8A\xE3\x82\x82\xE3\x81\x98";

  const char kEmoticon[] = "^^;";
  // "☹": A platform-dependent symbol
  const char kSymbol[] = "\xE2\x98\xB9";

  EXPECT_TRUE(converter->StartConversion(
      &segments, kKey));
  EXPECT_EQ(1, segments.conversion_segments_size());
  const Segment &segment = segments.conversion_segment(0);
  bool found_emoticon = false;
  bool found_symbol = false;

  for (size_t i = 0; i < segment.candidates_size(); ++i) {
    if (segment.candidate(i).value == kEmoticon) {
      found_emoticon = true;
    } else if (segment.candidate(i).value == kSymbol) {
      found_symbol = true;
    }
    if (found_symbol) {
      break;
    }
  }
  EXPECT_TRUE(found_emoticon);
}


TEST_F(ConverterTest, StartPartialPrediction) {
  ConverterInterface *converter = ConverterFactory::GetConverter();
  CHECK(converter);
  Segments segments;
  // "わたしは"
  EXPECT_TRUE(converter->StartPartialPrediction(
              &segments,
              "\xE3\x82\x8F\xE3\x81\x9F\xE3\x81\x97\xE3\x81\xAF"));
  EXPECT_EQ(1, segments.segments_size());
  // "わたしは"
  EXPECT_EQ("\xE3\x82\x8F\xE3\x81\x9F\xE3\x81\x97\xE3\x81\xAF",
            segments.segment(0).candidate(0).key);
  // "わたしは"
  EXPECT_EQ("\xE3\x82\x8F\xE3\x81\x9F\xE3\x81\x97\xE3\x81\xAF",
            segments.segment(0).candidate(0).content_key);
}

TEST_F(ConverterTest, StartPartialSuggestion) {
  ConverterInterface *converter = ConverterFactory::GetConverter();
  CHECK(converter);
  Segments segments;
  // "わたしは"
  EXPECT_TRUE(converter->StartPartialSuggestion(
              &segments,
              "\xE3\x82\x8F\xE3\x81\x9F\xE3\x81\x97\xE3\x81\xAF"));
  EXPECT_EQ(1, segments.segments_size());
  // "わたしは"
  EXPECT_EQ("\xE3\x82\x8F\xE3\x81\x9F\xE3\x81\x97\xE3\x81\xAF",
            segments.segment(0).candidate(0).key);
  // "わたしは"
  EXPECT_EQ("\xE3\x82\x8F\xE3\x81\x9F\xE3\x81\x97\xE3\x81\xAF",
            segments.segment(0).candidate(0).content_key);
}



TEST_F(ConverterTest, Predict_SetKey) {
  const char kPredictionKey[] = "prediction key";
  const char kPredictionKey2[] = "prediction key2";
  // Tests whether SetKey method is called or not.
  struct TestData {
    const Segments::RequestType request_type_;
    const char *key_;
    const bool expect_reset_;
  };
  const TestData test_data_list[] = {
      {Segments::CONVERSION, NULL, true},
      {Segments::CONVERSION, kPredictionKey, true},
      {Segments::CONVERSION, kPredictionKey2, true},
      {Segments::REVERSE_CONVERSION, NULL, true},
      {Segments::REVERSE_CONVERSION, kPredictionKey, true},
      {Segments::REVERSE_CONVERSION, kPredictionKey2, true},
      {Segments::PREDICTION, NULL, true},
      {Segments::PREDICTION, kPredictionKey2, true},
      {Segments::SUGGESTION, NULL, true},
      {Segments::SUGGESTION, kPredictionKey2, true},
      {Segments::PARTIAL_PREDICTION, NULL, true},
      {Segments::PARTIAL_PREDICTION, kPredictionKey2, true},
      {Segments::PARTIAL_SUGGESTION, NULL, true},
      {Segments::PARTIAL_SUGGESTION, kPredictionKey2, true},
      // If we are predicting, and one or more segment exists,
      // and the segments's key equals to the input key, then do not reset.
      {Segments::PREDICTION, kPredictionKey, false},
      {Segments::SUGGESTION, kPredictionKey, true},
      {Segments::PARTIAL_PREDICTION, kPredictionKey, false},
      {Segments::PARTIAL_SUGGESTION, kPredictionKey, true},
  };

  ConverterImpl *converter;
  converter = dynamic_cast<ConverterImpl*>(ConverterFactory::GetConverter());

  CHECK(converter);

  class StubPredictor : public PredictorInterface {
    bool Predict(Segments *segments) const { return true; };
  };
  StubPredictor predictor;

  class StubRewriter : public RewriterInterface {
    bool Rewrite(Segments *segments) const { return true; };
  };
  StubRewriter rewriter;

  PredictorFactory::SetPredictor(&predictor);
  RewriterFactory::SetRewriter(&rewriter);
  // Note that TearDown method will reset above stubs.

  for (size_t i = 0; i < ARRAYSIZE(test_data_list); ++i) {
    const TestData &test_data = test_data_list[i];
    Segments segments;
    segments.set_request_type(test_data.request_type_);

    if (test_data.key_) {
      mozc::Segment *seg = segments.add_segment();
      seg->Clear();
      seg->set_key(test_data.key_);
      // The segment has a candidate.
      seg->add_candidate();
    }
    ConversionRequest request;
    converter->Predict(request, kPredictionKey,
                       Segments::PREDICTION, &segments);

    EXPECT_EQ(1, segments.conversion_segments_size());
    EXPECT_EQ(kPredictionKey, segments.conversion_segment(0).key());
    EXPECT_EQ(test_data.expect_reset_ ? 0 : 1,
              segments.conversion_segment(0).candidates_size());
  }
}

TEST_F(ConverterTest, StartPredictionForRequest_KikiIppatsu) {
  ConverterInterface *converter = ConverterFactory::GetConverter();
  // To see preceding text helps prediction, consider the case where user
  // converts "いっぱつ".
  {
    // Without preceding text, test dictionary predicts "いっぱつ" to "一発".
    Segments segments;
    mozc::composer::Composer composer;
    composer.InsertCharacter(
        "\xE3\x81\x84\xE3\x81\xA3\xE3\x81\xB1\xE3\x81\xA4");  // "いっぱつ"
    mozc::ConversionRequest request(&composer);
    converter->StartPredictionForRequest(request, &segments);

    EXPECT_EQ(1, segments.conversion_segments_size());
    EXPECT_EQ("\xE4\xB8\x80\xE7\x99\xBA",  // "一発"
              segments.conversion_segment(0).candidate(0).value);
  }
  {
    // However, with preceding text "危機", test dictionary converts "いっぱつ"
    // to "一髪".
    Segments segments;
    mozc::composer::Composer composer;
    composer.InsertCharacter(
        "\xE3\x81\x84\xE3\x81\xA3\xE3\x81\xB1\xE3\x81\xA4");  // "いっぱつ"
    mozc::ConversionRequest request(&composer);
    request.set_preceding_text("\xE5\x8D\xB1\xE6\xA9\x9F");  // "危機"
    converter->StartPredictionForRequest(request, &segments);

    EXPECT_EQ(1, segments.conversion_segments_size());
    EXPECT_EQ("\xE4\xB8\x80\xE9\xAB\xAA",  // "一髪"
              segments.conversion_segment(0).candidate(0).value);
  }
}

TEST_F(ConverterTest, StartPredictionForRequest_Jyosushi) {
  ConverterInterface *converter = ConverterFactory::GetConverter();
  // To see preceding text helps prediction after number characters, consider
  // the case where user converts "ひき".
  {
    // Without preceding text, test dictionary predicts "ひき" as "引換".
    Segments segments;
    mozc::composer::Composer composer;
    composer.InsertCharacter("\xE3\x81\xB2\xE3\x81\x8D");  // "ひき"
    mozc::ConversionRequest request(&composer);
    converter->StartPredictionForRequest(request, &segments);

    EXPECT_EQ(1, segments.conversion_segments_size());
    EXPECT_EQ("\xE5\xBC\x95\xE6\x8F\x9B",  // "引換"
              segments.conversion_segment(0).candidate(0).value);
  }
  {
    // However, if providing "猫が5" as preceding text, "匹" is predicted from
    // "ひき" with test dictionary.
    Segments segments;
    mozc::composer::Composer composer;
    composer.InsertCharacter("\xE3\x81\xB2\xE3\x81\x8D");  // "ひき"
    mozc::ConversionRequest request(&composer);
    request.set_preceding_text("\xE7\x8C\xAB\xE3\x81\x8C\x35");  // "猫が5"
    converter->StartPredictionForRequest(request, &segments);

    EXPECT_EQ(1, segments.conversion_segments_size());
    EXPECT_EQ("\xE5\x8C\xB9",  // "匹"
              segments.conversion_segment(0).candidate(0).value);
  }
}

}  // namespace mozc
