#!/usr/bin/python2.7

import sys, argparse, subprocess, time
from multiprocessing import Process
import zmq

class Monitor(object):
  '''Monitoring for addam nodes'''
  def __init__(self, verbose=False):
    self.verbose = verbose
    self.ctx = zmq.Context()
    self.logger = self.ctx.socket(zmq.PULL)
    self.logger.bind("tcp://*:54321")
    self.poller = zmq.Poller()
    self.poller.register(self.logger, zmq.POLLIN)
    self.pidDict = {}

  def monitor(self):
    print "Started monitoring"
    while True:
      socks = dict(self.poller.poll())
      if self.logger in socks and socks[self.logger] == zmq.POLLIN:
        message = self.logger.recv()
        if self.verbose:
          print time.time(), message
        if message.split()[1] == "Died":
          self.nodeDies(int(round(time.time()*1000)), message)
        if message.split()[1] == "pid":
          self.nodePID(int(round(time.time()*1000)), message)

  def nodeDies(self, time, message):
    self.pidDict[int(message.split()[0].split('[')[1][:-1])] = time
    if self.verbose:
      print self.pidDict

  def nodePID(self, time, message):
    key = int(message.split()[0].split('[')[1][:-1])
    if key in self.pidDict.keys():
      print time-self.pidDict[key]
    self.pidDict[key] = time
    if self.verbose:
      print self.pidDict

if __name__=='__main__':
  parser = argparse.ArgumentParser(description='''Launch an example \
      topology of addam nodes on the Iota cluster given the size of \
      the desired system.''')
  parser.add_argument('-v', action='store_true', default=False, \
      help='Be verbose')
  cli = parser.parse_args()

  mon = Monitor(cli.v)
  mon.monitor()
