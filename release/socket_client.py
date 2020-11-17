#!/usr/bin/python
# -*- coding: UTF-8 -*-

import socket
import time
import traceback
import io
import os

ROOT_DIR = os.path.abspath(os.path.dirname(__file__))

i = 0

class socket_client:
    def __init__(self, port):
        self.port = port
        self._conn = None

    def connect(self):
        self._conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._conn.connect(("127.0.0.1", self.port))
        self._conn.settimeout(10)
    
    def close(self):
        self._conn.shutdown(5)
        self._conn.close()

    def recvall(self, length):
        bufs = []
        while length:
            buf = self._conn.recv(length)
            if not buf:
                return None
            bufs.append(buf)
            length -= len(buf)
        return b''.join(bufs)

    def send(self, buf):
        if self._conn is None:
            raise Exception("No connection")
        return self._conn.sendall(buf)

    def recv(self, len):
        if self._conn is None:
            raise Exception("No connection")
        return self.recvall(len)

    def send_message(self, msg):
        data = msg.encode()
        byteSize = len(data).to_bytes(4, byteorder="little")
        # byteSize = msg.ByteSize()
        # data = msg.SerializeToString()
        # print("[%s -> %s]send message %s, message size %d" % (self.port, "perfdog", str(msg), byteSize))
        #print_bytes(msg.ByteSize().to_bytes(4, byteorder="big"))
        #print_bytes(data)
        print("byteSize", byteSize)
        self.send(byteSize)
        self.send(data)

    def read_message(self):
        try:
            buffer = self.recv(4)
            print("buffer", buffer)
            byteSize = int.from_bytes(buffer, byteorder="little")
            print("recv message size %d" % (byteSize))
            # tmp = self.recv(byteSize)
            # byteSize = int.from_bytes(tmp, byteorder="little")
            # print("recv message size %d" % (byteSize))
            if byteSize > 0:
                data = bytearray()
                print("data size %d" % len(data))
                buffer = bytearray(10240)
                while byteSize > 0:
                    buffer = self.recv(min(len(buffer), byteSize))
                    data += buffer
                    byteSize -= len(buffer)
                    # print("byteSize=%d, len(data)=%d" % (byteSize, len(data)))
                print(data)
                print("recv data size %d" % len(data))
                return data
        except:
            traceback.print_exc()


if __name__ == '__main__':
    os.system('adb forward tcp:9999 localabstract:memorySnapshotTptServer ')
    s = socket_client(9999)
    s.connect()
    msg = "begin"
    s.send_message(msg)
    data = s.read_message()
    os.system('adb pull'+data)
    s.close()
