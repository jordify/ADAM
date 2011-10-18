#!/usr/local/bin/python2.7
# -*- coding: utf8 -*-
"""
File: ham.py
Author: Jorge Gómez
Last Modified: Mon May 23, 2011 at 05:12
Description:
  A python sketch of the functionality to be implemented in the HAM
  subsytem of ADAM. Don't tell Greg I've written a python version of
  this because he will kill me.
"""

import sys, time
import socket as sock
import unittest, argparse, signal

from struct import pack,unpack


class Connection(object):
  def __init__(self, isServer=False):
    self.isServer = isServer
    self.socket = sock.socket(sock.AF_INET, sock.SOCK_STREAM)
    host = ''; port = 54321
    if isServer:
      self.socket.bind((host,port))
      print "Listening on 54321"
      self.socket.listen(1)
      print "Serving"
      self.client, self.clientAddress = self.socket.accept()
      print "Accepted connection from:", self.clientAddress
    else:
      self.socket.connect((host,port))
    signal.signal(signal.SIGINT, self._sigHandler)

  def _sigHandler(self, signum, frame):#{{{
    """Shuts down the server gracefully on SIGINT"""
    print '\nCaught SIGINT, shutting down!'
    if self.isServer:
      self.client.close()
    self.socket.close()
    sys.exit(136)#}}}

  def send(self, data):
    if self.isServer:
      self.client.send(pack('i', len(data)))
      self.client.send(data)
    else:
      self.socket.send(pack('i', len(data)))
      self.socket.send(data)
    return 0

  def receive(self):
    if self.isServer:
      size = self.client.recv(4)
      if size:
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
      if size:
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
    if self.isServer:
      self.client.close()
    self.socket.close()


class Benchmark(object):
  def __init__(self):
    from timeit import default_timer as timer
    self.timer = timer
    self.msgSizes = [2**n for n in range(26)]
    self.latencies = []
    self.throughputs = []

  def runTests(self, reps):
    for size in self.msgSizes:
      self.latencies.append(self.testLat(mw, size, reps))
#       self.throughputs.append(self.testThr(mw, size, reps)) 
#     return self.throughputs 
    return self.latencies

  def testLat(self, size, reps):
    msg = 'a'*size
    pipe = Connection()
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
#       csvWriter.writerow([size,self.throughputs[i]]) 
      csvWriter.writerow([size,self.latencies[i]])
      i+=1


def beBmServer():
  while True:
    pipe = Connection(isServer=True)
    while True:
      msg = pipe.receive()
      if not msg:
        break
      print len(msg)
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
    pipe = Connection()
    pipe.send("hello"*1000000)
    print len(pipe.receive())
    pipe.close()
  elif cli.server:
    beBmServer()

if __name__=='__main__':
  main()
