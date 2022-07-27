//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <__assert>
#include <__config>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#ifdef __BIONIC__
#  include <android/api-level.h>
#  include <syslog.h>
extern "C" void android_set_abort_message(const char* msg);
#endif

#if defined(__APPLE__) && __has_include(<CrashReporterClient.h>)
#  include <CrashReporterClient.h>
#endif

_LIBCPP_BEGIN_NAMESPACE_STD

_LIBCPP_WEAK
void __libcpp_assertion_handler(char const* format, ...) {
  // Write message to stderr. We do this before formatting into a
  // buffer so that we still get some information out if that fails.
  {
    va_list list;
    va_start(list, format);
    std::vfprintf(stderr, format, list);
    va_end(list);
  }

  // Format the arguments into an allocated buffer for CrashReport & friends.
  // We leak the buffer on purpose, since we're about to abort() anyway.
  char* buffer; (void)buffer;
  va_list list;
  va_start(list, format);

#if defined(__APPLE__) && __has_include(<CrashReporterClient.h>)
  // Note that we should technically synchronize accesses here (by e.g. taking a lock),
  // however concretely we're only setting a pointer, so the likelihood of a race here
  // is low.
  vasprintf(&buffer, format, list);
  CRSetCrashLogMessage(buffer);
#elif defined(__BIONIC__)
  // Show error in tombstone.
  vasprintf(&buffer, format, list);
  android_set_abort_message(buffer);

  // Show error in logcat.
  openlog("libc++", 0, 0);
  syslog(LOG_CRIT, "%s", buffer);
  closelog();
#endif
  va_end(list);

  std::abort();
}

_LIBCPP_END_NAMESPACE_STD
