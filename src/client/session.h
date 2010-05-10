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

// A class handling the IPC connection for the session b/w server and clients.

#ifndef MOZC_CLIENT_SESSION_H_
#define MOZC_CLIENT_SESSION_H_

#include <string>
#include <vector>
#include "base/base.h"
#include "session/commands.pb.h"
#include "client/session_interface.h"

namespace mozc {
class IPCClientFactoryInterface;

namespace config {
class Config;
}  // config

namespace client {

// default StartServerHanlder implemntation.
// This class uses fork&exec (linux/mac) and CreateProcess() (Windows)
// to launch server process
class StartServerHandler : public StartServerHandlerInterface {
 public:
  bool StartServer(SessionInterface *session);

  bool ForceTerminateServer(const string &name);

  bool WaitServer(uint32 pid);

  void OnFatal(StartServerHandlerInterface::ServerErrorType type);

  // specify server program. On Mac, we need to specify the server path
  // using this method.
  void set_server_program(const string &server_program) {
    server_program_ = server_program;
  }

  // return server program
  const string &server_program() const {
    return server_program_;
  }

  void set_restricted(bool restricted) {
    restricted_ = restricted;
  }

  StartServerHandler();
  virtual ~StartServerHandler();

 private:
  string server_program_;
  bool restricted_;
};

class Session : public SessionInterface {
 public:
  Session();
  virtual ~Session();
  void SetIPCClientFactory(IPCClientFactoryInterface *client_factory);

  // set StartServerHandler.
  // StartServerHandler is used as default
  // NOTE: Session class takes the owership of start_server_handler.
  void SetStartServerHandler(StartServerHandlerInterface *start_server_handler);

  // return true if connection is alive.
  // If connection is not available,  re-launch mozc_server internally.
  bool EnsureConnection();

  // return true if session id is valid.
  // if session id is invalid, re-issue a valid sssion id.
  bool EnsureSession();

  // Check protocol/product version.
  // If a  new version is avaialable, restart the server.
  // return true the server is available.
  // return false some error happend during the server restart.
  // This method calls EnsureConnection automatically.
  bool CheckVersionOrRestartServer();

  // SendKey/TestSendKey/SendCommand automatically
  // make a connection and issue an session id
  // if valid session id is not found.
  bool SendKey(const commands::KeyEvent &key,
               commands::Output *output);
  bool TestSendKey(const commands::KeyEvent &key,
                   commands::Output *output);
  bool SendCommand(const commands::SessionCommand &command,
                   commands::Output *output);

  // The methods below don't call
  // StartServer if server is not available. This treatment
  // avoids unexceptional and continuous server restart trials.
  // If you really want to ensure the connection,
  // call EnsureConnection() in advance
  bool GetConfig(config::Config *config);
  bool SetConfig(const config::Config &config);

  bool ClearUserHistory();
  bool ClearUserPrediction();
  bool ClearUnusedUserPrediction();
  bool Shutdown();
  bool SyncData();
  bool Reload();
  bool Cleanup();

  // This method is similar to PingServer(), but the internal
  // state may change. In almost all cases, you don't need to
  // call this method
  bool NoOperation();

  // Ping server:
  // This method will never change the internal state.
  bool PingServer() const;

  // Reset internal state (changs the state to be SERVER_UNKNWON)
  void Reset();

  // Enables or disables using cascading window.
  void EnableCascadingWindow(bool enable);

  // Sets the time out in milli second used for the IPC connection.
  void set_timeout(int timeout);

  // Dump the recent user inputs to specified file with label
  // This is used for debugging
  void DumpHistorySnapshot(const string &filename,
                           const string &label) const;

 private:
  enum ServerStatus {
    SERVER_UNKNOWN,          // initial status
    SERVER_SHUTDOWN,         // server is currently not working
    SERVER_INVALID_SESSION,  // current session is not available
    SERVER_OK,               // both server and session are health
    SERVER_TIMEOUT,          // server is blocked
    SERVER_VERSION_MISMATCH, // server version is different
    SERVER_BROKEN_MESSAGE,   // server's message is broken
    SERVER_FATAL             // cannot start server (binary is broken/missing)
  };

  // Start server:
  // return true if server is launched sucessfully or server is already running.
  // return false if server cannot be launched.
  // If server_program is empty, which is default setting, the path to
  // GoogleJapaneseInputConverter is determined automatically.
  // Windows: "C:\Program Files\Google\Google Japanese Input\"
  // Linux/Mac: searching from default path
  bool StartServer();

  // Displays a message box to notify the user of fatal error.
  void OnFatal(StartServerHandlerInterface::ServerErrorType type);

  // Initialize input filling id and preferences.
  void InitInput(commands::Input *input) const;

  bool CreateSession();
  bool DeleteSession();
  bool CallCommand(commands::Input::CommandType type);

  // This method automatically re-launch mozc_server and
  // re-issue session id if it is not available.
  bool EnsureCallCommand(commands::Input *input,
                         commands::Output *output);

  // The most primitive Call method
  // This method won't change the server_status_ even
  // when version mismatch happens. In this case,
  // just return false.
  bool Call(const commands::Input &input,
            commands::Output *output);

  // first invoke Call() command and check the
  // protocol_version. When protocol version mismatch,
  // client goes to FATAL state
  bool CallAndCheckVersion(const commands::Input &input,
                           commands::Output *output);

  // Making a journal inputs to restore
  // the current state even when mozc_server crashes
  void PlaybackHistory();
  void PushHistory(const commands::Input &input,
                   const commands::Output &output);
  // Returns true if the |key| corresponds to the key combination for aborting
  // the application.
  bool IsAbortKey(const commands::KeyEvent &key);

  // The alias of
  // DumpHistorySnapshot("query_of_death.log", "QUERY OF DEATH");
  // and history_inputs_.clear();
  void DumpQueryOfDeath();

  // Execute |input| and check the version by seeing the
  // initial response. If a new version is available, automatically
  // restart the server and exectute the same input command again.
  // If any errors happen inside the version up, shows an error dialog
  // and returns false.
  bool CheckVersionOrRestartServerInternal(const commands::Input &input,
                                           commands::Output *output);

  uint64 id_;
  IPCClientFactoryInterface *client_factory_;
  scoped_ptr<StartServerHandlerInterface> start_server_handler_;
  scoped_array<char> result_;
  scoped_ptr<config::Config> preferences_;
  bool use_cascading_window_;
  int timeout_;
  ServerStatus server_status_;
  uint32 server_protocol_version_;
  uint32 server_process_id_;
  string server_product_version_;
  vector<commands::Input> history_inputs_;
};

}  // namespace client
}  // namespace mozc
#endif  // MOZC_CLIENT_SESSION_H_
