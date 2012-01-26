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

#ifndef MOZC_SYNC_OAUTH2_TOKEN_UTIL_H_
#define MOZC_SYNC_OAUTH2_TOKEN_UTIL_H_

#include <string>
#include "base/port.h"

namespace mozc {
namespace sync {
// Common util for parsing OAuth2 access token.
// Google's OAuth2 page http://code.google.com/apis/accounts/docs/OAuth2.html
// says:
// | On many platforms, your application should be able to monitor the window
// | title of a browser window it creates and close the window when it sees a
// | valid response.
class OAuth2TokenUtil {
 public:
  // Returns access token from the given window title.
  // Returns an empty string if it fails to parse the window title.
  // On Windows, typical browsers sets their window title as follows when the
  // access token is granted from the Google's OAuth2 server.
  // Google Chrome 15.0.874.106 m
  //   "Success code=4/a1B2c3D4e5F6g7H8i9J1k2l3M4n5 - Google Chrome"
  // Mozilla Firefox 8.0
  //   "Success code=4/a1B2c3D4e5F6g7H8i9J1k2l3M4n5 - Mozilla Firefox"
  // Opera 11.52
  //   "Success code=4/a1B2c3D4e5F6g7H8i9J1k2l3M4n5 - Opera"
  // Microsoft Internet Explorer 9
  //   "Success code=4/a1B2c3D4e5F6g7H8i9J1k2l3M4n5 - Windows Internet Explorer"
  static string ParseAuthCodeFromWindowTitleForWindows(const string &title);

  // Mostly same as "ForWindows", but Mac APIs do not have application
  // names in the window title, so we don't need to take care of the
  // remaining part of " - Google Chrome".
  static string ParseAuthCodeFromWindowTitleForMac(const string &title);

 private:
  OAuth2TokenUtil() {}
  ~OAuth2TokenUtil() {}
  DISALLOW_COPY_AND_ASSIGN(OAuth2TokenUtil);
};
}  // sync
}  // mozc

#endif  // MOZC_SYNC_OAUTH2_TOKEN_UTIL_H_
