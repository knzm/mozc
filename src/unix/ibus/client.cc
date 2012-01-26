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

#include "unix/ibus/client.h"

#include "base/base.h"
#include "base/singleton.h"
#include "session/commands.pb.h"
#include "session/session_handler.h"

namespace mozc {
namespace ibus {

// Session Handler must be singleton per user.
class StandaloneSessionHandler {
 public:
  StandaloneSessionHandler()
    : handler_(new SessionHandler) {}

  bool EvalCommand(commands::Command *command) {
    // SessionHandler::EvalCommand is not thread safe.
    // When IBusMozc creates multiple Session instances and
    // calls EvalCommand simultaneously, EvalCommand will crashes.
    // To prevent such situation, do a giant lock on EvalCommand
    // just in case. Seems that IBusMozc will never create
    // multiple instances.
    scoped_lock l(&mutex_);
    return handler_->EvalCommand(command);
  }

  void SetSessionFactory(session::SessionFactoryInterface *new_factory) {
    scoped_lock l(&mutex_);
    handler_->SetSessionFactory(new_factory);
  }

 private:
  Mutex mutex_;
  scoped_ptr<SessionHandler> handler_;
};

Client::Client()
    : id_(0),
      handler_(Singleton<StandaloneSessionHandler>::get()) {
}

Client::~Client() {
  DeleteSession();
}

bool Client::IsValidRunLevel() const {
  return true;
}

bool Client::EnsureSession() {
  if (id_ != 0) {
    VLOG(1) << "session has been already created.";
    return true;
  }
  return CreateSession();
}

bool Client::EnsureConnection() {
  return true;
}

bool Client::SendKey(const commands::KeyEvent &key,
                      commands::Output *output) {
  commands::Input input;
  input.set_type(commands::Input::SEND_KEY);
  input.mutable_key()->CopyFrom(key);
  return EnsureCallCommand(&input, output);
}

bool Client::TestSendKey(const commands::KeyEvent &key,
                          commands::Output *output) {
  commands::Input input;
  input.set_type(commands::Input::TEST_SEND_KEY);
  input.mutable_key()->CopyFrom(key);
  return EnsureCallCommand(&input, output);
}

bool Client::SendCommand(const commands::SessionCommand &command,
                          commands::Output *output) {
  commands::Input input;
  input.set_type(commands::Input::SEND_COMMAND);
  input.mutable_command()->CopyFrom(command);
  return EnsureCallCommand(&input, output);
}

bool Client::CheckVersionOrRestartServer() {
  return true;
}

bool Client::EnsureCallCommand(commands::Input *input,
                                commands::Output *output) {
  if (!EnsureSession()) {
    LOG(ERROR) << "EnsureSession failed";
    return false;
  }

  input->set_id(id_);
  output->set_id(0);
  return Call(*input, output);
}

void Client::EnableCascadingWindow(bool enable) {
  // TODO(mazda): implement this
}

void Client::set_timeout(int timeout) {
}

void Client::set_restricted(bool restricted) {
}

void Client::set_server_program(const string &program_path) {
}

void Client::set_client_capability(const commands::Capability &capability) {
  client_capability_.CopyFrom(capability);
}

bool Client::LaunchTool(const string &mode,
                         const string &extra_arg) {
  return true;
}

bool Client::LaunchToolWithProtoBuf(const commands::Output &output) {
  return true;
}

bool Client::OpenBrowser(const string &url) {
  return true;
}

bool Client::CreateSession() {
  if (id_ != 0) {
    VLOG(1) << "session has been already created.";
    return true;
  }

  commands::Command command;
  command.mutable_input()->set_type(commands::Input::CREATE_SESSION);
  command.mutable_input()->mutable_capability()->CopyFrom(client_capability_);
  if (!handler_->EvalCommand(&command)) {
    LOG(ERROR) << "EvalCommand failed";
    return false;
  }
  id_ = command.output().id();
  return true;
}

bool Client::DeleteSession() {
  // No need to delete session
  if (id_ == 0) {
    VLOG(1) << "session has been already deleted.";
    return true;
  }

  commands::Command command;
  command.mutable_input()->set_id(id_);
  command.mutable_input()->set_type(commands::Input::DELETE_SESSION);
  bool result = handler_->EvalCommand(&command);
  id_ = 0;
  return result;
}

bool Client::GetConfig(config::Config *config) {
  commands::Input input;
  input.set_id(id_);
  input.set_type(commands::Input::GET_CONFIG);

  commands::Output output;
  if (!Call(input, &output)) {
    return false;
  }

  if (!output.has_config()) {
    return false;
  }

  config->Clear();
  config->CopyFrom(output.config());
  return true;
}

bool Client::SetConfig(const config::Config &config) {
  commands::Input input;
  input.set_id(id_);
  input.set_type(commands::Input::SET_CONFIG);
  input.mutable_config()->CopyFrom(config);

  commands::Output output;
  if (!Call(input, &output)) {
    return false;
  }

  return true;
}

bool Client::ClearUserHistory() {
  return CallCommand(commands::Input::CLEAR_USER_HISTORY);
}

bool Client::ClearUserPrediction() {
  return CallCommand(commands::Input::CLEAR_USER_PREDICTION);
}

bool Client::ClearUnusedUserPrediction() {
  return CallCommand(commands::Input::CLEAR_UNUSED_USER_PREDICTION);
}

bool Client::Shutdown() {
  CallCommand(commands::Input::SHUTDOWN);
  return true;
}

bool Client::SyncData() {
  return CallCommand(commands::Input::SYNC_DATA);
}

bool Client::Reload() {
  return CallCommand(commands::Input::RELOAD);
}

bool Client::Cleanup() {
  return CallCommand(commands::Input::CLEANUP);
}

bool Client::NoOperation() {
  return CallCommand(commands::Input::NO_OPERATION);
}

bool Client::PingServer() const {
  return true;
}


bool Client::CallCommand(commands::Input::CommandType type) {
  commands::Input input;
  input.set_id(id_);
  input.set_type(type);
  commands::Output output;
  return Call(input, &output);
}

bool Client::Call(const commands::Input &input,
                   commands::Output *output) {
  DCHECK(output) << "output is null";

  commands::Command command;
  command.mutable_input()->CopyFrom(input);
  if (!handler_->EvalCommand(&command)) {
    LOG(ERROR) << "EvalCommand failed.";
    return false;
  }
  output->CopyFrom(command.output());
  return true;
}

void Client::Reset() {
}

void Client::SetSessionFactory(
    session::SessionFactoryInterface *new_factory) {
  Singleton<StandaloneSessionHandler>::get()->SetSessionFactory(new_factory);
}

void Client::SetIPCClientFactory(IPCClientFactoryInterface *interface) {
  // Do nothing because ChromeOS does not adopt multi-process architecture.
}

void Client::SetServerLauncher(client::ServerLauncherInterface *interface) {
  // Do nothing because ChromeOS does not adopt multi-process architecture.
}

}  // namespace ibus
}  // namespace mozc
