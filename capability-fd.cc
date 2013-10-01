/*
 * Tests for Capsicum, a capability API for UNIX.
 *
 * Copyright (C) 2012 The Chromium OS Authors <chromium-os-dev@chromium.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#include "gtest/gtest.h"
#include "capsicum.h"

// Run a test case in a forked process, so that trapdoors don't
// affect other tests.
#define FORK_TEST(test_case_name, test_name)                   \
    static int test_case_name##_##test_name##_ForkTest();      \
    TEST(test_case_name, test_name ## Forked) {                \
      pid_t pid = fork();                                      \
      if (pid == 0) {                                          \
        test_case_name##_##test_name##_ForkTest();             \
        exit(HasFailure());                                    \
      } else if (pid > 0) {                                    \
        int status;                                            \
        waitpid(pid, &status, 0);                              \
        int rc = WIFEXITED(status) ? WEXITSTATUS(status) : -1; \
        EXPECT_EQ(0, rc);                                      \
      }                                                        \
    }                                                          \
    static int test_case_name##_##test_name##_ForkTest()

TEST(Capability, CapNew) {
  int cap_fd = cap_new(STDOUT_FILENO, CAP_READ|CAP_WRITE|CAP_SEEK);
  EXPECT_NE(-1, cap_fd);
  if (cap_fd > 0) {
    EXPECT_EQ(4, write(cap_fd, "OK!\n", 4));
    EXPECT_EQ(0, close(cap_fd));
  }
}

FORK_TEST(Capability, CapEnter) {
  EXPECT_EQ(0, cap_enter());
}

FORK_TEST(Capability, BasicInterception) {
  // TODO(drysdale): check on use of 0 for rights
  int cap_fd = cap_new(1, 0);
  EXPECT_NE(-1, cap_fd);

  int rc = write(cap_fd, "", 0);
  // TODO(drysdale): this test is written to assume that rights are not
  // enforced until cap_enter() occurs, which I don't think is right.
  EXPECT_EQ(0, rc);

  EXPECT_EQ(0, cap_enter());

  rc = write(cap_fd, "", 0);
  EXPECT_EQ(-1, rc);
  EXPECT_EQ(ENOTCAPABLE, errno);

  // Create a new capability which does have write permission
  cap_fd = cap_new(1, CAP_WRITE|CAP_SEEK);
  EXPECT_NE(-1, cap_fd);
  rc = write(cap_fd, "", 0);
  EXPECT_EQ(0, rc);
}

FORK_TEST(Capability, OpenAtDirectoryTraversal) {
  int dir = open("/tmp", O_RDONLY);
  EXPECT_LE(0, dir);

  cap_enter();

  int file = openat(dir, "testfile", O_RDONLY|O_CREAT);
  EXPECT_LE(0, file);

  // Test that we are confined to /tmp, and cannot
  // escape using absolute paths or ../.
  file = openat(dir, "../dev/null", O_RDONLY);
  EXPECT_EQ(-1, file);

  file = openat(dir, "..", O_RDONLY);
  EXPECT_EQ(-1, file);

  file = openat(dir, "/dev/null", O_RDONLY);
  EXPECT_EQ(-1, file);

  file = openat(dir, "/", O_RDONLY);
  EXPECT_EQ(-1, file);
  close(dir);
}

// Create a capability on /tmp that does not allow CAP_WRITE,
// and check that this restriction is inherited through openat().
FORK_TEST(Capability, Inheritance) {
  int dir = open("/tmp", O_RDONLY);
  EXPECT_LE(0, dir);
  int dircap = cap_new(dir, CAP_READ|CAP_LOOKUP);

  const char *fn = "testfile";
  int file = openat(dir, fn, O_WRONLY|O_CREAT);
  EXPECT_LE(0, file);
  EXPECT_EQ(5, write(file, "TEST\n", 5));
  close(file);

  EXPECT_EQ(0, cap_enter());
  file = openat(dircap, "testfile", O_RDONLY);
  EXPECT_LE(0, file);
  if (file > 0) close(file);

  file = openat(dircap, "testfile", O_WRONLY|O_APPEND);
  EXPECT_EQ(-1, file);
  EXPECT_EQ(ENOTCAPABLE, errno);
  if (file > 0) close(file);
}