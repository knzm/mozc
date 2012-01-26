# Copyright 2010-2012, Google Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

{
  'variables': {
    'relative_dir': 'rewriter',
    'gen_out_dir': '<(SHARED_INTERMEDIATE_DIR)/<(relative_dir)',
  },
  'targets': [
    {
      # TODO(team): split this test into individual tests.
      'target_name': 'rewriter_test',
      'type': 'executable',
      'sources': [
        'calculator_rewriter_test.cc',
        'collocation_util_test.cc',
        'date_rewriter_test.cc',
        'dice_rewriter_test.cc',
        'command_rewriter_test.cc',
        'dictionary_generator_test.cc',
        'emoticon_rewriter_test.cc',
        'english_variants_rewriter_test.cc',
        'focus_candidate_rewriter_test.cc',
        'fortune_rewriter_test.cc',
        'merger_rewriter_test.cc',
        'number_rewriter_test.cc',
        'normalization_rewriter_test.cc',
        'remove_redundant_candidate_rewriter_test.cc',
        # 'rewriter_test.cc',
        'symbol_rewriter_test.cc',
        'transliteration_rewriter_test.cc',
        'unicode_rewriter_test.cc',
        'user_boundary_history_rewriter_test.cc',
        'user_segment_history_rewriter_test.cc',
        'variants_rewriter_test.cc',
        'version_rewriter_test.cc'
      ],
      'dependencies': [
        '../base/base.gyp:base',
        '../converter/converter.gyp:converter',
        '../testing/testing.gyp:gtest_main',
        'calculator/calculator.gyp:calculator_mock',
        'rewriter.gyp:rewriter',
      ],
      'variables': {
        'test_size': 'small',
      },
      # TODO(horo): usage is available only in Mac and Win now.
      'conditions': [
        ['OS=="mac" or OS=="win"', {
          'sources': [
            'usage_rewriter_test.cc',
          ],
        }],
      ],
    },
    {
      'target_name': 'single_kanji_rewriter_test',
      'type': 'executable',
      'sources': [
        'single_kanji_rewriter_test.cc',
      ],
      'dependencies': [
        '../base/base.gyp:base',
        '../testing/testing.gyp:gtest_main',
        'rewriter.gyp:rewriter',
      ],
      'variables': {
        'test_size': 'small',
      },
    },
    # Test cases meta target: this target is referred from gyp/tests.gyp
    {
      'target_name': 'rewriter_all_test',
      'type': 'none',
      'dependencies': [
        'calculator/calculator.gyp:calculator_all_test',
        'rewriter_test',
        'single_kanji_rewriter_test',
      ],
    },
  ],
}
