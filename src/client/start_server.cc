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

#include "client/session.h"

#ifdef OS_WINDOWS
#include <string.h>
#include <windows.h>
#include <sddl.h>
#include <shlobj.h>
#include "third_party/mozc/sandbox/restricted_token_utils.h"
#else
#include <spawn.h>  // for posix_spawnp().
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif
#include "base/base.h"
#include "base/const.h"
#include "base/file_stream.h"
#include "base/process.h"
#include "base/run_level.h"
#include "base/util.h"
#include "ipc/ipc.h"
#include "ipc/named_event.h"
#ifdef OS_MACOSX
#include "base/mac_util.h"
#endif

namespace mozc {
namespace client {
namespace {
const char kServerName[] = "session";

// Wait at most kServerWaitTimeout msec until server gets ready
const uint32 kServerWaitTimeout = 20000;  // 20 sec

// for every 1000m sec, check server
const uint32 kRetryIntervalForServer = 1000;

// Try 20 times to check mozc_server is running
const uint32 kTrial = 20;

#ifdef _DEBUG
// Load special flags for server.
// This should be enabled on debug build
const string LoadServerFlags() {
  const char kServerFlagsFile[] = "mozc_server_flags.txt";
  const string filename = Util::JoinPath(Util::GetUserProfileDirectory(),
                                         kServerFlagsFile);
  string flags;
  InputFileStream ifs(filename.c_str());
  if (ifs) {
    getline(ifs, flags);
  }
  VLOG(1) << "New server flag: " << flags;
  return flags;
}
#endif
}  // namespace

// initialize default path
StartServerHandler::StartServerHandler()
    : server_program_(Util::GetServerPath()),
      restricted_(false) {}

StartServerHandler::~StartServerHandler() {}

bool StartServerHandler::StartServer(SessionInterface *session) {
  if (server_program().empty()) {
    LOG(ERROR) << "Server path is empty";
    return false;
  }

  // ping first
  if (session->PingServer()) {
    return true;
  }

  string arg;

#ifdef OS_WINDOWS
  // When mozc is not used as a default IME and some applications (like notepad)
  // are registered in "Start up", mozc_server may not be launched successfully.
  // This is because the Explorer launches start-up processes inside a group job
  // and the process inside a job cannot make our sandboxed child processes.
  // The group job is unregistered after 60 secs (default).
  //
  // Here we relax the sandbox restriction if process is in a job.
  // In order to keep security, the mozc_server is launched
  // with restricted mode.

  const bool process_in_job = RunLevel::IsProcessInJob();
  if (process_in_job || restricted_) {
    LOG(WARNING)
        << "Parent process is in job. start with restricted mode";
    arg += "--restricted";
  }
#endif

#ifdef _DEBUG
  // In oreder to test the Session treatment (timeout/size constratins),
  // Server flags can be configurable on DEBUG build
  if (!arg.empty()) {
    arg += " ";
  }
  arg += LoadServerFlags();
#endif  // _DEBUG

  NamedEventListener listener(kServerName);
  const bool listener_is_available = listener.IsAvailable();

  size_t pid = 0;
#ifdef OS_WINDOWS
  mozc::Process::SecurityInfo info;
  info.primary_level = sandbox::USER_INTERACTIVE;
  info.impersonation_level = sandbox::USER_RESTRICTED_SAME_ACCESS;
  info.job_level =  process_in_job ?
      sandbox::JOB_NO_JOB : sandbox::JOB_LOCKDOWN;
  info.integrity_level = sandbox::INTEGRITY_LEVEL_LOW;
  info.allow_ui_operation = false;
  info.in_system_dir = true;  // use system dir not to lock current directory
  info.creation_flags = CREATE_DEFAULT_ERROR_MODE | CREATE_NO_WINDOW;

  DWORD tmp_pid = 0;
  const bool result = mozc::Process::SpawnProcessAs(
      server_program(), arg, info, &tmp_pid);
  pid = static_cast<size_t>(tmp_pid);

  if (!result) {
    LOG(ERROR) << "Can't start process: " << ::GetLastError();
    return false;
  }
#elif defined(OS_MACOSX)
  // Use launchd API instead of spawning process.  It doesn't use
  // server_program() at all.
  const bool result = MacUtil::StartLaunchdServce(
      "Converter", reinterpret_cast<pid_t *>(&pid));
  if (!result) {
      LOG(ERROR) << "Can't start process";
      return false;
    }
#else
  const bool result = mozc::Process::SpawnProcess(server_program(),
                                                  arg,
                                                  &pid);
  if (!result) {
    LOG(ERROR) << "Can't start process: " << strerror(result);
    return false;
  }
#endif  // OS_WINDOWS

  // maybe another process will launch mozc_server at the same time.
  if (session->PingServer()) {
    VLOG(1) << "Another process has launched the server";
    return true;
  }

  // Common part:
  // Wait until mozc_server becomes ready to process requests
  if (listener_is_available) {
    const int ret = listener.WaitEventOrProcess(kServerWaitTimeout, pid);
    switch (ret) {
      case NamedEventListener::TIMEOUT:
        LOG(WARNING) << "seems that Google Japanese Input Converter is not "
                     << "ready within " << kServerWaitTimeout << " msec";
        break;
      case NamedEventListener::EVENT_SIGNALED:
        VLOG(1) << "Google Japanese Input Converter is launched successfully "
                << "within " << kServerWaitTimeout << " msec";
        break;
      case NamedEventListener::PROCESS_SIGNALED:
        LOG(ERROR) << "Mozc server is terminated";
        // Mozc may be terminated because another client launches mozc_server
        if (session->PingServer()) {
          return true;
        }
        return false;
    }
  } else {
    // maybe another process is trying to launch mozc_server.
    LOG(ERROR) << "cannot make NamedEventListener ";
    Util::Sleep(kRetryIntervalForServer);
  }

  // Try to connect mozc_server just in case.
  for (int trial = 0; trial < kTrial; ++trial) {
    if (session->PingServer()) {
      return true;
    }
    Util::Sleep(kRetryIntervalForServer);
  }

  LOG(ERROR) << "Google Japanese Input Converter cannot be launched";

  return false;
}

bool StartServerHandler::ForceTerminateServer(const string &name) {
  return IPCClient::TerminateServer(name);
}

bool StartServerHandler::WaitServer(uint32 pid) {
  const int kTimeout = 10000;
  return Process::WaitProcess(static_cast<size_t>(pid), kTimeout);
}

void StartServerHandler::OnFatal(
    StartServerHandlerInterface::ServerErrorType type) {
  LOG(ERROR) << "OnFatal is called: " << static_cast<int>(type);

  string error_type;
  switch (type) {
    case StartServerHandlerInterface::SERVER_TIMEOUT:
      error_type = "server_timeout";
      break;
    case StartServerHandlerInterface::SERVER_BROKEN_MESSAGE:
      error_type = "server_broken_message";
      break;
    case StartServerHandlerInterface::SERVER_VERSION_MISMATCH:
      error_type = "server_version_mismatch";
      break;
    case StartServerHandlerInterface::SERVER_SHUTDOWN:
      error_type = "server_shutdown";
      break;
    case StartServerHandlerInterface::SERVER_FATAL:
      error_type = "server_fatal";
      break;
    default:
      LOG(ERROR) << "Unknown error: " << type;
      return;
  }

  Process::LaunchErrorMessageDialog(error_type);
}
}  // client
}  // mozc
