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

# IMPORTANT:
# Please don't directly include this file since the gypi files is
# automatically included.
# You may find cool techniques in the following *.gypi file.
# http://src.chromium.org/viewvc/chrome/trunk/src/build/common.gypi
{
  'variables': {
    # Top directory of third party libraries.
    'third_party_dir%': '<(DEPTH)/third_party',

    # Top directory of third party libraries for Windows.
    'third_party_dir_win%': '<(DEPTH)/third_party',

    # Flags to temporarily disable some unit tests which are still
    # unavailable on OSS Mozc.
    'enable_extra_unit_tests': 0,

    # This variable need to be set to 1 when you build Mozc for Chromium OS.
    'chromeos%': 0,
    # Extra libraries for Linux. This can be used like:
    # GYP_DEFINES='extra_linux_libs="-lfoo -lbar"' python build_mozc.py gyp
    'extra_linux_libs%': [],

    # warning_cflags will be shared with Mac and Linux.
    'warning_cflags': [
      '-Wall',
      '-Werror',
      '-Wno-char-subscripts',
      '-Wno-sign-compare',
      '-Wno-deprecated-declarations',
      '-Wwrite-strings',
    ],
    # gcc_cflags will be shared with Mac and Linux.
    'gcc_cflags': [
      '-fmessage-length=0',
      '-fno-omit-frame-pointer',
      '-fno-strict-aliasing',
      '-funsigned-char',
      '-include', 'base/namespace.h',
      '-pipe',
      '-pthread',
    ],
    # Libraries for GNU/Linux environment.
    'linux_ldflags': [
      '-pthread',
    ],
    'linux_libs': [
      '-lcrypto',
      '-lssl',
      '-lz',
    ],

    # 'conditions' is put inside of 'variables' so that we can use
    # another 'conditions' in this gyp element level later. Note that
    # you can have only one 'conditions' in a gyp element.
    'variables': {
      'target_compiler': '',
      'conditions': [
        ['OS=="win"', {
          # Variable 'MSVS_VERSION' is available on Windows only.
          'conditions': [
            ['MSVS_VERSION=="2008"', {
              'target_compiler': 'msvs2008',
            }],
            ['MSVS_VERSION=="2010"', {
              'target_compiler': 'msvs2010',
            }],
          ],
        }],
      ],
    },

    # The target compiler such as 'msvs2008' or 'msvs2010'. This value is
    # currently used only on Windows.
    'target_compiler': '<(target_compiler)',

    # Extra defines
    'additional_defines%': [],

    # Extra headers and libraries for Visual C++.
    'msvs_includes%': [],
    'msvs_libs_x86%': [],
    'msvs_libs_x64%': [],

    # enable_unittest represents if gtest-based unittest is available or not.
    # This flag is valid on all the platforms except for NaCl.
    # TODO(yukawa): Support NaCl environment.
    'enable_unittest%': '1',

    # On internal Visual C++ 2008 build, we use checked-in version of Visual C++
    # libraries and Platform SDK 7.1 libraries.
    'conditions': [
      ['target_platform=="ChromeOS"', {
        # Unittest is not integrated to the automated test framework yet.
        # TODO(nona): Enable unittest on Chromium OS. crosbug.com/19325
        'enable_unittest': '0',
      }],
    ],
    'msvc_disabled_warnings': [
      # 'expression' : signed/unsigned mismatch
      # http://msdn.microsoft.com/en-us/library/y92ktdf2.aspx
      '4018',
      # 'argument' : conversion from 'type1' to 'type2', possible loss
      # of data
      # http://msdn.microsoft.com/en-us/library/2d7604yb.aspx
      '4244',
      # 'var' : conversion from 'size_t' to 'type', possible loss of
      # data
      # http://msdn.microsoft.com/en-us/library/6kck0s93.aspx
      '4267',
      # 'identifier' : truncation from 'type1' to 'type2'
      # http://msdn.microsoft.com/en-us/library/0as1ke3f.aspx
      '4305',
      # 'type' : forcing value to bool 'true' or 'false'
      # (performance warning)
      # http://msdn.microsoft.com/en-us/library/b6801kcy.aspx
      '4800',
      # The file contains a character that cannot be represented in the
      # current code page (number). Save the file in Unicode format to
      # prevent data loss.
      # http://msdn.microsoft.com/en-us/library/ms173715.aspx
      '4819',
    ],

    # We wanted to have this directory under the build output directory
    # (ex. 'out' for Linux), but there is no variable defined for the top
    # level source directory, hence we create the directory in the top
    # level source directory.
    'mozc_build_tools_dir': '<(DEPTH)/mozc_build_tools/<(OS)',
    'proto_out_dir': '<(SHARED_INTERMEDIATE_DIR)/proto_out',
    'branding%': 'Mozc',
    # use_qt is 'YES' only if you want to use GUI binaries.
    'use_qt%': 'YES',

    # use_libprotobuf represents if protobuf library is used or not.
    # This option is only for Linux.
    'use_libprotobuf%': 0,

    # use_libzinnia represents if zinnia library is used or not.
    # This option is only for Linux.
    'use_libzinnia%': 1,

    # use_libxml represents if libxml library is used or not.
    # This option is only for Linux.
    'use_libxml%': 1,

    # use_libibus represents if ibus library is used or not.
    # This option is only for Linux.
    'use_libibus%': 0,

    # use_libscim represents if scim library is used or not.
    # This option is only for Linux.
    'use_libscim%': 0,

    # a flag whether the current build is dev-channel or not.
    'channel_dev%': '0',

    # enable_cloud_sync represents if cloud sync feature is enabled or not.
    'enable_cloud_sync%': 0,

    # enable_cloud_handwriting represents if cloud handwriting feature is
    # enabled or not.
    'enable_cloud_handwriting%': 0,

    # enable_webservice_infolist represents if webservice infolist feature is
    # enabled or not.
    'enable_webservice_infolist%': 0,

    # enable_gtk_renderer represents if gtk native candidate window feature is
    # enabled or not.
    'enable_gtk_renderer%': 0,


    # The pkg-config command to get the cflags/ldflags for Linux
    # builds.  We make it customizable to allow building in a special
    # environment such like cross-platform build.
    'pkg_config_command%': 'pkg-config',

    # Android SDK home.
    # Used only on Android build.
    'android_home_dir%': '',

    'mozc_data_dir': '<(SHARED_INTERMEDIATE_DIR)/',
  },
  'target_defaults': {
    'variables': {
      # See http://gcc.gnu.org/onlinedocs/gcc-4.4.2/gcc/Optimize-Options.html
      'mac_release_optimization%': '2',  # Use -O2 unless overridden
      'mac_debug_optimization%': '0',    # Use -O0 unless overridden
      # See http://msdn.microsoft.com/en-us/library/aa652360(VS.71).aspx
      'win_optimization_debug%': '0',    # 0 = /Od
      'win_optimization_release%': '2',  # 2 = /Og /Oi /Ot /Oy /Ob2 /Gs /GF /Gy
      'win_optimization_custom%': '4',   # 4 = None but prevents vcbuild from
                                         # inheriting default optimization.
      # See http://msdn.microsoft.com/en-us/library/aa652367(VS.71).aspx
      'win_release_static_crt%': '0',   # 0 = /MT (nondebug static)
      'win_debug_static_crt%': '1',     # 1 = /MTd (debug static)
      'win_release_dynamic_crt%': '2',  # 2 = /MD (nondebug dynamic)
      'win_debug_dynamic_crt%': '3',    # 3 = /MDd (debug dynamic)
      # See http://msdn.microsoft.com/en-us/library/aa652352(VS.71).aspx
      'win_target_machine_x86%': '1',
      'win_target_machine_x64%': '17',
      # See http://msdn.microsoft.com/en-us/library/aa652256(VS.71).aspx
      'win_char_set_not_set%': '0',
      'win_char_set_unicode%': '1',
      'win_char_set_mbcs%': '2',
      # Extra cflags for gcc
      'release_extra_cflags%': ['-O2'],
      'debug_extra_cflags%': ['-O0', '-g'],
    },
    'configurations': {
      'Common_Base': {
        'abstract': 1,
        'msvs_configuration_attributes': {
          'CharacterSet': '<(win_char_set_unicode)',
        },
        'conditions': [
          ['branding=="GoogleJapaneseInput"', {
            'defines': ['GOOGLE_JAPANESE_INPUT_BUILD'],
          }, {
            'defines': ['MOZC_BUILD'],
          }],
          ['channel_dev==1', {
            'defines': ['CHANNEL_DEV'],
          }],
          ['not(OS=="linux" and use_libprotobuf!=0)', {
            'include_dirs': [
              '../protobuf/files/src',
            ],
          }],
          # In order to extend language support of Mozc on Linux, we use
          # additional suffix except for Japanese so that multiple
          # converter processes can coexist. Note that Mozc on ChromeOS does
          # not use IPC so this kind of special treatment is not required.
          ['language!="japanese" and target_platform=="Linux"', {
            'defines': [
              'MOZC_LANGUAGE_SUFFIX_FOR_LINUX="_<(language)"',
            ],
          }],
          ['OS=="linux"', {
            'ldflags': [
              '<@(linux_ldflags)',
            ],
          }],
          ['use_separate_connection_data==1', {
            'defines': ['MOZC_USE_SEPARATE_CONNECTION_DATA'],
          }],
          ['use_separate_dictionary==1', {
            'defines': ['MOZC_USE_SEPARATE_DICTIONARY'],
          }],
          ['enable_cloud_sync==1', {
            'defines': ['ENABLE_CLOUD_SYNC'],
          }],
          ['enable_cloud_handwriting==1', {
            'defines': ['ENABLE_CLOUD_HANDWRITING'],
          }],
          ['enable_webservice_infolist==1', {
            'defines': ['ENABLE_WEBSERVICE_INFOLIST'],
          }],
          ['enable_gtk_renderer==1', {
            'defines': ['ENABLE_GTK_RENDERER'],
          }],
          ['enable_unittest==1', {
            'defines': ['MOZC_ENABLE_UNITTEST'],
          }],
        ],
      },
      'x86_Base': {
        'abstract': 1,
        'msvs_settings': {
          'VCLibrarianTool': {
            'AdditionalLibraryDirectories': [
              '<@(msvs_libs_x86)',
            ],
            'AdditionalLibraryDirectories!': [
              '<@(msvs_libs_x64)',
            ],
          },
          'VCLinkerTool': {
            'TargetMachine': '<(win_target_machine_x86)',
            'AdditionalOptions': [
              '/SAFESEH',
            ],
            'AdditionalLibraryDirectories': [
              '<@(msvs_libs_x86)',
            ],
            'AdditionalLibraryDirectories!': [
              '<@(msvs_libs_x64)',
            ],
            'EnableUAC': 'true',
            'UACExecutionLevel': '0',  # level="asInvoker"
            'UACUIAccess': 'false',    # uiAccess="false"
          },
        },
        'msvs_configuration_attributes': {
          'OutputDirectory': '<(build_base)/$(ConfigurationName)',
          'IntermediateDirectory': '<(build_base)/$(ConfigurationName)/obj/$(ProjectName)',
        },
        'msvs_configuration_platform': 'Win32',
      },
      'x64_Base': {
        'abstract': 1,
        'msvs_configuration_attributes': {
          'OutputDirectory': '<(build_base)/$(ConfigurationName)64',
          'IntermediateDirectory': '<(build_base)/$(ConfigurationName)64/obj/$(ProjectName)',
        },
        'msvs_configuration_platform': 'x64',
        'msvs_settings': {
          'VCLibrarianTool': {
            'AdditionalLibraryDirectories': [
              '<@(msvs_libs_x64)',
            ],
            'AdditionalLibraryDirectories!': [
              '<@(msvs_libs_x86)',
            ],
          },
          'VCLinkerTool': {
            'TargetMachine': '<(win_target_machine_x64)',
            'AdditionalLibraryDirectories': [
              '<@(msvs_libs_x64)',
            ],
            'AdditionalLibraryDirectories!': [
              '<@(msvs_libs_x86)',
            ],
          },
        },
      },
      'Debug_Base': {
        'abstract': 1,
        'defines': [
          'DEBUG',
        ],
        'xcode_settings': {
          'COPY_PHASE_STRIP': 'NO',
          'GCC_OPTIMIZATION_LEVEL': '<(mac_debug_optimization)',
          'GCC_INLINES_ARE_PRIVATE_EXTERN': 'YES',
          'OTHER_CFLAGS': [ '<@(debug_extra_cflags)', ],
        },
        'msvs_settings': {
          'VCCLCompilerTool': {
            'Optimization': '<(win_optimization_debug)',
            'PreprocessorDefinitions': ['_DEBUG'],
            'BasicRuntimeChecks': '3',
            'conditions': [
              ['use_dynamically_linked_qt=="YES"', {
                # As a quick workaround, use dynamically-linked version of CRT
                # if 'use_dynamically_linked_qt' is specified.
                # As for GoogleJapaneseInput branding build, this is not enough
                # because we cannot completely depend on dynamic CRT as
                # described in b/2506385. We should use static CRT for the
                # following binaries.
                # - GoogleIMEJaCacheService.exe
                # - GoogleIMEJaInstallerHelper32.dll
                # - GoogleIMEJaInstallerHelper64.dll
                # - any artifacts build in 'build_mozc.py build_tools'
                # TODO(yukawa): Support GoogleJapaneseInput branding build.
                'RuntimeLibrary': '<(win_debug_dynamic_crt)',
              }, {  # else
                'RuntimeLibrary': '<(win_debug_static_crt)',
              }],
            ],
          },
          'VCResourceCompilerTool': {
            'PreprocessorDefinitions': ['_DEBUG'],
          },
        },
        'conditions': [
          ['OS=="linux"', {
            'cflags': [
              '<@(debug_extra_cflags)',
            ],
          }],
        ],
      },
      'Optimize_Base': {
        'abstract': 1,
        'defines': [
          'NDEBUG',
          'QT_NO_DEBUG',
        ],
        'xcode_settings': {
          'DEAD_CODE_STRIPPING': 'YES',  # -Wl,-dead_strip
          'GCC_OPTIMIZATION_LEVEL': '<(mac_release_optimization)',
          'OTHER_CFLAGS': [ '<@(release_extra_cflags)', ],
        },
        'msvs_settings': {
          'VCCLCompilerTool': {
            # '<(win_optimization_release)' (that is /O2) contains /Oy option,
            # which makes debugging extremely difficult. (See b/1852473)
            # http://msdn.microsoft.com/en-us/library/8f8h5cxt.aspx
            # We can still disable FPO by using /Oy- option but the document
            # says there is an order dependency, that is, the last /Oy  or /Oy-
            # is valid.  See the following document for details.
            # http://msdn.microsoft.com/en-us/library/2kxx5t2c.aspx
            # As far as we observed, /Oy- adding in 'AdditionalOptions' always
            # appears at the end of options so using
            # '<(win_optimization_release) here is considered to be harmless.
            # Another reason to use /O2 here is b/2822535, where we observed
            # warning C4748 when we build mozc_tool with Qt libraries, which
            # are built with /O2.  We use the same optimization option between
            # Mozc and Qt just in case warning C4748 is true.
            'Optimization': '<(win_optimization_release)',
            'conditions': [
              ['use_dynamically_linked_qt=="YES"', {
                # As a quick workaround, use dynamically-linked version of CRT
                # if 'use_dynamically_linked_qt' is specified.
                # As for GoogleJapaneseInput branding build, this is not enough
                # because we cannot completely depend on dynamic CRT as
                # described in b/2506385. We should use static CRT for the
                # following binaries.
                # - GoogleIMEJaCacheService.exe
                # - GoogleIMEJaInstallerHelper32.dll
                # - GoogleIMEJaInstallerHelper64.dll
                # - any artifacts build in 'build_mozc.py build_tools'
                # TODO(yukawa): Support GoogleJapaneseInput branding build.
                'RuntimeLibrary': '<(win_release_dynamic_crt)',
              }, {  # else
                'RuntimeLibrary': '<(win_release_static_crt)',
              }],
            ],
          },
        },
        'conditions': [
          ['OS=="linux"', {
            'cflags': [
              '<@(release_extra_cflags)',
            ],
          }],
        ],
      },
      'Release_Base': {
        'abstract': 1,
        'defines': [
          'NO_LOGGING',
          'IGNORE_HELP_FLAG',
          'IGNORE_INVALID_FLAG'
        ],
        'conditions': [
          ['target_compiler=="msvs2008"', {
            'msvs_configuration_attributes': {
              'WholeProgramOptimization': '1',
            },
          }],
          ['target_compiler=="msvs2010"', {
            'msvs_settings': {
              'VCCLCompilerTool': {
                'WholeProgramOptimization': 'true',
              },
              'VCLinkerTool': {
                # 1 = 'LinkTimeCodeGenerationOptionUse'
                'LinkTimeCodeGeneration': '1',
              },
            },
          }],
        ],
        'msvs_settings': {
          'VCLinkerTool': {
            # /PDBALTPATH is documented in Visual C++ 2010
            # http://msdn.microsoft.com/en-us/library/dd998269(VS.100).aspx
            'AdditionalOptions': ['/PDBALTPATH:%_PDB%'],
          },
        },
      },
      'Android_Base': {
        'abstract': 1,
        'defines': [
          'OS_ANDROID',
          # For Protobuf
          'GOOGLE_PROTOBUF_NO_RTTI',
        ],
        'include_dirs!': [
          '<(third_party_dir)/breakpad/src',
        ],
        'ldflags!': [  # Remove all libraries for GNU/Linux.
          '<@(linux_ldflags)',
        ],
        'ldflags': [
          '-llog',
        ],
        'conditions': [
          # For GTEST
          ['enable_unittest==1', {
            'defines+': [
              'GTEST_HAS_CLONE=0',
              'GTEST_HAS_GLOBAL_WSTRING=0',
              'GTEST_HAS_POSIX_RE=0',
              'GTEST_HAS_STD_WSTRING=0',
              'GTEST_OS_LINUX=1',
              'GTEST_OS_LINUX_ANDROID=1',
            ],
          }],
        ],
      },
      #
      # Concrete configurations
      #
      'Debug': {
        'inherit_from': ['Common_Base', 'x86_Base', 'Debug_Base'],
      },
      'Optimize': {
        'inherit_from': ['Common_Base', 'x86_Base', 'Optimize_Base'],
      },
      'Release': {
        'inherit_from': ['Optimize', 'Release_Base'],
      },
      'conditions': [
        ['OS=="win"', {
          'Debug_x64': {
            'inherit_from': ['Common_Base', 'x64_Base', 'Debug_Base'],
          },
          'Optimize_x64': {
            'inherit_from': ['Common_Base', 'x64_Base', 'Optimize_Base'],
          },
          'Release_x64': {
            'inherit_from': ['Optimize_x64', 'Release_Base'],
          },
        }],
        ['target_platform=="Android"', {
          'Debug_Android': {
            'inherit_from': ['Common_Base', 'Android_Base', 'Debug_Base'],
          },
          'Release_Android': {
            'inherit_from': ['Common_Base', 'Android_Base', 'Optimize_Base', 'Release_Base'],
          },
        }],
        ['target_platform=="Linux" and nacl_sdk_root!=""', {
          # The following configurations, i.e. directories, are meant for binary
          # files built with an NaCl toolchain.
          # A special hack in build_for_nacl.py sets environment variables such
          # as CC, LD, etc., so it builds NaCl binaries in these directories.
          'Debug_NaCl_i686': {
            'inherit_from': ['Common_Base', 'Debug_Base'],
          },
          'Release_NaCl_i686': {
            'inherit_from': ['Common_Base', 'Optimize_Base', 'Release_Base'],
          },
          'Debug_NaCl_x86-64': {
            'inherit_from': ['Common_Base', 'Debug_Base'],
          },
          'Release_NaCl_x86-64': {
            'inherit_from': ['Common_Base', 'Optimize_Base', 'Release_Base'],
          },
        }],
      ],
    },
    'default_configuration': 'Debug',
    'defines': [
      '<@(additional_defines)',
    ],
    'include_dirs': [
      '<(DEPTH)',
      '<(third_party_dir)/breakpad/src',
      '<(SHARED_INTERMEDIATE_DIR)',
      '<(SHARED_INTERMEDIATE_DIR)/proto_out',
    ],
    'conditions': [
      # For GTEST
      ['enable_unittest==1', {
        'defines+': [
          'GTEST_HAS_TR1_TUPLE=1',
        ],
        'include_dirs+': [
          '<(third_party_dir)/gmock/include',
          '<(third_party_dir)/gtest/include',
        ],
      }],
      ['OS=="win"', {
        'defines': [
          'COMPILER_MSVC',
          'BUILD_MOZC',  # for ime_shared library
          'ID_TRACE_LEVEL=1',
          'OS_WINDOWS',
          'UNICODE',
          'WIN32',
          'WIN32_IE=0x0600',
          'WINVER=0x0501',
          '_ATL_ALL_WARNINGS',
          '_ATL_APARTMENT_THREADED',
          '_ATL_CSTRING_EXPLICIT_CONSTRUCTORS',
          '_CONSOLE',
          '_CRT_SECURE_NO_DEPRECATE',
          '_MIDL_USE_GUIDDEF_',
          '_STL_MSVC',
          '_UNICODE',
          '_USRDLL',
          '_WIN32',
          '_WIN32_WINDOWS=0x0410',
          '_WIN32_WINNT=0x0501',
          '_WINDLL',
          '_WINDOWS',
        ],
        'include_dirs': [
          '<@(msvs_includes)',
          '<(third_party_dir_win)/wtl_80/files/include',
          # Add atl_wrapper dir into the 'include_dirs' so that we can
          # include the header file as <atlbase_mozc.h>, which
          # is more lintian-friendly than "atlbase_mozc.h".
          # See b/5101916 for the background information.
          '<(DEPTH)/win32/atl_wrapper',
        ],
        # We don't have cygwin in our tree, but we need to have
        # setup_env.bat in the directory specified in 'msvs_cygwin_dirs'
        # for GYP to be happy.
        'msvs_cygwin_dirs': [
          '<(third_party_dir)/cygwin',
        ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'BufferSecurityCheck': 'true',         # /GS
            'CompileAs': '2',                      # /TP
            'DebugInformationFormat': '3',         # /Zi
            'DefaultCharIsUnsigned': 'true',       # /J
            'DisableSpecificWarnings': ['<@(msvc_disabled_warnings)'],  # /wdXXXX
            'EnableFunctionLevelLinking': 'true',  # /Gy
            'EnableIntrinsicFunctions': 'true',    # /Oi
            'ExceptionHandling': '2',              # /EHs
            'ForcedIncludeFiles': ['base/namespace.h'],
                                                   # /FI<header_file.h>
            'SuppressStartupBanner': 'true',       # /nologo
            'TreatWChar_tAsBuiltInType': 'false',  # /Zc:wchar_t-
            'WarningLevel': '3',                   # /W3
            'conditions': [
              # Unfortunately, 'OmitFramePointers': 'false' does not mean
              # /Oy- on Visual C++ 2008 or prior.  You need to use
              # 'AdditionalOptions' option to specify /Oy- instead.
              ['target_compiler=="msvs2008"', {
                'AdditionalOptions': ['/Oy-'],  # /Oy-
              }],
              ['target_compiler=="msvs2010"', {
                'OmitFramePointers': 'false',  # /Oy- (for Visual C++ 2010)
              }],
            ],
          },
          'VCLinkerTool': {
            'AdditionalDependencies': [
              'advapi32.lib',
              'comdlg32.lib',
              'delayimp.lib',
              'gdi32.lib',
              'imm32.lib',
              'kernel32.lib',
              'ole32.lib',
              'oleaut32.lib',
              'psapi.lib',
              'shell32.lib',
              'user32.lib',
              'uuid.lib',
            ],
            'AdditionalOptions': [
              '/DYNAMICBASE',  # 'RandomizedBaseAddress': 'true'
              '/NXCOMPAT',
            ],
            'EnableCOMDATFolding': '2',            # /OPT:ICF
            'GenerateDebugInformation': 'true',    # /DEBUG
            'LinkIncremental': '1',                # /INCREMENTAL:NO
            'OptimizeReferences': '2',             # /OPT:REF
            'target_conditions': [
              # /TSAWARE is valid only on executable target.
              ['_type=="executable"', {
                'TerminalServerAware': '2',        # /TSAWARE
              }],
            ],
          },
          'VCResourceCompilerTool': {
            'PreprocessorDefinitions': [
              'MOZC_RES_USE_TEMPLATE=1',
            ],
            'AdditionalIncludeDirectories': [
              '<(SHARED_INTERMEDIATE_DIR)/',
              '<(DEPTH)/',
            ],
          },
        },
      }],
      ['OS=="linux"', {
        'defines': [
          'OS_LINUX',
          'MOZC_SERVER_DIRECTORY="<@(server_dir)"',
        ],
        'cflags': [
          '<@(gcc_cflags)',
          '<@(warning_cflags)',
          '-fPIC',
          '-fno-exceptions',
        ],
        'cflags_cc': [
          # We use deprecated <hash_map> and <hash_set> instead of upcoming
          # <unordered_map> and <unordered_set>.
          '-Wno-deprecated',
        ],
        'conditions': [
          ['target_platform!="Android"', {
            'link_settings': {
              'libraries': [
                '<@(linux_libs)',
                '<@(extra_linux_libs)',
              ],
            },
          }],
          ['chromeos==1', {
            'defines': [
              'OS_CHROMEOS',
            ],
          }],
        ],
      }],
      ['OS=="mac"', {
        'defines': [
          'OS_MACOSX',
        ],
        'mac_framework_dirs': [
          '<(mac_dir)/Releases/GoogleBreakpad',
          '<(DEPTH)/mozc_build_tools/mac',
        ],
        'xcode_settings': {
          'GCC_ENABLE_CPP_EXCEPTIONS': 'NO',  # -fno-exceptions
          'OTHER_CFLAGS': [
            '<@(gcc_cflags)',
          ],
          'WARNING_CFLAGS': [
            '<@(warning_cflags)',
            '-Wno-#warnings',
          ],
          'SDKROOT': 'macosx10.5',
        },
        'link_settings': {
          'libraries': [
            '$(SDKROOT)/System/Library/Frameworks/Cocoa.framework',
            '$(SDKROOT)/System/Library/Frameworks/CoreFoundation.framework',
            '$(SDKROOT)/System/Library/Frameworks/IOKit.framework',
            '$(SDKROOT)/System/Library/Frameworks/Security.framework',
            '$(SDKROOT)/System/Library/Frameworks/SystemConfiguration.framework',
            '/usr/lib/libcrypto.dylib',
            '/usr/lib/libiconv.dylib',
            '/usr/lib/libssl.dylib',
            '/usr/lib/libz.dylib',
          ],
        }
      }],
    ],
  },
  'xcode_settings': {
    'SYMROOT': '<(build_base)',
    'GCC_INLINES_ARE_PRIVATE_EXTERN': 'YES',
  },
}
