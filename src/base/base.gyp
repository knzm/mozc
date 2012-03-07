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
    'relative_dir': 'base',
    'gen_out_dir': '<(SHARED_INTERMEDIATE_DIR)/<(relative_dir)',
  },
  'targets': [
    {
      'target_name': 'base',
      'type': 'static_library',
      'sources': [
        'clock_mock.cc',
        'cpu_stats.cc',
        'crash_report_handler.cc',
        'crash_report_util.cc',
        'iconv.cc',
        'process.cc',
        'process_mutex.cc',
        'run_level.cc',
        'scheduler.cc',
        'stopwatch.cc',
        'svm.cc',
        'timer.cc',
        'unnamed_event.cc',
        'update_checker.cc',
        'update_util.cc',
        'url.cc',
      ],
      'dependencies': [
        'base_core',
      ],
      'conditions': [
        ['OS=="mac"', {
          'sources': [
            'mac_process.mm',
            'mac_util.mm',
          ],
          'conditions': [
            ['branding=="GoogleJapaneseInput"', {
              'sources': [
                'crash_report_handler.mm',
               ],
            }],
          ],
        }],
        ['OS=="win"', {
          'sources': [
            'win_sandbox.cc',
            'win_util.cc',
          ],
          'conditions': [
            ['branding=="GoogleJapaneseInput"', {
              'dependencies': [
                '<(DEPTH)/third_party/breakpad/breakpad.gyp:breakpad',
              ],
            }],
          ],
        }],
      ],
    },
    {
      'target_name': 'base_core',
      'type': 'static_library',
      'sources': [
        '<(gen_out_dir)/character_set.h',
        '<(gen_out_dir)/version_def.h',
        'flags.cc',
        'hash.cc',
        'init.cc',
        'logging.cc',
        'mutex.cc',
        'singleton.cc',
        'text_converter.cc',
        'text_normalizer.cc',
        'util.cc',
        'version.cc',
      ],
      'dependencies': [
        'gen_character_set',
        'gen_version_def',
      ],
    },
    {
      'target_name': 'gen_character_set',
      'type': 'none',
      'actions': [
        {
          'action_name': 'gen_character_set',
          'variables': {
            'input_files': [
              '../data/unicode/CP932.TXT',
              '../data/unicode/JIS0201.TXT',
              '../data/unicode/JIS0208.TXT',
              '../data/unicode/JIS0212.TXT',
              '../data/unicode/jisx0213-2004-std.txt',
            ],
          },
          'inputs': [
            'gen_character_set.py',
            '<@(input_files)',
          ],
          'outputs': [
            '<(gen_out_dir)/character_set.h',
          ],
          'action': [
            'python', 'gen_character_set.py',
            '--cp932file=../data/unicode/CP932.TXT',
            '--jisx0201file=../data/unicode/JIS0201.TXT',
            '--jisx0208file=../data/unicode/JIS0208.TXT',
            '--jisx0212file=../data/unicode/JIS0212.TXT',
            '--jisx0213file=../data/unicode/jisx0213-2004-std.txt',
            '--output=<(gen_out_dir)/character_set.h'
          ],
        },
      ],
    },
    {
      'target_name': 'encryptor',
      'type': 'static_library',
      'sources': [
        'encryptor.cc',
        'password_manager.cc',
      ],
      'dependencies': [
        'base',
      ],
    },
    {
      'target_name': 'gen_version_def',
      'type': 'none',
      'actions': [
        {
          'action_name': 'gen_version_def',
          'inputs': [
            '../mozc_version.txt',
            '../build_tools/replace_version.py',
            'version_def_template.h',
          ],
          'outputs': [
            '<(gen_out_dir)/version_def.h',
          ],
          'action': [
            'python', '../build_tools/replace_version.py',
            '--version_file', '../mozc_version.txt',
            '--input', 'version_def_template.h',
            '--output', '<(gen_out_dir)/version_def.h',
            '--target_platform', '<(target_platform)',
          ],
        },
      ],
    },
    {
      'target_name': 'config_file_stream',
      'type': 'static_library',
      'sources': [
        '<(gen_out_dir)/config_file_stream_data.h',
        'config_file_stream.cc',
      ],
      'dependencies': [
        'gen_config_file_stream_data',
      ],
    },
    {
      'target_name': 'gen_config_file_stream_data',
      'type': 'none',
      'actions': [
        {
          'action_name': 'gen_config_file_stream_data',
          'variables': {
            'input_files': [
              '../data/keymap/atok.tsv',
              '../data/keymap/kotoeri.tsv',
              '../data/keymap/mobile.tsv',
              '../data/keymap/ms-ime.tsv',
              '../data/preedit/kana.tsv',
              '../data/preedit/hiragana-romanji.tsv',
              '../data/preedit/romanji-hiragana.tsv',
              '../data/preedit/12keys-hiragana.tsv',
              '../data/preedit/12keys-halfwidthascii.tsv',
              '../data/preedit/12keys-number.tsv',
              '../data/preedit/flick-halfwidthascii.tsv',
              '../data/preedit/flick-hiragana.tsv',
              '../data/preedit/flick-number.tsv',
              '../data/preedit/toggle_flick-hiragana.tsv',
              '../data/preedit/toggle_flick-halfwidthascii.tsv',
              '../data/preedit/toggle_flick-number.tsv',
              '../data/preedit/qwerty_mobile-hiragana.tsv',
              '../data/preedit/qwerty_mobile-hiragana-number.tsv',
              '../data/preedit/qwerty_mobile-halfwidthascii.tsv',
            ],
          },
          'inputs': [
            '<@(input_files)',
          ],
          'outputs': [
            '<(gen_out_dir)/config_file_stream_data.h',
          ],
          'action': [
            'python', '../build_tools/redirect.py',
            '<(gen_out_dir)/config_file_stream_data.h',
            '<(mozc_build_tools_dir)/gen_config_file_stream_data_main',
            '<@(input_files)'
          ],
        },
      ],
    },
    {
      'target_name': 'gen_config_file_stream_data_main',
      'type': 'executable',
      'sources': [
        'gen_config_file_stream_data_main.cc',
      ],
      'dependencies': [
        'base',
      ],
    },
    {
      'target_name': 'install_gen_config_file_stream_data_main',
      'type': 'none',
      'variables': {
        'bin_name': 'gen_config_file_stream_data_main'
      },
      'includes' : [
        '../gyp/install_build_tool.gypi',
      ],
    },
  ],
  'conditions': [
    ['OS=="mac"', {
      'targets': [
        {
          'target_name': 'mac_util_main',
          'type': 'executable',
          'sources': [
            'mac_util_main.cc',
          ],
          'dependencies': [
            'base',
          ],
        },
      ]},
    ]
  ],
}
