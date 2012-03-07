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
    # We accept following warnings come from protobuf.
    # This list should be revised when protobuf is updated.
    'msvc_disabled_warnings_for_protoc': [
      # unary minus operator applied to unsigned type, result still
      # unsigned.
      # http://msdn.microsoft.com/en-us/library/4kh09110.aspx
      '4146',
      # 'this' : used in base member initializer list
      # http://msdn.microsoft.com/en-us/library/3c594ae3.aspx
      '4355',
      # 'function': was declared deprecated.
      # http://msdn.microsoft.com/en-us/library/ttcz0bys.aspx
      '4996',
    ],
    'protobuf_sources': [
      'files/src/google/protobuf/compiler/importer.cc',
      'files/src/google/protobuf/compiler/parser.cc',
      'files/src/google/protobuf/descriptor.cc',
      'files/src/google/protobuf/descriptor.pb.cc',
      'files/src/google/protobuf/descriptor_database.cc',
      'files/src/google/protobuf/dynamic_message.cc',
      'files/src/google/protobuf/extension_set.cc',
      'files/src/google/protobuf/extension_set_heavy.cc',
      'files/src/google/protobuf/generated_message_reflection.cc',
      'files/src/google/protobuf/generated_message_util.cc',
      'files/src/google/protobuf/io/coded_stream.cc',
      'files/src/google/protobuf/io/gzip_stream.cc',
      'files/src/google/protobuf/io/printer.cc',
      'files/src/google/protobuf/io/tokenizer.cc',
      'files/src/google/protobuf/io/zero_copy_stream.cc',
      'files/src/google/protobuf/io/zero_copy_stream_impl.cc',
      'files/src/google/protobuf/io/zero_copy_stream_impl_lite.cc',
      'files/src/google/protobuf/message.cc',
      'files/src/google/protobuf/message_lite.cc',
      'files/src/google/protobuf/reflection_ops.cc',
      'files/src/google/protobuf/repeated_field.cc',
      'files/src/google/protobuf/service.cc',
      'files/src/google/protobuf/stubs/common.cc',
      'files/src/google/protobuf/stubs/once.cc',
      'files/src/google/protobuf/stubs/structurally_valid.cc',
      'files/src/google/protobuf/stubs/strutil.cc',
      'files/src/google/protobuf/stubs/substitute.cc',
      'files/src/google/protobuf/text_format.cc',
      'files/src/google/protobuf/unknown_field_set.cc',
      'files/src/google/protobuf/wire_format.cc',
      'files/src/google/protobuf/wire_format_lite.cc',
    ],
    'protoc_sources': [
      'files/src/google/protobuf/compiler/code_generator.cc',
      'files/src/google/protobuf/compiler/command_line_interface.cc',
      'files/src/google/protobuf/compiler/cpp/cpp_enum.cc',
      'files/src/google/protobuf/compiler/cpp/cpp_enum_field.cc',
      'files/src/google/protobuf/compiler/cpp/cpp_extension.cc',
      'files/src/google/protobuf/compiler/cpp/cpp_field.cc',
      'files/src/google/protobuf/compiler/cpp/cpp_file.cc',
      'files/src/google/protobuf/compiler/cpp/cpp_generator.cc',
      'files/src/google/protobuf/compiler/cpp/cpp_helpers.cc',
      'files/src/google/protobuf/compiler/cpp/cpp_message.cc',
      'files/src/google/protobuf/compiler/cpp/cpp_message_field.cc',
      'files/src/google/protobuf/compiler/cpp/cpp_primitive_field.cc',
      'files/src/google/protobuf/compiler/cpp/cpp_service.cc',
      'files/src/google/protobuf/compiler/cpp/cpp_string_field.cc',
      'files/src/google/protobuf/compiler/java/java_enum.cc',
      'files/src/google/protobuf/compiler/java/java_enum_field.cc',
      'files/src/google/protobuf/compiler/java/java_extension.cc',
      'files/src/google/protobuf/compiler/java/java_field.cc',
      'files/src/google/protobuf/compiler/java/java_file.cc',
      'files/src/google/protobuf/compiler/java/java_generator.cc',
      'files/src/google/protobuf/compiler/java/java_helpers.cc',
      'files/src/google/protobuf/compiler/java/java_message.cc',
      'files/src/google/protobuf/compiler/java/java_message_field.cc',
      'files/src/google/protobuf/compiler/java/java_primitive_field.cc',
      'files/src/google/protobuf/compiler/java/java_service.cc',
      'files/src/google/protobuf/compiler/java/java_string_field.cc',
      'files/src/google/protobuf/compiler/main.cc',
      'files/src/google/protobuf/compiler/plugin.cc',
      'files/src/google/protobuf/compiler/plugin.pb.cc',
      'files/src/google/protobuf/compiler/python/python_generator.cc',
      'files/src/google/protobuf/compiler/subprocess.cc',
      'files/src/google/protobuf/compiler/zip_writer.cc',
    ],
  },
  'targets': [
    {
      'target_name': 'protobuf',
      'type': 'static_library',
      'conditions': [
        ['OS=="linux"', {
          'conditions': [
            ['use_libprotobuf==1', {
              'link_settings': {
                'libraries': [
                  '-lprotobuf',
                ],
              },
            }, {  # OS=="linux" and use_libprotobuf==0
              'sources': ['<@(protobuf_sources)'],
              'include_dirs': ['.'],
            }],
          ],
        }],
        ['OS=="mac"', {
          'sources': ['<@(protobuf_sources)'],
          'include_dirs': ['.'],
          'xcode_settings': {
            'WARNING_CFLAGS': ['-Wno-error'],
          },
        }],
        ['OS=="win"', {
          'sources': ['<@(protobuf_sources)'],
          'include_dirs': ['.'],
          'msvs_settings': {
            'VCCLCompilerTool': {
              'DisableSpecificWarnings': [
                '<@(msvc_disabled_warnings_for_protoc)',
              ],
            },
          },
        }],
      ],
    },
    {
      'target_name': 'protoc',
      'type': 'executable',
      'dependencies': [
        'protobuf',
      ],
      'include_dirs': [
        '.',
      ],
      'conditions': [
        ['OS=="linux"', {
          'conditions': [
            ['use_libprotobuf!=1', {
              'cflags': [
                '-Wno-unused-result',  # protoc has unused result.
              ],
              'sources': ['<@(protoc_sources)'],
            }],
          ],
        }],
        ['OS=="mac"', {
          'sources': ['<@(protoc_sources)'],
          'xcode_settings': {
            'WARNING_CFLAGS': ['-Wno-error'],
          },
        }],
        ['OS=="win"', {
          'sources': ['<@(protoc_sources)'],
          'msvs_settings': {
            'VCCLCompilerTool': {
              'DisableSpecificWarnings': [
                '<@(msvc_disabled_warnings_for_protoc)'
              ],
            },
          },
        }],
      ],
    },
    {
      'target_name': 'install_protoc',
      'type': 'none',
      'variables': {
        'bin_name': 'protoc',
      },
      'conditions': [
        # use system-installed protoc on Linux
        ['OS!="linux"', {
          'includes' : [
            '../gyp/install_build_tool.gypi',
          ],
        }, {  # OS=="linux"
          'conditions': [
            ['use_libprotobuf==0', {
              'includes' : [
                '../gyp/install_build_tool.gypi',
              ],
            }],
          ],
        }],
      ],
    },
  ],
}
