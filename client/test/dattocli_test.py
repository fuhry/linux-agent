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
import time

IPC_SOCKET_PATH = '/tmp/dattocli_test'
DATTO_CLI_PATH = 'dattocli/dattocli'
DATTO_SRV_PATH = 'dattocli/dattosrv'

class TestDattoCli(unittest.TestCase):
    def setUp(self):
        self.dattosrv = subprocess.Popen([DATTO_SRV_PATH])
        time.sleep(0.33)
        pass


    def tearDown(self):
        self.dattosrv.kill()
        if os.path.exists(IPC_SOCKET_PATH):
            os.remove(IPC_SOCKET_PATH)
        pass


    def test_startbackup_goodargs(self):
        process = subprocess.Popen([DATTO_CLI_PATH, 'startbackup', '/dev/null'],
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE)
        process.wait()
        self.assertEqual(process.returncode, 0)
        pass


    def test_startbackup_badargs(self):
        process = subprocess.Popen([DATTO_CLI_PATH, 'startbackup', '/dev/null', '?'],
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE)
        process.wait()
        self.assertNotEqual(process.returncode, 0)
        pass


    def test_startbackup_badargs_2(self):
        process = subprocess.Popen([DATTO_CLI_PATH, 'startbackup'],
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE)
        process.wait()
        self.assertNotEqual(process.returncode, 0)
        pass


    def test_stopbackup_goodargs(self):
        process = subprocess.Popen([DATTO_CLI_PATH, 'stopbackup', '/dev/null'],
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE)
        process.wait()
        self.assertEqual(process.returncode, 0)
        pass


    def test_stopbackup_badargs(self):
        process = subprocess.Popen([DATTO_CLI_PATH, 'stopbackup', '/dev/null', '?'],
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE)
        process.wait()
        self.assertNotEqual(process.returncode, 0)
        pass


    def test_stopbackup_badargs_2(self):
        process = subprocess.Popen([DATTO_CLI_PATH, 'stopbackup'],
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE)
        process.wait()
        self.assertNotEqual(process.returncode, 0)
        pass


    def test_progress_goodargs(self):
        process = subprocess.Popen([DATTO_CLI_PATH, 'progress', '/dev/null'],
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE)
        process.wait()
        self.assertEqual(process.returncode, 0)
        pass


    def test_progress_badargs(self):
        process = subprocess.Popen([DATTO_CLI_PATH, 'progress', '/dev/null', '?'],
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE)
        process.wait()
        self.assertNotEqual(process.returncode, 0)
        pass


    def test_progress_badargs_2(self):
        process = subprocess.Popen([DATTO_CLI_PATH, 'progress'],
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE)
        process.wait()
        self.assertNotEqual(process.returncode, 0)
        pass


    def test_badcommand(self):
        process = subprocess.Popen([DATTO_CLI_PATH, 'notacommand'],
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE)
        process.wait()
        self.assertNotEqual(process.returncode, 0)
        line = process.stdout.readline()
        self.assertNotEqual(line.find('usage'), -1)
        pass

if __name__ == '__main__':
    unittest.main()
