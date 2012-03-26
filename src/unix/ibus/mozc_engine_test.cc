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

#include <ibus.h>
#include <map>
#include "base/util.h"
#include "base/port.h"
#include "client/client_mock.h"
#include "session/commands.pb.h"
#include "session/key_event_util.h"
#include "testing/base/public/gunit.h"
#include "unix/ibus/mozc_engine.h"

namespace mozc {
namespace ibus {

class MozcEngineTest : public testing::Test {
 protected:
  virtual void SetUp() {
    is_non_modifier_key_pressed_ = false;
    currently_pressed_modifiers_.clear();
    modifiers_to_be_sent_.clear();
    keyval_to_modifier_.clear();
    keyval_to_modifier_[IBUS_Shift_L] = commands::KeyEvent::LEFT_SHIFT;
    keyval_to_modifier_[IBUS_Shift_R] = commands::KeyEvent::RIGHT_SHIFT;
    keyval_to_modifier_[IBUS_Control_L] = commands::KeyEvent::LEFT_CTRL;
    keyval_to_modifier_[IBUS_Control_R] = commands::KeyEvent::RIGHT_CTRL;
    keyval_to_modifier_[IBUS_Alt_L] = commands::KeyEvent::LEFT_ALT;
    keyval_to_modifier_[IBUS_Alt_R] = commands::KeyEvent::RIGHT_ALT;
  }

  void Clear() {
    is_non_modifier_key_pressed_ = false;
    currently_pressed_modifiers_.clear();
    modifiers_to_be_sent_.clear();
  }

  // Currently this function does not supports special keys.
  void AppendToKeyEvent(gint keyval, commands::KeyEvent *key) const {
    const map<gint, commands::KeyEvent::ModifierKey>::const_iterator it =
        keyval_to_modifier_.find(keyval);
    if (it != keyval_to_modifier_.end()) {
      key->add_modifier_keys(it->second);
    } else {
      key->set_key_code(keyval);
    }
  }

  bool ProcessKey(bool is_key_up, gint keyval, commands::KeyEvent *key) {
    AppendToKeyEvent(keyval, key);

    return MozcEngine::ProcessModifiers(is_key_up,
                                        keyval,
                                        key,
                                        &is_non_modifier_key_pressed_,
                                        &currently_pressed_modifiers_,
                                        &modifiers_to_be_sent_);
  }

  bool ProcessKeyWithCapsLock(bool is_key_up, gint keyval,
                              commands::KeyEvent *key) {
    key->add_modifier_keys(commands::KeyEvent::CAPS);
    return ProcessKey(is_key_up, keyval, key);
  }

  bool IsPressed(gint keyval) const {
    return currently_pressed_modifiers_.end() !=
        currently_pressed_modifiers_.find(keyval);
  }

  bool is_non_modifier_key_pressed_;
  set<gint> currently_pressed_modifiers_;
  set<commands::KeyEvent::ModifierKey> modifiers_to_be_sent_;
  map<gint, commands::KeyEvent::ModifierKey> keyval_to_modifier_;
};

class LaunchToolTest : public testing::Test {
 public:
  LaunchToolTest() {
    g_type_init();
  }

 protected:
  virtual void SetUp() {
    mozc_engine_.reset(new MozcEngine());

    mock_ = new client::ClientMock();
    mock_->ClearFunctionCounter();
    mozc_engine_->client_.reset(mock_);
  }

  virtual void TearDown() {
    mozc_engine_.reset(NULL);
  }

  client::ClientMock* mock_;
  scoped_ptr<MozcEngine> mozc_engine_;

 private:
  DISALLOW_COPY_AND_ASSIGN(LaunchToolTest);
};

TEST_F(LaunchToolTest, LaunchToolTest) {
  commands::Output output;

  // Launch config dialog
  mock_->ClearFunctionCounter();
  mock_->SetBoolFunctionReturn("LaunchToolWithProtoBuf", true);
  output.set_launch_tool_mode(commands::Output::CONFIG_DIALOG);
  EXPECT_TRUE(mozc_engine_->LaunchTool(output));

  // Launch dictionary tool
  mock_->ClearFunctionCounter();
  mock_->SetBoolFunctionReturn("LaunchToolWithProtoBuf", true);
  output.set_launch_tool_mode(commands::Output::DICTIONARY_TOOL);
  EXPECT_TRUE(mozc_engine_->LaunchTool(output));

  // Launch word register dialog
  mock_->ClearFunctionCounter();
  mock_->SetBoolFunctionReturn("LaunchToolWithProtoBuf", true);
  output.set_launch_tool_mode(commands::Output::WORD_REGISTER_DIALOG);
  EXPECT_TRUE(mozc_engine_->LaunchTool(output));

  // Launch no tool(means do nothing)
  mock_->ClearFunctionCounter();
  mock_->SetBoolFunctionReturn("LaunchToolWithProtoBuf", false);
  output.set_launch_tool_mode(commands::Output::NO_TOOL);
  EXPECT_FALSE(mozc_engine_->LaunchTool(output));

  // Something occurring in client::Client::LaunchTool
  mock_->ClearFunctionCounter();
  mock_->SetBoolFunctionReturn("LaunchToolWithProtoBuf", false);
  output.set_launch_tool_mode(commands::Output::CONFIG_DIALOG);
  EXPECT_FALSE(mozc_engine_->LaunchTool(output));
}

TEST_F(MozcEngineTest, ProcessShiftModifiers) {
  commands::KeyEvent key;

  // 'Shift-a' senario
  // Shift down
  EXPECT_FALSE(ProcessKey(false, IBUS_Shift_L, &key));
  EXPECT_TRUE(IsPressed(IBUS_Shift_L));
  // Doesn't use EXPECT_NE because it can't handle iterator on some environment.
  EXPECT_TRUE(modifiers_to_be_sent_.end() !=
              modifiers_to_be_sent_.find(commands::KeyEvent::LEFT_SHIFT));
  EXPECT_TRUE(modifiers_to_be_sent_.end() !=
              modifiers_to_be_sent_.find(commands::KeyEvent::SHIFT));
  EXPECT_EQ(modifiers_to_be_sent_.size(), 2);

  // "a" down
  key.Clear();
  EXPECT_TRUE(ProcessKey(false, 'a', &key));
  EXPECT_FALSE(IsPressed(IBUS_Shift_L));
  EXPECT_EQ(modifiers_to_be_sent_.size(), 0);

  // "a" up
  key.Clear();
  EXPECT_FALSE(ProcessKey(true, 'a', &key));
  EXPECT_FALSE(IsPressed(IBUS_Shift_L));
  EXPECT_EQ(modifiers_to_be_sent_.size(), 0);

  // Shift up
  key.Clear();
  EXPECT_FALSE(ProcessKey(true, IBUS_Shift_L, &key));
  EXPECT_TRUE(currently_pressed_modifiers_.empty());
  EXPECT_TRUE(modifiers_to_be_sent_.empty());

  /* Currently following test scenario does not pass.
   * This bug was issued as b/4338394
  // 'Shift-0' senario
  // Shift down
  EXPECT_FALSE(ProcessKey(false, IBUS_Shift_L, &key));
  EXPECT_TRUE(IsPressed(IBUS_Shift_L));
  EXPECT_TRUE(modifiers_to_be_sent_.end() !=
              modifiers_to_be_sent_.find(commands::KeyEvent::SHIFT));
  EXPECT_EQ(modifiers_to_be_sent_.size(), 1);

  // "0" down
  key.Clear();
  EXPECT_TRUE(ProcessKey(false, '0', &key));
  EXPECT_TRUE(IsPressed(IBUS_Shift_L));
  EXPECT_EQ(modifiers_to_be_sent_.size(), 0);

  // "0" up
  key.Clear();
  EXPECT_FALSE(ProcessKey(true, '0', &key));
  EXPECT_TRUE(IsPressed(IBUS_Shift_L));
  EXPECT_EQ(modifiers_to_be_sent_.size(), 0);

  // Shift up
  key.Clear();
  EXPECT_TRUE(ProcessKey(true, IBUS_Shift_L, &key));
  EXPECT_TRUE(currently_pressed_modifiers_.empty());
  EXPECT_TRUE(modifiers_to_be_sent_.empty());
  */
}

TEST_F(MozcEngineTest, ProcessAltModifiers) {
  commands::KeyEvent key;

  // Alt down
  EXPECT_FALSE(ProcessKey(false, IBUS_Alt_L, &key));
  EXPECT_TRUE(IsPressed(IBUS_Alt_L));
  // Doesn't use EXPECT_NE because it can't handle iterator on some environment.
  EXPECT_TRUE(modifiers_to_be_sent_.end() !=
              modifiers_to_be_sent_.find(commands::KeyEvent::ALT));
  EXPECT_TRUE(modifiers_to_be_sent_.end() !=
              modifiers_to_be_sent_.find(commands::KeyEvent::LEFT_ALT));
  EXPECT_EQ(modifiers_to_be_sent_.size(), 2);

  // "a" down
  key.Clear();
  key.add_modifier_keys(commands::KeyEvent::ALT);
  key.add_modifier_keys(commands::KeyEvent::LEFT_ALT);
  EXPECT_TRUE(ProcessKey(false, 'a', &key));
  EXPECT_TRUE(IsPressed(IBUS_Alt_L));
  EXPECT_TRUE(modifiers_to_be_sent_.empty());

  // "a" up
  key.Clear();
  key.add_modifier_keys(commands::KeyEvent::ALT);
  key.add_modifier_keys(commands::KeyEvent::LEFT_ALT);
  EXPECT_FALSE(ProcessKey(true, 'a', &key));
  EXPECT_TRUE(IsPressed(IBUS_Alt_L));
  EXPECT_TRUE(modifiers_to_be_sent_.empty());

  // ALt up
  key.Clear();
  EXPECT_FALSE(ProcessKey(true, IBUS_Alt_L, &key));
  EXPECT_TRUE(currently_pressed_modifiers_.empty());
  EXPECT_TRUE(modifiers_to_be_sent_.empty());
}

TEST_F(MozcEngineTest, ProcessCtrlModifiers) {
  commands::KeyEvent key;

  // Ctrl down
  EXPECT_FALSE(ProcessKey(false, IBUS_Control_L, &key));
  EXPECT_TRUE(IsPressed(IBUS_Control_L));
  // Doesn't use EXPECT_NE because it can't handle iterator on some environment.
  EXPECT_TRUE(modifiers_to_be_sent_.end() !=
              modifiers_to_be_sent_.find(commands::KeyEvent::CTRL));
  EXPECT_TRUE(modifiers_to_be_sent_.end() !=
              modifiers_to_be_sent_.find(commands::KeyEvent::LEFT_CTRL));
  EXPECT_EQ(modifiers_to_be_sent_.size(), 2);

  // "a" down
  key.Clear();
  key.add_modifier_keys(commands::KeyEvent::CTRL);
  key.add_modifier_keys(commands::KeyEvent::LEFT_CTRL);
  EXPECT_TRUE(ProcessKey(false, 'a', &key));
  EXPECT_TRUE(IsPressed(IBUS_Control_L));
  EXPECT_TRUE(modifiers_to_be_sent_.empty());

  // "a" up
  key.Clear();
  key.add_modifier_keys(commands::KeyEvent::CTRL);
  key.add_modifier_keys(commands::KeyEvent::LEFT_CTRL);
  EXPECT_FALSE(ProcessKey(true, 'a', &key));
  EXPECT_TRUE(IsPressed(IBUS_Control_L));
  EXPECT_TRUE(modifiers_to_be_sent_.empty());

  // Ctrl up
  key.Clear();
  EXPECT_FALSE(ProcessKey(true, IBUS_Control_L, &key));
  EXPECT_TRUE(currently_pressed_modifiers_.empty());
  EXPECT_TRUE(modifiers_to_be_sent_.empty());
}

TEST_F(MozcEngineTest, ProcessShiftModifiersWithCapsLockOn) {
  commands::KeyEvent key;

  // 'Shift-a' senario
  // Shift down
  EXPECT_FALSE(ProcessKeyWithCapsLock(false, IBUS_Shift_L, &key));
  EXPECT_TRUE(IsPressed(IBUS_Shift_L));
  // Doesn't use EXPECT_NE because it can't handle iterator on some environment.
  EXPECT_TRUE(modifiers_to_be_sent_.end() !=
              modifiers_to_be_sent_.find(commands::KeyEvent::CAPS));
  EXPECT_TRUE(modifiers_to_be_sent_.end() !=
              modifiers_to_be_sent_.find(commands::KeyEvent::SHIFT));
  EXPECT_TRUE(modifiers_to_be_sent_.end() !=
              modifiers_to_be_sent_.find(commands::KeyEvent::LEFT_SHIFT));
  EXPECT_EQ(modifiers_to_be_sent_.size(), 3);

  // "a" down
  key.Clear();
  EXPECT_TRUE(ProcessKeyWithCapsLock(false, 'a', &key));
  EXPECT_FALSE(IsPressed(IBUS_Shift_L));
  EXPECT_TRUE(modifiers_to_be_sent_.empty());

  // "a" up
  key.Clear();
  EXPECT_FALSE(ProcessKeyWithCapsLock(true, 'a', &key));
  EXPECT_FALSE(IsPressed(IBUS_Shift_L));
  EXPECT_TRUE(modifiers_to_be_sent_.empty());

  // Shift up
  key.Clear();
  EXPECT_FALSE(ProcessKeyWithCapsLock(true, IBUS_Shift_L, &key));
  EXPECT_TRUE(currently_pressed_modifiers_.empty());
  EXPECT_TRUE(modifiers_to_be_sent_.empty());
}

TEST_F(MozcEngineTest, LeftRightModifiers) {
  commands::KeyEvent key;

  // Left-Shift down
  EXPECT_FALSE(ProcessKey(false, IBUS_Shift_L, &key));
  EXPECT_TRUE(IsPressed(IBUS_Shift_L));
  // Doesn't use EXPECT_NE because it can't handle iterator on some environment.
  EXPECT_TRUE(modifiers_to_be_sent_.end() !=
              modifiers_to_be_sent_.find(commands::KeyEvent::SHIFT));
  EXPECT_TRUE(modifiers_to_be_sent_.end() !=
              modifiers_to_be_sent_.find(commands::KeyEvent::LEFT_SHIFT));
  EXPECT_EQ(modifiers_to_be_sent_.size(), 2);

  // Right-Shift down
  key.Clear();
  key.add_modifier_keys(commands::KeyEvent::SHIFT);
  key.add_modifier_keys(commands::KeyEvent::LEFT_SHIFT);
  EXPECT_FALSE(ProcessKey(false, IBUS_Shift_R, &key));
  EXPECT_TRUE(IsPressed(IBUS_Shift_L));
  EXPECT_TRUE(IsPressed(IBUS_Shift_R));
  EXPECT_TRUE(modifiers_to_be_sent_.end() !=
              modifiers_to_be_sent_.find(commands::KeyEvent::SHIFT));
  EXPECT_TRUE(modifiers_to_be_sent_.end() !=
              modifiers_to_be_sent_.find(commands::KeyEvent::LEFT_SHIFT));
  EXPECT_TRUE(modifiers_to_be_sent_.end() !=
              modifiers_to_be_sent_.find(commands::KeyEvent::RIGHT_SHIFT));
  EXPECT_EQ(modifiers_to_be_sent_.size(), 3);
}

TEST_F(MozcEngineTest, ProcessModifiers) {
  commands::KeyEvent key;

  // Shift down => Shift up
  key.Clear();
  ProcessKey(false, IBUS_Shift_L, &key);

  key.Clear();
  EXPECT_TRUE(ProcessKey(true, IBUS_Shift_L, &key));
  EXPECT_TRUE(currently_pressed_modifiers_.empty());
  EXPECT_TRUE(modifiers_to_be_sent_.empty());
  EXPECT_EQ((commands::KeyEvent::SHIFT | commands::KeyEvent::LEFT_SHIFT),
            KeyEventUtil::GetModifiers(key));

  // Shift down => Ctrl down => Shift up => Alt down => Ctrl up => Alt up
  key.Clear();
  ProcessKey(false, IBUS_Shift_L, &key);
  key.Clear();
  EXPECT_FALSE(ProcessKey(false, IBUS_Control_L, &key));
  key.Clear();
  EXPECT_FALSE(ProcessKey(true, IBUS_Shift_L, &key));
  key.Clear();
  EXPECT_FALSE(ProcessKey(false, IBUS_Alt_L, &key));
  key.Clear();
  EXPECT_FALSE(ProcessKey(true, IBUS_Control_L, &key));
  key.Clear();
  EXPECT_TRUE(ProcessKey(true, IBUS_Alt_L, &key));
  EXPECT_TRUE(currently_pressed_modifiers_.empty());
  EXPECT_TRUE(modifiers_to_be_sent_.empty());
  EXPECT_EQ((commands::KeyEvent::ALT | commands::KeyEvent::LEFT_ALT |
             commands::KeyEvent::CTRL | commands::KeyEvent::LEFT_CTRL |
             commands::KeyEvent::SHIFT | commands::KeyEvent::LEFT_SHIFT),
            KeyEventUtil::GetModifiers(key));
}

TEST_F(MozcEngineTest, ProcessModifiersRandomTest) {
  // This test generates random key sequence and check that
  // - All states are cleared when all keys are released.
  // - All states are cleared when a non-modifier key with no modifier keys
  //   is pressed / released.

  const gint kKeySet[] = {
    IBUS_Alt_L,
    IBUS_Alt_R,
    IBUS_Control_L,
    IBUS_Control_R,
    IBUS_Shift_L,
    IBUS_Shift_R,
    IBUS_Caps_Lock,
    IBUS_a,
  };
  const size_t kKeySetSize = arraysize(kKeySet);
  Util::SetRandomSeed(static_cast<uint32>(Util::GetTime()));

  const int kTrialNum = 10000;
  for (int trial = 0; trial < kTrialNum; ++trial) {
    Clear();
    set<gint> pressed_keys;
    string key_sequence;

    const int kSequenceLength = 100;
    for (int i = 0; i < kSequenceLength; ++i) {
      const int key_index = Util::Random(kKeySetSize);
      const gint key_value = kKeySet[key_index];

      bool is_key_up;
      if (pressed_keys.find(key_value) == pressed_keys.end()) {
        pressed_keys.insert(key_value);
        is_key_up = false;
      } else {
        pressed_keys.erase(key_value);
        is_key_up = true;
      }

      key_sequence += Util::StringPrintf("is_key_up: %d, key_index = %d\n",
                                         is_key_up, key_index);

      commands::KeyEvent key;
      for (set<gint>::const_iterator it = pressed_keys.begin();
           it != pressed_keys.end(); ++it) {
        AppendToKeyEvent(*it, &key);
      }

      ProcessKey(is_key_up, key_value, &key);

      if (pressed_keys.empty()) {
        SCOPED_TRACE("key_sequence:\n" + key_sequence);
        EXPECT_FALSE(is_non_modifier_key_pressed_);
        EXPECT_TRUE(currently_pressed_modifiers_.empty());
        EXPECT_TRUE(modifiers_to_be_sent_.empty());
      }
    }

    // Anytime non-modifier key without modifier key should clear states.
    commands::KeyEvent key;
    const gint non_modifier_key = IBUS_b;
    AppendToKeyEvent(non_modifier_key, &key);
    ProcessKey(false, non_modifier_key, &key);

    {
      const bool is_key_up = static_cast<bool>(Util::Random(2));
      SCOPED_TRACE(Util::StringPrintf(
          "Should be reset by non_modifier_key %s. key_sequence:\n%s",
          (is_key_up ? "up" : "down"), key_sequence.c_str()));
      EXPECT_FALSE(is_non_modifier_key_pressed_);
      EXPECT_TRUE(currently_pressed_modifiers_.empty());
      EXPECT_TRUE(modifiers_to_be_sent_.empty());
    }
  }
}

}  // namespace ibus
}  // namespace mozc
