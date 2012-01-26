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

#include <vector>
#include "base/base.h"
#include "base/file_stream.h"
#include "base/mmap.h"
#include "converter/sparse_connector.h"
#include "storage/sparse_array_image.h"

DEFINE_bool(use_1byte_cost, false,
            "Cost is encoded int a byte, instead of 2 bytes.");
DEFINE_int32(cost_resolution, 64,
             "Cost value is calculated by the value in SparseArray * "
             "cost_resolution. So every cost value should be smaller than "
             "resolution*256.");

namespace mozc {

namespace {

size_t LoadIDSize(const string &filename) {
  InputFileStream ifs(filename.c_str());
  CHECK(ifs);
  string line;
  vector<string> fields;
  int max_id = 0;
  int line_num = 0;
  while (getline(ifs, line)) {
    fields.clear();
    Util::SplitStringUsing(line, "\t ", &fields);
    CHECK_GE(fields.size(), 2);
    const int id = atoi32(fields[0].c_str());
    max_id = max(id, max_id);
    ++line_num;
  }
  ++max_id;

  CHECK_EQ(line_num, max_id);

  return max_id;
}

size_t LoadSpecialPOSSize(const string &filename) {
  InputFileStream ifs(filename.c_str());
  CHECK(ifs);
  string line;
  int line_num = 0;
  while (getline(ifs, line)) {
    if (line.empty() || line[0] == '#') {
      continue;
    }
    ++line_num;
  }

  return line_num;
}
}  // namespace

void SparseConnectorBuilder::Compile(
    const string &text_connection_file,
    const string &id_file,
    const string &special_pos_file,
    const string &output_file) {
  const size_t id_size = LoadIDSize(id_file);
  const size_t special_pos_size = LoadSpecialPOSSize(special_pos_file);

  InputFileStream ifs(text_connection_file.c_str());
  CHECK(ifs);
  string line;
  vector<string> fields;
  getline(ifs, line);
  Util::SplitStringUsing(line, "\t ", &fields);
  CHECK_GE(fields.size(), 2);

  const uint16 original_lsize = atoi32(fields[0].c_str());
  const uint16 original_rsize = atoi32(fields[1].c_str());

  CHECK_EQ(id_size, original_lsize);
  CHECK_EQ(id_size, original_rsize);

  const uint16 lsize = original_lsize + special_pos_size;
  const uint16 rsize = original_rsize + special_pos_size;

  CHECK_EQ(lsize, rsize);

  LOG(INFO) << "Making " << lsize << " x " << rsize << " matrix";
  vector<int16> matrix(lsize * rsize);
  fill(matrix.begin(), matrix.end(), 0);

  CHECK_EQ(lsize, rsize);

  while (getline(ifs, line)) {
    fields.clear();
    Util::SplitStringUsing(line, "\t ", &fields);
    CHECK_GE(fields.size(), 3);
    const int l = atoi32(fields[0].c_str());
    const int r = atoi32(fields[1].c_str());
    int       c = atoi32(fields[2].c_str());
    CHECK(l < lsize && r < rsize) << "index values are out of range";
    // BOS->EOS connection cost is always 0
    if (l == 0 && r == 0) {
      c = 0;
    }
    matrix[(l + lsize * r)] = static_cast<int16>(c);
  }

  for (int l = original_lsize; l < lsize; ++l) {
    for (int r = 1; r < rsize; ++r) {   // SKIP EOS (r == 0)
      CHECK(l < lsize && r < rsize) << "index values are out of range";
      matrix[(l + lsize * r)] = ConnectorInterface::kInvalidCost;
    }
  }

  for (int r = original_rsize; r < rsize; ++r) {
    for (int l = 1; l < lsize; ++l) {   // SKIP BOS (r == 0)
      CHECK(l < lsize && r < rsize) << "index values are out of range";
      matrix[(l + lsize * r)] = ConnectorInterface::kInvalidCost;
    }
  }

  {
    LOG(INFO) << "compiling matrix with " << lsize * rsize;

    SparseArrayBuilder builder;
    if (FLAGS_use_1byte_cost) {
      builder.SetUse1ByteValue(true);
    }
    vector<int16> default_cost(lsize);
    fill(default_cost.begin(), default_cost.end(), static_cast<int16>(0));

    for (int lid = 0; lid < lsize; ++lid) {
      for (int rid = 0; rid < rsize; ++rid) {
        const int16 c = matrix[lid + lsize * rid];
        if (ConnectorInterface::kInvalidCost != c) {
          default_cost[lid] = max(default_cost[lid], c);  // save default cost
        }
      }
    }

    for (int lid = 0; lid < lsize; ++lid) {
      for (int rid = 0; rid < rsize; ++rid) {
        int16 c = matrix[lid + lsize * rid];
        if (c != default_cost[lid]) {
          if (FLAGS_use_1byte_cost) {
            if (c == ConnectorInterface::kInvalidCost) {
              c = SparseConnector::kInvalid1ByteCostValue;
            } else {
              c = c / FLAGS_cost_resolution;
              CHECK(c < 256 && c != SparseConnector::kInvalid1ByteCostValue);
            }
          }
          builder.AddValue(SparseConnector::EncodeKey(lid, rid), c);
        }
      }
    }

    builder.Build();

    OutputFileStream ofs(output_file.c_str(), ios::binary|ios::out);
    CHECK(ofs) << "permission denied: " << output_file;

    CHECK_EQ(lsize, default_cost.size());

    const uint16 magic = SparseConnector::kSparseConnectorMagic;
    ofs.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
    uint16 resolution = 1;
    if (FLAGS_use_1byte_cost) {
      resolution = FLAGS_cost_resolution;
    }
    ofs.write(reinterpret_cast<const char*>(&resolution), sizeof(resolution));
    ofs.write(reinterpret_cast<const char*>(&lsize), sizeof(lsize));
    ofs.write(reinterpret_cast<const char*>(&rsize), sizeof(rsize));
    ofs.write(reinterpret_cast<const char*>(&default_cost[0]),
              sizeof(default_cost[0]) * default_cost.size());
    ofs.write(builder.GetImage(), builder.GetSize());
    ofs.close();
  }

  // verify connector
  {
    Mmap<char> mmap;
    CHECK(mmap.Open(output_file.c_str(), "r"));

    scoped_ptr<SparseConnector> connector(
        new SparseConnector(mmap.begin(), mmap.GetFileSize()));
    CHECK(connector.get());

    const int resolution = connector->GetResolution();

    for (int rid = 0; rid < rsize; ++rid) {
      for (int lid = 0; lid < lsize; ++lid) {
        const int diff = abs(connector->GetTransitionCost(lid, rid) -
                             matrix[lid + lsize * rid]);
        CHECK_LT(diff, resolution);
      }
    }
  }

  return;
}
}  // namespace mozc
