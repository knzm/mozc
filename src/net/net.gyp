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
  'targets': [
    {
      'target_name': 'net',
      'type': 'static_library',
      'sources': [
        'http_client.cc',
        'proxy_manager.cc',
      ],
      'dependencies': [
        '../base/base.gyp:base',
      ],
      'conditions': [
        ['OS=="mac"', {
          'sources': [
            'http_client_mac.mm',
          ],
        }],
        ['OS=="win"', {
          'link_settings': {
            'msvs_settings': {
              'VCLinkerTool': {
                'AdditionalDependencies': [
                  'wininet.lib',
                  # used in 'http_client.cc'
                  # TODO(yukawa): Remove this dependency by replacing
                  # ::timeGetTime() with Util::GetTick().
                  'winmm.lib',
                ],
              },
            },
          },
        }],
        ['OS=="linux" and target_platform!="Android"', {
          # Enable libcurl
          'cflags': [
            '<!@(<(pkg_config_command) --cflags-only-other libcurl)',
          ],
          'defines': [
            'HAVE_CURL=1',
          ],
          'include_dirs': [
            '<!@(<(pkg_config_command) --cflags-only-I libcurl)',
          ],
          # Following settings will be applied into all the executable targets
          # and shared_library targets which directly or indirectly depend on
          # this target.
          'link_settings': {
            'ldflags': [
              '<!@(<(pkg_config_command) --libs-only-L libcurl)',
            ],
            'libraries': [
              '<!@(<(pkg_config_command) --libs-only-l libcurl)',
            ],
          },
        }],
      ],
    },
    {
      'target_name': 'http_client_mock',
      'type': 'static_library',
      'sources': [
        'http_client_mock.cc',
      ],
      'dependencies': [
        '../base/base.gyp:base',
        'net',
      ],
    },
    {
      'target_name': 'http_client_main',
      'type': 'executable',
      'sources': [
        'http_client_main.cc',
      ],
      'dependencies': [
        '../base/base.gyp:base',
        'net',
      ],
    },
    {
      'target_name': 'http_client_mock_test',
      'type': 'executable',
      'sources': [
        'http_client_mock_test.cc',
      ],
      'dependencies': [
        '../testing/testing.gyp:gtest_main',
        'http_client_mock',
      ],
      'variables': {
        'test_size': 'small',
      },
    },
    {
      'target_name': 'jsonpath',
      'type': 'static_library',
      'sources': [
        'jsonpath.cc',
      ],
      'dependencies': [
        '../base/base.gyp:base',
        '<(DEPTH)/third_party/jsoncpp/jsoncpp.gyp:jsoncpp',
      ],
    },
    {
      'target_name': 'jsonpath_test',
      'type': 'executable',
      'sources': [
        'jsonpath_test.cc',
      ],
      'dependencies': [
        '../testing/testing.gyp:gtest_main',
        'jsonpath',
      ],
      'variables': {
        'test_size': 'small',
      },
    },
    # Test cases meta target: this target is referred from gyp/tests.gyp
    {
      'target_name': 'net_all_test',
      'type': 'none',
      'dependencies': [
        'http_client_mock_test',
      ],
      'conditions': [
        ['enable_extra_unit_tests==1', {
          'dependencies': [
            # Temporarily disable following test because JsonCpp still
            # cannot be built on OSS Mozc.
            # TODO(nona): Support building JsonCpp on OSS Mozc and remove
            #     the condtion above.
            'jsonpath_test',
          ],
        }],
      ],
    },
  ],
}
