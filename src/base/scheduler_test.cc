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

#include "base/scheduler.h"

#include "base/util.h"
#include "base/mutex.h"
#include "testing/base/public/gunit.h"

namespace mozc {
namespace {

static int g_counter1 = 0;
static Mutex g_counter_mutex1;
bool TestFuncOk1(void *data) {
  scoped_lock l(&g_counter_mutex1);
  ++g_counter1;
  return true;
}

static int g_counter2 = 0;
static Mutex g_counter_mutex2;
bool TestFuncOk2(void *data) {
  scoped_lock l(&g_counter_mutex2);
  ++g_counter2;
  return true;
}

static int g_counter_ng = 0;
bool TestFuncNg(void *data) {
  ++g_counter_ng;
  return false;
}

static int g_num = 0;
bool TestFunc(void *num) {
  CHECK(num);
  g_num = *(reinterpret_cast<int *>(num));
  return true;
}
}  // namespace


TEST(SchedulerTest, SchedulerTestData) {
  const string kTestJob = "Test";
  g_num = 0;
  int num = 10;
  Scheduler::AddJob(kTestJob, 100000, 100000, 500, 0, &TestFunc, &num);
  EXPECT_EQ(0, g_num);
  Util::Sleep(1000);
  EXPECT_EQ(10, g_num);
  Scheduler::RemoveJob(kTestJob);
}

TEST(SchedulerTest, SchedulerTestDelay) {
  const string kTestJob = "Test";
  g_counter1 = 0;
  Scheduler::AddJob(kTestJob, 100000, 100000, 500, 0, &TestFuncOk1, NULL);
  EXPECT_EQ(0, g_counter1);
  Util::Sleep(1000);
  EXPECT_EQ(1, g_counter1);
  Scheduler::RemoveJob(kTestJob);
}

TEST(SchedulerTest, SchedulerTestRandomDelay) {
  int total_run = 0;
  int num_run[6] = {0, 0, 0, 0, 0, 0};
  for (int i = 0; i < 100; ++i) {
    const string kTestJob = "Test";
    g_counter1 = 0;
    Scheduler::AddJob(kTestJob, 100000, 100000, 100, 500, &TestFuncOk1, NULL);
    EXPECT_EQ(0, g_counter1);
    bool incremented = false;
    Util::Sleep(100);  // delay
    if (g_counter1 == 1 && !incremented) {
      // corner case. just after fixed time has passed.
      ++total_run;
      ++num_run[0];
      incremented = true;
    }
    Util::Sleep(100);
    if (g_counter1 == 1 && !incremented) {
      ++total_run;
      ++num_run[1];
      incremented = true;
    }
    Util::Sleep(100);
    if (g_counter1 == 1 && !incremented) {
      ++total_run;
      ++num_run[2];
      incremented = true;
    }
    Util::Sleep(100);
    if (g_counter1 == 1 && !incremented) {
      ++total_run;
      ++num_run[3];
      incremented = true;
    }
    Util::Sleep(100);
    if (g_counter1 == 1 && !incremented) {
      ++total_run;
      ++num_run[4];
      incremented = true;
    }
    Util::Sleep(100);
    EXPECT_EQ(1, g_counter1);
    if (g_counter1 == 1 && !incremented) {
      ++total_run;
      ++num_run[5];
    }
    Scheduler::RemoveJob(kTestJob);
  }
  EXPECT_EQ(100, total_run);
  EXPECT_LT(0, num_run[1]);
  EXPECT_LT(0, num_run[2]);
  EXPECT_LT(0, num_run[3]);
  EXPECT_LT(0, num_run[4]);
  EXPECT_LT(0, num_run[5]);
}

TEST(SchedulerTest, SchedulerTestNoDelay) {
  const string kTestJob = "Test";
  g_counter1 = 0;
  Scheduler::AddJob(kTestJob, 1000, 1000, 0, 0, &TestFuncOk1, NULL);
  Util::Sleep(500);
  EXPECT_EQ(1, g_counter1);
  Scheduler::RemoveJob(kTestJob);
}

// Update each variables for each tasks and count the number each
// functions are called.
TEST(SchedulerTest, SchedulerTestInterval) {
  const string kTestJob1 = "Test1";
  const string kTestJob2 = "Test2";
  {
    g_counter1 = 0;
    Scheduler::AddJob(kTestJob1, 1000, 1000, 500, 0, &TestFuncOk1, NULL);

    Util::Sleep(3000);
    EXPECT_EQ(3, g_counter1);
    Scheduler::RemoveJob(kTestJob1);

    Util::Sleep(3000);
    EXPECT_EQ(3, g_counter1);
  }
  {
    g_counter1 = 0;
    Scheduler::AddJob(kTestJob1, 1000, 1000, 500, 0, &TestFuncOk1, NULL);

    Util::Sleep(3000);
    EXPECT_EQ(3, g_counter1);

    g_counter2 = 0;
    Scheduler::AddJob(kTestJob2, 1000, 1000, 500, 0, &TestFuncOk2, NULL);

    Util::Sleep(3000);
    EXPECT_EQ(6, g_counter1);
    EXPECT_EQ(3, g_counter2);
    Scheduler::RemoveJob(kTestJob1);

    Util::Sleep(3000);
    EXPECT_EQ(6, g_counter1);
    EXPECT_EQ(6, g_counter2);
  }
  Scheduler::RemoveJob(kTestJob2);

  Util::Sleep(3000);
  EXPECT_EQ(6, g_counter1);
  EXPECT_EQ(6, g_counter2);
}

TEST(SchedulerTest, SchedulerTestRemoveAll) {
  const string kTestJob1 = "Test1";
  const string kTestJob2 = "Test2";

  g_counter1 = 0;
  g_counter2 = 0;
  Scheduler::AddJob(kTestJob1, 1000, 1000, 500, 0, &TestFuncOk1, NULL);
  Scheduler::AddJob(kTestJob2, 1000, 1000, 500, 0, &TestFuncOk2, NULL);
  Util::Sleep(3000);
  EXPECT_EQ(3, g_counter1);
  EXPECT_EQ(3, g_counter2);

  Scheduler::RemoveAllJobs();

  Util::Sleep(3000);
  EXPECT_EQ(3, g_counter1);
  EXPECT_EQ(3, g_counter2);
}

TEST(SchedulerTest, SchedulerTestFailed) {
  const string kTestJob = "Test";
  g_counter_ng = 0;
  Scheduler::AddJob(kTestJob, 1000, 5000, 500, 0, &TestFuncNg, NULL);

  Util::Sleep(1000);  // 1000 count=1 next 2000
  EXPECT_EQ(1, g_counter_ng);
  Util::Sleep(1000);  // 2000
  EXPECT_EQ(1, g_counter_ng);
  Util::Sleep(1000);  // 3000 count=2 next 3000
  EXPECT_EQ(2, g_counter_ng);
  Util::Sleep(3000);  // 6000 count=4 next 5000
  EXPECT_EQ(3, g_counter_ng);
  Util::Sleep(5000);  // 11000 count=4 next 5000
  EXPECT_EQ(4, g_counter_ng);
  Util::Sleep(5000);  // 16000
  EXPECT_EQ(5, g_counter_ng);

  Scheduler::RemoveJob(kTestJob);
}
}  // namespace mozc
