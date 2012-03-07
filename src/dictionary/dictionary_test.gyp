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
    'relative_dir': 'dictionary',
    'gen_out_dir': '<(SHARED_INTERMEDIATE_DIR)/<(relative_dir)',
  },
  'targets': [
    {
      'target_name': 'dictionary_test',
      'type': 'executable',
      'sources': [
        'dictionary_mock_test.cc',
        'dictionary_test.cc',
        'suppression_dictionary_test.cc',
        'user_dictionary_importer_test.cc',
        'user_dictionary_storage_test.cc',
        'user_dictionary_test.cc',
        'user_dictionary_util_test.cc',
        'user_pos_test.cc',
      ],
      'dependencies': [
        '../base/base.gyp:base',
        '../config/config.gyp:config_handler',
        '../testing/testing.gyp:gtest_main',
        '../usage_stats/usage_stats.gyp:usage_stats',
        '../usage_stats/usage_stats.gyp:usage_stats_protocol',
        'dictionary.gyp:dictionary',
        'dictionary.gyp:dictionary_mock',
        'dictionary.gyp:suppression_dictionary',
        'dictionary.gyp:user_dictionary',
      ],
      'variables': {
        'test_size': 'small',
      },
    },
    {
      'target_name': 'text_dictionary_loader_test',
      'type': 'executable',
      'sources': [
        'text_dictionary_loader_test.cc',
      ],
      'dependencies': [
        '../base/base.gyp:base',
        '../testing/testing.gyp:gtest_main',
        'dictionary_base.gyp:text_dictionary_loader',
      ],
      'variables': {
        'test_size': 'small',
      },
    },
    # Test cases meta target: this target is referred from gyp/tests.gyp
    {
      'target_name': 'dictionary_all_test',
      'type': 'none',
      'dependencies': [
        'dictionary_test',
        'text_dictionary_loader_test',
      ],
    },
  ],
}
