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
    'relative_dir': 'prediction',
    'gen_out_dir': '<(SHARED_INTERMEDIATE_DIR)/<(relative_dir)',
  },
  'targets': [
    {
      'target_name': 'prediction',
      'type': 'static_library',
      'sources': [
        '<(proto_out_dir)/<(relative_dir)/user_history_predictor.pb.cc',
        'suggestion_filter.cc',
        'dictionary_predictor.cc',
        'predictor.cc',
        'user_history_predictor.cc',
      ],
      'dependencies': [
        '../base/base.gyp:base',
        '../base/base.gyp:config_file_stream',
        '../config/config.gyp:config_handler',
        '../converter/converter_base.gyp:immutable_converter',
        '../converter/converter_base.gyp:segments',
        '../dictionary/dictionary.gyp:dictionary',
        '../dictionary/dictionary.gyp:suffix_dictionary',
        '../dictionary/dictionary.gyp:suppression_dictionary',
        '../rewriter/rewriter.gyp:rewriter',
        '../session/session_base.gyp:genproto_session',
        '../usage_stats/usage_stats.gyp:usage_stats',
        '../usage_stats/usage_stats.gyp:genproto_usage_stats',
        'gen_suggestion_filter_data',
        'genproto_prediction',
      ],
      'conditions': [['two_pass_build==0', {
        'dependencies': [
          'install_gen_suggestion_filter_main',
        ],
      }]],
    },
    {
      'target_name': 'gen_suggestion_filter_data',
      'type': 'none',
      'actions': [
        {
          'action_name': 'gen_suggestion_filter_data',
          'variables': {
            'input_files%': [
              '../data/dictionary/suggestion_filter.txt',
            ],
          },
          'inputs': [
            '<@(input_files)',
          ],
          'conditions': [['two_pass_build==0', {
            'inputs': [
              '<(mozc_build_tools_dir)/gen_suggestion_filter_main',
            ],
          }]],
          'outputs': [
            '<(gen_out_dir)/suggestion_filter_data.h',
          ],
          'action': [
            '<(mozc_build_tools_dir)/gen_suggestion_filter_main',
            '<@(input_files)',
            '<(gen_out_dir)/suggestion_filter_data.h',
            '--header',
            '--logtostderr',
          ],
        },
      ],
    },
    {
      'target_name': 'gen_suggestion_filter_main',
      'type': 'executable',
      'sources': [
        'gen_suggestion_filter_main.cc',
      ],
      'dependencies': [
        '../storage/storage.gyp:storage',
      ],
    },
    {
      'target_name': 'install_gen_suggestion_filter_main',
      'type': 'none',
      'variables': {
        'bin_name': 'gen_suggestion_filter_main'
      },
      'includes' : [
        '../gyp/install_build_tool.gypi',
      ]
    },
    {
      'target_name': 'genproto_prediction',
      'type': 'none',
      'sources': [
        'user_history_predictor.proto',
      ],
      'includes': [
        '../protobuf/genproto.gypi',
      ],
    },
    {
      'target_name': 'prediction_test',
      'type': 'executable',
      'sources': [
        'dictionary_predictor_test.cc',
        'suggestion_filter_test.cc',
        'user_history_predictor_test.cc',
        'predictor_test.cc',
      ],
      'dependencies': [
        '../dictionary/dictionary.gyp:dictionary_mock',
        '../dictionary/dictionary_base.gyp:install_dictionary_test_data',
        '../config/config.gyp:config_handler',
        '../testing/testing.gyp:gtest_main',
        'prediction',
      ],
      'variables': {
        'test_size': 'small',
      },
    },
    # Test cases meta target: this target is referred from gyp/tests.gyp
    {
      'target_name': 'prediction_all_test',
      'type': 'none',
      'dependencies': [
        'prediction_test',
      ],
    },
  ],
}
