#!/usr/bin/env python

# This file should unit test dattocli
# Keep track of failures, but don't exit on the first one
# If no failures, then exit 0, otherwise exit 1

# Create a server that listens on /var/datto/dattod_ipc
# Call each of dattocli start/stop/progress of dattocli and
# make sure the server gets those messages. Reply from the server,
# and make sure the reply is shown on stdout. (probably just send a unique
# string and make sure it's there with str.find)

import unittest
import os
import subprocess

class TestDattoCli(unittest.TestCase):
    def setUp(self):
        # Create the server
        pass

    def test_startbackup_goodargs(self):
        pass

    def test_startbackup_badargs(self):
        pass

    def test_stopbackup_goodargs(self):
        pass

    def test_stopbackup_badargs(self):
        pass

    def test_progress_goodargs(self):
        pass

    def test_progress_badargs(self):
        pass

    def test_badcommand(self):
        process = subprocess.Popen(["dattocli/dattocli", "notacommand"],
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE)
        # We are lazy with the pipe buffer, if this ever deadlocks then revisit
        process.wait()

        self.assertNotEqual(process.returncode, 0)

        line = process.stdout.readline()
        self.assertNotEqual(line.find("usage"), -1)

if __name__ == '__main__':
    unittest.main()
