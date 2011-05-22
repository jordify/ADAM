#!/usr/local/bin/python2.7
# -*- coding: utf8 -*-
"""
File: ham.py
Author: Jorge Gómez
Last Modified: Sun May 22, 2011 at 16:45
Description:
  A python sketch of the functionality to be implemented in the HAM
  subsytem of ADAM. Don't tell Greg I've written a python version of
  this because he will kill me.
"""

import sys, math, time
import unittest, argparse, zmq

from zlib import crc32
from struct import pack,unpack

import topoParser

class HamNode(object):
  def __init__(self):
    self.neighbors = topology.getInitNeighborList()
    pass


class Topology(object):
  def __init__(self):
    pass

  def getInitNeighborList(self):
    pass


class Health(object):
  def __init__(self):
    pass


class Message(object):
  def __init__(self):
    pass


class Connection(object):
  def __init__(self, isServer=False, middleWare="zeroMQ"):
    self.mw = 1
    self.crc = False
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
        self.socket = self.ctx.socket(zmq.REP)
        self.socket.bind("tcp://*:54321")
        print "Serving on 54321"
      else:
        self.socket = self.ctx.socket(zmq.REQ)
        self.socket.connect("tcp://localhost:54321")
        print "Connected to 54321"
    else:
      pass

  def send(self, data):
    if self.mw == 1:
      if self.crc:
        data += pack('i', crc32(data))
      return self.socket.send(data)
    else:
      return 1

  def receive(self):
    if self.mw == 1:
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
      return ""

  def close(self):
    self.socket.close()
    self.ctx.term()

class Benchmark(object):
  def __init__(self):
    from timeit import default_timer as timer
    self.timer = timer
    self.msgSizes = [2**n for n in range(28)]
    self.latencies = []

  def runTests(self, mw, reps):
    for size in self.msgSizes:
      self.latencies.append(self.testLat(mw, size, reps))
    return self.latencies

  def testLat(self, mw, size, reps):
    msg = 'a'*size
    pipe = Connection(middleWare=mw)
    t1 = self.timer()
    for i in xrange(reps):
      pipe.send(msg)
      newMsg = pipe.receive()
      assert msg == newMsg
    t2 = self.timer()
    pipe.close()
    return 1000000*(t2-t1)/(2*reps) # Latencies in µSec

  def writeCSV(self):
    import csv
    csvWriter = csv.writer(open('results.csv', 'w'), delimiter=',')
    i = 0
    lines = []
    for size in self.msgSizes:
      csvWriter.writerow([size,self.latencies[i]])


def beBmServer(mw):
  pipe = Connection(middleWare=mw, isServer=True)
  print pipe.mw, pipe.crc
  for i in xrange(28*300): # Echo only a certain amount
    msg = pipe.receive()
    pipe.send(msg)
  pipe.close()


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
    print bm.runTests("zeroMQ", 300)
    bm.writeCSV()
  elif cli.server:
    beBmServer("zeroMQ")

if __name__=='__main__':
  print "ZermoMQ Version:", zmq.zmq_version()
  main()
