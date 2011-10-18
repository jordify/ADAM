#!/usr/local/bin/python2.7
# -*- coding: utf8 -*-
"""
File: ham.py
Author: Jorge Gómez
Last Modified: Fri Jun 03, 2011 at 18:50
Description:
  A python sketch of the functionality to be implemented in the HAM
  subsytem of ADAM.
"""

import sys, time
import socket as sock
import unittest, argparse, signal, zmq

from zlib import crc32
from struct import pack,unpack

import topoParser


class Connection(object):
  def __init__(self, isServer=False, middleWare="zeroMQ"):
    self.mw = 1
    self.crc = False
    self.isServer = isServer
    if middleWare == "tcp":
      self.mw = 0
    elif middleWare == "zeroMQ":
      self.mw = 1
    elif middleWare == "FTzeroMQ":
      self.mw = 1
      self.crc = True
    if self.mw == 1:
      self.ctx = zmq.Context()
      if isServer:
        self.socket = self.ctx.socket(zmq.SUB)
#         self.socket = self.ctx.socket(zmq.REP) 
        self.socket.bind("tcp://*:54321")
        print "Serving on 54321"
      else:
        self.socket = self.ctx.socket(zmq.PUB)
#         self.socket = self.ctx.socket(zmq.REQ) 
        self.socket.connect("tcp://iota.hcs.ufl.edu:54321")
        print "Connected to 54321"
    else:
      self.socket = sock.socket(sock.AF_INET, sock.SOCK_STREAM)
      host = ''; port = 54321
      if isServer:
        self.socket.bind((host,port))
        print "Serving on 54321"
        self.socket.listen(1)
        self.client, self.clientAddress = self.socket.accept()
      else:
        self.socket.connect(("iota.hcs.ufl.edu",port))
        print "Connected to 54321"
    signal.signal(signal.SIGINT, self._sigHandler)

  def _sigHandler(self, signum, frame):#{{{
    """Shuts down the server gracefully on SIGINT"""
    print '\nCaught SIGINT, shutting down!'
    try:
      self.client.close()
    except:
      pass
    try:
      self.socket.close()
    except:
      pass
    try:
      self.ctx.term()
    except:
      pass
    sys.exit(136)#}}}

  def send(self, data):
    if self.mw == 1:
    # ZMQ Send
      if self.crc:
        data += pack('i', crc32(data))
      return self.socket.send(data)
    else:
    # TCP Send
      if self.isServer:
        self.client.send(pack('i', len(data)))
        self.client.send(data)
      else:
        self.socket.send(pack('i', len(data)))
        self.socket.send(data)
      return 0

  def receive(self):
    if self.mw == 1:
    # ZMQ Receive
      data = self.socket.recv()
      if self.crc:
        if len(data) < 4:
          print "ERROR: Data received is not crc'd."
        else:
          checksum = unpack('i', data[-4:])[0]
          if crc32(data[:-4]) == checksum:
            return data[:-4]
          else:
            print "Checksum failed"
            return ""
      else:
        return data
    else:
    # TCP receive
      if self.isServer:
        size = self.client.recv(4)
        if len(size) < 4 and len(size) >0:
          msg = self.client.recv(4096) # WHAT IS THISSSS
          return None
        elif len(size) == 4:
          size = unpack('i', size)[0]
          recvSize = 0
          msg = ""
          while recvSize < size:
            msg += self.client.recv(4096)
            recvSize += 4096
          return msg
        else:
          return None
      else:
        size = self.socket.recv(4)
        if len(size) == 4:
          size = unpack('i', size)[0]
          recvSize = 0
          msg = ""
          while recvSize < size:
            msg += self.socket.recv(4096)
            recvSize += 4096
          return msg
        else:
          return None

  def close(self):
    try:
      self.client.close()
    except:
      pass
    try:
      self.socket.close()
    except:
      pass
    try:
      self.ctx.term()
    except:
      pass


class Benchmark(object):
  def __init__(self):
    from timeit import default_timer as timer
    self.timer = timer
    self.msgSizes = [2**n for n in range(26)]
    self.latencies = []
    self.throughputs = []

  def runTests(self, mw, reps):
    for size in self.msgSizes:
#       time.sleep(0.5) 
#       self.latencies.append(self.testLat(mw, size, reps)) 
      self.throughputs.append(self.testThr(mw, size, reps))
    return self.throughputs
#     return self.latencies 

  def testLat(self, mw, size, reps):
    msg = 'a'*size
    pipe = Connection(middleWare=mw)
    t1 = self.timer()
    for i in xrange(reps):
      pipe.send(msg)
      newMsg = pipe.receive()
      if not msg == newMsg:
        print len(msg), len(newMsg)
        continue
    t2 = self.timer()
    pipe.close()
    return 1000000*(t2-t1)/(2*reps) # Latencies in µSec

  def testThr(self, mw, size, reps):
    msg = 'a'*size
    pipe = Connection(middleWare=mw)
    t1 = self.timer()
    for i in xrange(reps):
      pipe.send(msg)
    t2 = self.timer()
    pipe.close()
    print size*reps*8/(1000000*(t2-t1)) # Throughput in Mb/s
    return size*reps*8/(1000000*(t2-t1)) # Throughput in Mb/s

  def writeCSV(self):
    import csv
    csvWriter = csv.writer(open('zThr_26_30.csv', 'w'), delimiter=',')
    i = 0
    lines = []
    for size in self.msgSizes:
      csvWriter.writerow([size,self.throughputs[i]])
#       csvWriter.writerow([size,self.latencies[i]]) 
      i+=1


def beBmServer(mw):
  while True:
    pipe = Connection(middleWare=mw, isServer=True)
    while True:
      msg = pipe.receive()
      if not msg:
        break
#       pipe.send(msg) 
    pipe.close()
#     time.sleep(1) 


def main():
  parser = argparse.ArgumentParser(description='''Communications \
      subsytem of the ADAM system. Library should be inherited not \
      called directly. This is here for testing and development \
      purposes.''')
  parser.add_argument('--test', action='store_true', default=False, \
      help='run the unit tests')
  parser.add_argument('--time', action='store_true', default=False, \
      help='run the benchmark suite')
  parser.add_argument('--server', action='store_true', default=False, \
      help='Be a server for the benchmark suite')

  cli = parser.parse_args() # Get the command line arguments
  if cli.test:
    sys.argv.remove('--test')
    unittest.main()
  elif cli.time:
    bm = Benchmark()
    print bm.runTests("FTzeroMQ", 30)
    bm.writeCSV()
  elif cli.server:
    beBmServer("FTzeroMQ")

if __name__=='__main__':
  print "ZermoMQ Version:", zmq.zmq_version()
  main()
