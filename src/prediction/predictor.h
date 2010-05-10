// Copyright 2010, Google Inc.
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

#ifndef MOZC_PREDICTION_PREDICTOR_H_
#define MOZC_PREDICTION_PREDICTOR_H_

#include <vector>

namespace mozc {

class Segments;
class PredictorInterface;
class UserHistoryPredictor;

class Predictor {
 public:
  Predictor();
  virtual ~Predictor();

  // This method is basically called when user hit TAB key.
  virtual bool Predict(Segments *segments) const;

  // Automatic prediction. More conservative than Predict()
  virtual bool Suggest(Segments *segments) const;

  // Hook(s) for all mutable operations
  virtual void Finish(Segments *segments);

  // Revert the last Finish operation
  virtual void Revert(Segments *segments);

  // clear all history data of UserHistoryPredictor
  virtual bool ClearAllHistory();

  // clear unused history data of UserHistoryPredictor
  virtual bool ClearUnusedHistory();

  // Sync user history
  virtual bool Sync();

 public:
  UserHistoryPredictor *user_history_predictor_;
  vector<PredictorInterface *> predictors_;
};
}  // mozc

#endif  // MOZC_REWRITER_REWRITER_H_
