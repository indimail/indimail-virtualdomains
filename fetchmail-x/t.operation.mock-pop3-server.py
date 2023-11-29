#!/usr/bin/env python3
# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-

# Copyright (C) 2019 Bryce W. Harrington
# Copyright (C) 2022 Matthias Andree
#
# Released under GNU GPLv2 or later, read the file 'LICENSE.GPLv2+' for
# more information.
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# Author:  Bryce Harrington <bryce@canonical.com>

import os
import sys
import socket
from tempfile import mkstemp
from pathlib import Path

DEBUGGING = True
DEFAULT_HOST = ''
DEFAULT_PORT = 11110
NEWLINE = b"\r\n"
MESSAGE_CONTENT = b'''
From: test@example.com\r
Subject: Test message\r
\r
This is a body for testing\r
\r
'''

def dbg(msg):
    """Prints information to stdout if debugging is enabled"""
    if DEBUGGING:
        sys.stdout.write("{}\n".format(msg))

def command_user(data, msg):
    return b'+OK user accepted'

def command_pass(data, msg):
    return b'+OK pass accepted'

def command_stat(data, msg):
    return b'+OK 1 %i' %(msg.size)

def command_list(data, msg):
    if data == b'LIST':
        return b'+OK 1 messages (%i octets)'%(msg.size) \
            + NEWLINE \
            + b'1 %i'%(msg.size) \
            + NEWLINE \
            + b'.'
    else:
        cmd, num = data.split()
        return b'+OK 1 (%i octects)'%(msg.size)

def command_last(data, msg):
    return b'+OK 0'

def command_top(data, msg):
    cmd, num, lines = data.split()
    assert num == b'1', "unknown message number: {num}".format(num)
    bottom = NEWLINE.join(msg.bottom[:int(lines)])
    text = msg.top + NEWLINE + NEWLINE + bottom
    dbg(text)
    return b'+OK top of message follows%s' % (NEWLINE + text + NEWLINE + b'.')

def command_retr(data, msg):
    return b'+OK %i octets'%(msg.size) \
        + NEWLINE \
        + msg.data \
        + NEWLINE \
        + b'.'

def command_dele(data, msg):
    return b'+OK 1 %i'%(msg.size)

def command_noop(data, msg):
    return b'+OK 1 %i'%(msg.size)

def command_quit(data, msg):
    return b'+OK mock pop3 server signing off'

COMMANDS = {
    b'USER' : command_user,
    b'PASS' : command_pass,
    b'STAT' : command_stat,
    b'LIST' : command_list,
    b'LAST' : command_last,
    b'TOP'  : command_top,
    b'RETR' : command_retr,
    b'DELE' : command_dele,
    b'NOOP' : command_noop,
    b'QUIT' : command_quit,
}

class Mailbox:
    """Encapsulates a mailbox containing a single email message"""
    def __init__(self, message_filename):
        with open(message_filename, "rb") as msg:
            data = msg.read()
            self.data = data
            self.size = len(data)
            self.top, rest = data.split(NEWLINE + NEWLINE, 1)
            self.bottom = rest.split(NEWLINE)


class Mailserver:
    def __init__(self, conn, mbox):
        self._conn = conn
        self._mbox = mbox
        self._chunk_size = 4096

    def close(self):
        dbg("  - Server exiting")
        self._conn.close()

    def send(self, data):
        dbg("  - Server sending")
        self._conn.sendall(data + NEWLINE)

    def receive(self):
        dbg("  - Server receiving")
        data = []
        while True:
            chunk = self._conn.recv(self._chunk_size)
            if NEWLINE in chunk:
                data.append(chunk[:chunk.index(NEWLINE)])
                break
            data.append(chunk)
        return b"".join(data)

    def process(self):
        data = self.receive()
        dbg("  - Received: '{}'".format(data))
        command = data.split(None, 1)[0].upper()
        if command in COMMANDS.keys():
            response = COMMANDS[command](data, self._mbox)
            dbg("  - Response: {}".format(response))
            try:
                self.send(response)
                if command == b'QUIT':
                    return False
            except BrokenPipeError:
                dbg("  - Client terminated connection")
                return False
        else:
            self.send(b"-ERR unrecognized command")
        return True

def serve(messages_filename, host=DEFAULT_HOST, port=DEFAULT_PORT):
    dbg("Serving for {} on {}".format(host, port))

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind((host, port))

    mailbox = Mailbox(messages_filename)

    try:
        sock.listen(1)
        dbg("* mock-pop3-server ready on {}:{}".format(host, port))
        while True:
            sock.listen(1)
            startmarker = os.environ.get("STARTMARKER")
            if startmarker:
                try:
                    Path(startmarker).touch()
                except FileNotFoundError:
                    # work directory has gone away, so we have not
                    # started in time.
                    os.exit(1)
            conn, address = sock.accept()
            dbg("* Connection from {}".format(address))
            server = Mailserver(conn, mailbox)
            server.send(b"+OK mock-pop3-server pop3 server ready")
            try:
                dbg("* Processing commands for connection...")
                while server.process():
                    dbg("* Finished command")
                    pass
            finally:
                dbg("* Closing remote connection")
                server.close()
    except KeyboardInterrupt:
        dbg("* mock-pop3-server interrupted")
        return 1
    except SystemExit:
        dbg("* mock-pop3-server exiting")
        return 1
    finally:
        sock.shutdown(socket.SHUT_RDWR)
        sock.close()
        return 0


if __name__ == "__main__":
    fd, message_filename = mkstemp(prefix='message-', suffix='.txt')
    dbg("Creating message file '{}'".format(message_filename))

    with open(message_filename, 'wb') as f:
        f.write(MESSAGE_CONTENT.lstrip())

    try:
        exit(serve(message_filename, host=DEFAULT_HOST, port=DEFAULT_PORT))
    except KeyError:
        sys.stderr.write("Exiting\n")
        sys.exit(1)
