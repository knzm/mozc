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

#include "dictionary/user_dictionary.h"

#include <algorithm>
#include <set>
#include <string>

#include "base/base.h"
#include "base/mutex.h"
#include "base/singleton.h"
#include "base/trie.h"
#include "config/config.pb.h"
#include "config/config_handler.h"
#include "converter/node.h"
#include "data_manager/user_dictionary_manager.h"
#include "dictionary/pos_matcher.h"
#include "dictionary/suppression_dictionary.h"
#include "dictionary/user_dictionary_storage.h"
#include "dictionary/user_dictionary_util.h"
#include "dictionary/user_pos.h"
#include "usage_stats/usage_stats.h"

namespace mozc {
namespace {

void ReloadUserDictionary() {
  VLOG(1) << "Reloading user dictionary";
  UserDictionary::GetUserDictionary()->AsyncReload();
  // Sync version:
  // UserDictionary::GetUserDictionary()->SyncReload();
}

// ReloadUserDictionary() is called by Session::Reload()
REGISTER_MODULE_RELOADER(reload_user_dictionary,
                         ReloadUserDictionary());

class POSTokenLess {
 public:
  bool operator()(const UserPOS::Token *lhs,
                  const UserPOS::Token *rhs) const {
    return lhs->key < rhs->key;
  }
};

class UserDictionaryFileManager {
 public:
  UserDictionaryFileManager() {}

  const string GetFileName() {
    scoped_lock l(&mutex_);
    if (filename_.empty()) {
      return UserDictionaryUtil::GetUserDictionaryFileName();
    } else {
      return filename_;
    }
  }

  void SetFileName(const string &filename) {
    scoped_lock l(&mutex_);
    filename_ = filename;
  }

 private:
  string filename_;
  Mutex mutex_;
  DISALLOW_COPY_AND_ASSIGN(UserDictionaryFileManager);
};
}  // namespace

class UserDictionaryReloader : public Thread {
 public:
  explicit UserDictionaryReloader(UserDictionary *dic)
      : dic_(dic) {
    DCHECK(dic_);
  }

  virtual ~UserDictionaryReloader() {
    Join();
  }

  virtual void Run() {
    scoped_ptr<UserDictionaryStorage>
        storage(
            new UserDictionaryStorage
            (Singleton<UserDictionaryFileManager>::get()->GetFileName()));
    // Load from file
    if (!storage->Load()) {
      return;
    }

    dic_->Load(*(storage.get()));
  }

 private:
  UserDictionary *dic_;
};

// TODO(noriyukit): Deprecate this method; we should explicity pass
// UserPOSInterface to user dictionary.
UserDictionary::UserDictionary()
    : user_pos_(
        UserDictionaryManager::GetUserDictionaryManager()->GetUserPOS()),
      empty_limit_(Limit()) {
  DCHECK(user_pos_);
  AsyncReload();
}

UserDictionary::UserDictionary(const UserPOSInterface *user_pos)
    : user_pos_(user_pos),
      empty_limit_(Limit()) {
  DCHECK(user_pos_);
  AsyncReload();
}

UserDictionary::~UserDictionary() {
  if (reloader_.get() != NULL) {
    reloader_->Join();
  }
  Clear();
}

bool UserDictionary::CheckReloaderAndDelete() const {
  if (reloader_.get() != NULL) {
    if (reloader_->IsRunning()) {
      return false;
    } else {
      reloader_.reset(NULL);  // remove
    }
  }

  return true;
}

Node *UserDictionary::LookupPredictiveWithLimit(
    const char *str, int size, const Limit &limit,
    NodeAllocatorInterface *allocator) const {
  if (size == 0) {
    LOG(WARNING) << "string of length zero is passed.";
    return NULL;
  }

  if (tokens_.empty()) {
    return NULL;
  }

  if (GET_CONFIG(incognito_mode)) {
    return NULL;
  }

  if (!CheckReloaderAndDelete()) {
    LOG(WARNING) << "Reloader is running";
    return NULL;
  }

  DCHECK(allocator != NULL);
  Node *result_node = NULL;
  string key(str, size);

  // Look for a starting point of iteration over dictionary contents.
  UserPOS::Token key_token;
  key_token.key = key;
  vector<UserPOS::Token *>::const_iterator it =
      lower_bound(tokens_.begin(), tokens_.end(), &key_token, POSTokenLess());

  for (; it != tokens_.end(); ++it) {
    if (!Util::StartsWith((*it)->key, key)) {
      break;
    }
    // check begin with
    if (limit.begin_with_trie != NULL) {
      string value;
      size_t key_length = 0;
      bool has_subtrie = false;
      if (!limit.begin_with_trie->LookUpPrefix((*it)->key.data() + size, &value,
                                               &key_length, &has_subtrie)) {
        continue;
      }
    }

    Node *new_node = allocator->NewNode();
    DCHECK(new_node);
    if (POSMatcher::IsSuggestOnlyWord((*it)->id)) {
      new_node->lid = POSMatcher::GetUnknownId();
      new_node->rid = POSMatcher::GetUnknownId();
    } else {
      new_node->lid = (*it)->id;
      new_node->rid = (*it)->id;
    }
    new_node->wcost = (*it)->cost;
    new_node->key = (*it)->key;
    new_node->value = (*it)->value;
    new_node->node_type = Node::NOR_NODE;
    new_node->attributes |= Node::NO_VARIANTS_EXPANSION;
    new_node->attributes |= Node::USER_DICTIONARY;
    new_node->bnext = result_node;
    result_node = new_node;
  }
  return result_node;
}

Node *UserDictionary::LookupPredictive(
    const char *str, int size, NodeAllocatorInterface *allocator) const {
  return LookupPredictiveWithLimit(str, size, empty_limit_, allocator);
}

Node *UserDictionary::LookupPrefixWithLimit(
    const char *str,
    int size,
    const Limit &limit,
    NodeAllocatorInterface *allocator) const {
  if (size == 0) {
    LOG(WARNING) << "string of length zero is passed.";
    return NULL;
  }

  if (tokens_.empty()) {
    return NULL;
  }

  if (GET_CONFIG(incognito_mode)) {
    return NULL;
  }

  if (!CheckReloaderAndDelete()) {
    LOG(WARNING) << "Reloader is running";
    return NULL;
  }

  DCHECK(allocator != NULL);
  Node *result_node = NULL;
  string key(str, size);

  // Look for a starting point of iteration over dictionary contents.
  UserPOS::Token key_token;
  key_token.key = key.substr(0, Util::OneCharLen(key.c_str()));
  vector<UserPOS::Token *>::const_iterator it =
      lower_bound(tokens_.begin(), tokens_.end(), &key_token, POSTokenLess());


  for (; it != tokens_.end(); ++it) {
    if ((*it)->key > key) {
      break;
    }

    if (POSMatcher::IsSuggestOnlyWord((*it)->id)) {
      continue;
    }

    if (!Util::StartsWith(key, (*it)->key)) {
      continue;
    }

    // check the lower limit of key length
    if ((*it)->key.size() < limit.key_len_lower_limit) {
      continue;
    }

    Node *new_node = allocator->NewNode();
    DCHECK(new_node);
    new_node->lid = (*it)->id;
    new_node->rid = (*it)->id;
    new_node->wcost = (*it)->cost;
    new_node->key = (*it)->key;
    new_node->value = (*it)->value;
    new_node->node_type = Node::NOR_NODE;
    new_node->attributes |= Node::NO_VARIANTS_EXPANSION;
    new_node->attributes |= Node::USER_DICTIONARY;
    new_node->bnext = result_node;
    result_node = new_node;
  }

  return result_node;
}

Node *UserDictionary::LookupPrefix(const char *str,
                                   int size,
                                   NodeAllocatorInterface *allocator) const {
  return LookupPrefixWithLimit(str, size, empty_limit_, allocator);
}

Node *UserDictionary::LookupReverse(const char *str, int size,
                                    NodeAllocatorInterface *allocator) const {
  if (!CheckReloaderAndDelete()) {
    LOG(WARNING) << "Reloader is running";
    return NULL;
  }

  if (GET_CONFIG(incognito_mode)) {
    return NULL;
  }

  return NULL;
}

bool UserDictionary::Reload() {
  return AsyncReload();
}

bool UserDictionary::SyncReload() {
  Clear();

  scoped_ptr<UserDictionaryStorage>
      storage(
          new UserDictionaryStorage
          (Singleton<UserDictionaryFileManager>::get()->GetFileName()));
  // Load from file
  if (!storage->Load()) {
    return false;
  }

  SuppressionDictionary::GetSuppressionDictionary()->Lock();

  return Load(*(storage.get()));
}

bool UserDictionary::AsyncReload() {
  // now loading
  if (!CheckReloaderAndDelete()) {
    return true;
  }

  SuppressionDictionary::GetSuppressionDictionary()->Lock();
  DCHECK(SuppressionDictionary::GetSuppressionDictionary()->IsLocked());

  reloader_.reset(new UserDictionaryReloader(this));
  reloader_->Start();

  return true;
}

void UserDictionary::WaitForReloader() {
  if (reloader_.get() != NULL) {
    reloader_->Join();
    reloader_.reset(NULL);
  }
}

bool UserDictionary::Load(const UserDictionaryStorage &storage) {
  Clear();

  set<uint64> seen;
  vector<UserPOS::Token> tokens;
  int sync_words_count = 0;

  SuppressionDictionary *suppression_dictionary =
      SuppressionDictionary::GetSuppressionDictionary();
  DCHECK(suppression_dictionary);
  if (!suppression_dictionary->IsLocked()) {
    LOG(ERROR) << "SuppressionDictionary must be locked first";
  }
  suppression_dictionary->Clear();

  for (size_t i = 0; i < storage.dictionaries_size(); ++i) {
    const UserDictionaryStorage::UserDictionary &dic =
        storage.dictionaries(i);
    if (!dic.enabled() || dic.entries_size() == 0) {
      continue;
    }

    if (dic.syncable()) {
      sync_words_count += dic.entries_size();
    }

    for (size_t j = 0; j < dic.entries_size(); ++j) {
      const UserDictionaryStorage::UserDictionaryEntry &entry =
          dic.entries(j);

      if (!UserDictionaryUtil::IsValidEntry(*user_pos_, entry)) {
        continue;
      }

      string tmp, reading;
      UserDictionaryUtil::NormalizeReading(entry.key(), &tmp);

      // We cannot call NormalizeVoiceSoundMark inside NormalizeReading,
      // because the normalization is user-visible.
      // http://b/2480844
      Util::NormalizeVoicedSoundMark(tmp, &reading);

      const uint64 fp = Util::Fingerprint(reading +
                                          "\t" +
                                          entry.value() +
                                          "\t" +
                                          entry.pos());
      if (!seen.insert(fp).second) {
        VLOG(1) << "Found dup item";
        continue;
      }

      // "抑制単語"
      if (entry.pos() == "\xE6\x8A\x91\xE5\x88\xB6\xE5\x8D\x98\xE8\xAA\x9E") {
        suppression_dictionary->AddEntry(reading, entry.value());
      } else {
        tokens.clear();
        user_pos_->GetTokens(reading, entry.value(), entry.pos(), &tokens);
        for (size_t k = 0; k < tokens.size(); ++k) {
          tokens_.push_back(new UserPOS::Token(tokens[k]));
        }
      }
    }
  }

  sort(tokens_.begin(), tokens_.end(), POSTokenLess());

  suppression_dictionary->UnLock();

  VLOG(1) << tokens_.size() << " user dic entries loaded";

  usage_stats::UsageStats::SetInteger("UserRegisteredWord",
                                      static_cast<int>(tokens_.size()));
  usage_stats::UsageStats::SetInteger("UserRegisteredSyncWord",
                                      sync_words_count);

  return true;
}

void UserDictionary::Clear() {
  for (vector<UserPOS::Token *>::iterator it = tokens_.begin();
       it != tokens_.end(); it++) {
    delete *it;
  }
  tokens_.clear();
}

void UserDictionary::SetUserDictionaryName(const string &filename) {
  Singleton<UserDictionaryFileManager>::get()->SetFileName(filename);
}

// TODO(noriyukit): Remove this method after completing the implementation of
// DataManager class.
UserDictionary *UserDictionary::GetUserDictionary() {
  return Singleton<UserDictionary>::get();
}
}  // namespace mozc
