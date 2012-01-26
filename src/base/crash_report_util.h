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

#ifndef MOZC_BASE_WIN32_CRASH_REPORT_UTIL_H_
#define MOZC_BASE_WIN32_CRASH_REPORT_UTIL_H_

#include <string>

#include "base/port.h"

namespace mozc {
// Provides utility functions used for crash reporting.
class CrashReportUtil {
 public:
  // Returns the directory path crash dumps are stored.
  static string GetCrashReportDirectory();

  // Returns true if valid crash dump exists in the crash report directory.
  static bool CrashDumpExists();

  // Returns an integer value encoded with the current date in UTC.
  // e.g. If the current date is "April 10, 2009", 20090410 is returned.
  static int GetCurrentDate();

  // Records the current date to the latest report file, which is stored in the
  // crash report directory.
  static bool WriteLatestReport(int date);

  // Reads the latest time when the crash dump was sent from the latest report
  // file.
  static bool ReadLatestReport(int *date);

  // Return the file name which is encoded with |crash_id| and |version|.
  static string EncodeDumpFileName(const string &crash_id,
                                   const string &version);

  // Retrieves |crash_id| and |version| from the encoded file name.
  static bool DecodeDumpFileName(const string &filename,
                                 string *crash_id,
                                 string *version);

  // Returns true if the |crash_id| is valid format.
  static bool ValidateCrashId(const string &crash_id);

  // Returns true if |version| is valid format.
  static bool ValidateVersion(const string &version);

  // Abort the running application.
  // This function only works on debug build and returns true if this function
  // does not make the application abort for some reasons.
  // On release build just returns false.
  static bool Abort();

 private:
  // Returns the path of the last report file. This file reocrds the latest time
  // when the crash dump is sent.
  static string GetLatestReportPath();

  // Disallow all constructors, destructors, and operator=.
  CrashReportUtil();
  ~CrashReportUtil();
  DISALLOW_COPY_AND_ASSIGN(CrashReportUtil);
};
}  // namespace mozc

#endif  // MOZC_BASE_WIN32_CRASH_REPORT_UTIL_H_
