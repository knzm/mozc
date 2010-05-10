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

#include <string>
#include <vector>
#include <map>
#include "base/base.h"
#include "base/file_stream.h"
#include "base/util.h"
#include "converter/pos_util.h"

// Input: id.def, user-pos.def, cforms.def
// Output: pos_data.h
DEFINE_string(id_file, "", "");
DEFINE_string(user_pos_file, "", "");
DEFINE_string(cforms_file, "", "");
DEFINE_string(output, "", "");
DECLARE_bool(logtostderr);

namespace mozc {

namespace {
string Escape(const string &str) {
  string output;
  Util::Escape(str, &output);
  return output;
}

struct ConjugationType {
  string form;
  string value_suffix;
  string key_suffix;
};

void LoadConjugation(const string &filename,
                    map<string, vector<ConjugationType> > *output) {
  InputFileStream ifs(filename.c_str());
  CHECK(ifs);

  string line;
  vector<string> fields;
  while (getline(ifs, line)) {
    if (line.empty() || line[0] == '#') {
      continue;
    }
    fields.clear();
    Util::SplitStringUsing(line, "\t ", &fields);
    CHECK_GE(fields.size(), 4);

    ConjugationType tmp;
    tmp.form = fields[1];
    tmp.value_suffix = fields[2] == "*" ? "" : fields[2];
    tmp.key_suffix   = fields[3] == "*" ? "" : fields[3];
    (*output)[fields[0]].push_back(tmp);   // insert
  }
}

void Convert() {
  mozc::POSUtil util;
  util.Open(FLAGS_id_file);

  map<string, vector<ConjugationType> > inflection_map;
  LoadConjugation(FLAGS_cforms_file, &inflection_map);

  InputFileStream ifs(FLAGS_user_pos_file.c_str());
  ostream *ofs = &cout;
  if (!FLAGS_output.empty()) {
    ofs = new OutputFileStream(FLAGS_output.c_str());
  }
  CHECK(ifs);
  CHECK(*ofs);
  string line;
  vector<string> fields, pos_fields;
  vector<pair<string, size_t> > pos_tokens;

  while (getline(ifs, line)) {
    if (line.empty() || line[0] == '#') {
      continue;
    }
    fields.clear();
    Util::SplitStringUsing(line, "\t ", &fields);
    CHECK_GE(fields.size(), 3);
    const string &user_pos = fields[0];
    const string ctype = fields[1];
    const string &feature = fields[2];

    if (ctype == "*") {
      const uint16 id = util.id(fields[2]);
      CHECK_NE(id, 0);
      *ofs << "static const ConjugationType kConjugation" << pos_tokens.size()
           << "[] = {" << endl;
      *ofs << "  { NULL, NULL, " << id << "}" << endl;
      *ofs << "};" << endl;
      pos_tokens.push_back(make_pair(user_pos, static_cast<size_t>(1)));
    } else {
      vector<ConjugationType> &forms = inflection_map[ctype];
      CHECK(!forms.empty());
      *ofs << "const ConjugationType kConjugation"
           << pos_tokens.size() << "[] = {" << endl;
      bool is_first = true;;
      size_t added = 0;
      for (size_t i = 0; i < forms.size(); ++i) {
        // repalce <cfrom> with actual cform
        string output;
        Util::StringReplace(feature, "<cform>", forms[i].form, true, &output);
        const uint16 id = util.id(output);
        if (id == 0) {
          LOG(ERROR) << "Cannot find id for:" << output;
          continue;
        }
        if (!is_first) {
          *ofs << ",";
        }
        *ofs << "  { ";
        *ofs << "\"" << Escape(forms[i].value_suffix) << "\"" << ", ";
        *ofs << "\"" << Escape(forms[i].key_suffix)    << "\"" << ", ";
        *ofs << id << " }";
        is_first = false;
        ++added;
      }
      CHECK_GT(added, 0);
      *ofs << "};" << endl;
      pos_tokens.push_back(make_pair(user_pos, forms.size()));
    }
  }

  *ofs << "const POSToken kPOSToken[] = {" << endl;
  for (size_t i = 0; i < pos_tokens.size(); ++i) {
    *ofs << "  { \"" << Escape(pos_tokens[i].first) << "\", "
         << pos_tokens[i].second << ", kConjugation" << i << " }," << endl;
  }
  *ofs << "  { NULL, 0, NULL }" << endl;
  *ofs << "};" << endl;

  *ofs << "const uint16 kZipcodeId = "
       << util.zipcode_id() << ";" << endl;

  const vector<uint16> &number_lids = util.number_ids();
  *ofs << "const uint16 kNumberId[] = {";
  for (size_t i = 0; i < number_lids.size(); ++i) {
    if (i != 0) {
      *ofs << ", ";
    }
    *ofs << number_lids[i];
  }
  *ofs << " };" << endl;

  const vector<uint16> &functional_lids = util.functional_word_ids();
  *ofs << "const uint16 kFunctionalWordId[] = {";
  for (size_t i = 0; i < functional_lids.size(); ++i) {
    if (i != 0) {
      *ofs << ", ";
    }
    *ofs << functional_lids[i];
  }
  *ofs << " };" << endl;

  if (ofs != &cout) {
    delete ofs;
  }
}
}  // namespace
}  // namespace mozc

int main(int argc, char **argv) {
  FLAGS_logtostderr = true;
  InitGoogle(argv[0], &argc, &argv, false);

  if (FLAGS_id_file.empty() &&
      FLAGS_user_pos_file.empty() &&
      FLAGS_cforms_file.empty() &&
      argc > 3) {
    FLAGS_id_file = argv[1];
    FLAGS_user_pos_file = argv[2];
    FLAGS_cforms_file = argv[3];
  }

  LOG(INFO) << FLAGS_id_file;
  LOG(INFO) << FLAGS_user_pos_file;
  LOG(INFO) << FLAGS_cforms_file;

  mozc::Convert();

  return 0;
}
