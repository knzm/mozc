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

/**
 * @fileoverview This file contains ASCII input mode class, which is used from
 * ime.js.
 */

'use strict';

console.assert(
    skk.mode, 'ime_mode_interface.js must be loaded prior to this module');

/**
 * Initializes ASCII direct input mode.
 * This mode does nothing; Just receives key event and pass it to system.
 * @constructor
 * @implements {skk.mode.InputModeInterface}
 * @param {skk.IME} ime IME instance.
 */
skk.mode.Ascii = function(ime) {
  this.ime_ = ime;
};

/**
 * @inheritDoc
 */
skk.mode.Ascii.prototype.enter = function() {};

/**
 * @inheritDoc
 */
skk.mode.Ascii.prototype.leave = function() {};

/**
 * @inheritDoc
 */
skk.mode.Ascii.prototype.suspend = function() {};

/**
 * @inheritDoc
 */
skk.mode.Ascii.prototype.resume = function() {};

/**
 * @inheritDoc
 */
skk.mode.Ascii.prototype.prepareForKey = function(keyEvent) {
  if (keyEvent.key === 'j' && keyEvent.ctrlKey) {
    this.ime_.switchMode(skk.InputMode.KANA);
    return null;
  }

  return keyEvent;
};

/**
 * @inheritDoc
 */
skk.mode.Ascii.prototype.addKey = function(keyEvent) { return false; };
