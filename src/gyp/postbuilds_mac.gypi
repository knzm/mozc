# Copyright 2010, Google Inc.
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
  'conditions': [
    ['branding=="GoogleJapaneseInput"', {
      'variables': {
        'mac_breakpad_dir': '<(DEPTH)/../mac/Releases/GoogleBreakpad/',
        'mac_breakpad_framework': '<(mac_breakpad_dir)/GoogleBreakpad.framework',
      },
      'copies': [
        {
          'files': [
            '<(mac_breakpad_framework)',
          ],
          'destination': '<(PRODUCT_DIR)/<(product_name).app/Contents/Frameworks',
        },
      ],
      'link_settings': {
        'libraries': [
          '<(mac_breakpad_framework)',
        ],
      },
      'postbuilds': [
        {
          'postbuild_name': 'dump symbols',
          'action': [
            'python', '../build_tools/redirect.py',
            '${BUILT_PRODUCTS_DIR}/<(product_name)_i386.breakpad',
            '<(mac_breakpad_dir)/dump_syms',
            '-a', 'i386',
            '${BUILT_PRODUCTS_DIR}/<(product_name).app/Contents/MacOS/<(product_name)',
          ],
        },
        {
          'postbuild_name': 'strip binary',
          'action': [
            '/usr/bin/strip',
            '${BUILT_PRODUCTS_DIR}/<(product_name).app/Contents/MacOS/<(product_name)'
          ],
        },
        {
          'postbuild_name': 'code signing',
          'action': [
            'python', '../build_tools/run_after_chdir.py',
            '../',
            './build_tools/codesign_mac.py',
            '--target', '${BUILT_PRODUCTS_DIR}/<(product_name).app',
            '--noautoconf',
          ]
        },
      ],
    }, {  # branding != GoogleJapaneseInput
      'postbuilds': [
        {
          'postbuild_name': 'strip binary',
          'action': [
            '/usr/bin/strip',
            '${BUILT_PRODUCTS_DIR}/<(product_name).app/Contents/MacOS/<(product_name)'
          ],
        },
      ],
    }],
  ],
}
