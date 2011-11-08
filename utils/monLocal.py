#!/usr/bin/python2.7

import sys, argparse, subprocess, time
from multiprocessing import Process
import zmq

class System(object):
  '''System of addam nodes running in Iota'''
  def __init__(self, size, verbose=False):
    self.size = size
    self.verbose = verbose
    self.nodeNames = ["compute-0-%d" % (i,) for i in xrange(self.size)]
    self.PIDs = []
    if self.verbose:
      print self.nodeNames

  def launch(self):
    i = 0;
    for node in self.nodeNames:
      self.PIDs.append(subprocess.Popen(["ssh", node, "./ADAM/src/addamd", \
        "./ADAM/exampleTopos/iota%d.topo" % (self.size,), "%d" % (i,)], \
        ))
      i+=1

  def kill(self, nodeID):
    self.PIDs[nodeID].kill()

class Monitor(object):
  '''Monitoring for addam nodes'''
  def __init__(self, verbose=False):
    self.verbose = verbose
    self.ctx = zmq.Context()
    self.logger = self.ctx.socket(zmq.PULL)
    self.logger.bind("tcp://*:54321")
    self.poller = zmq.Poller()
    self.poller.register(self.logger, zmq.POLLIN)

  def monitor(self):
    print "Started monitoring"
    while True:
      socks = dict(self.poller.poll())
      if self.logger in socks and socks[self.logger] == zmq.POLLIN:
        message = self.logger.recv()
        if self.verbose:
          print message
        if message.split()[0] == "Kill":
          subprocess.Popen(['./addamd', '../exampleTopos/local3.topo', message.split()[1]])

if __name__=='__main__':
  parser = argparse.ArgumentParser(description='''Launch an example \
      topology of addam nodes on the Iota cluster given the size of \
      the desired system.''')
  parser.add_argument('-n', action='store', type=int, help='Total \
      number of nodes in the network (default=4)')
  parser.add_argument('-v', action='store_true', default=False, \
      help='Be verbose')
  parser.set_defaults(n=4)

  cli = parser.parse_args()

  mon = Monitor(cli.v)
  mon.monitor()
##  system = System(cli.n, cli.v)
##  system.launch()
##  if cli.v:
##    print "Launched"
##  time.sleep(2)
##  system.kill(0)
##  if cli.v:
##    print "Killed 0"
##  time.sleep(2)
##  system.kill(1)
##  if cli.v:
##    print "Killed 1"
##  time.sleep(2)
##  system.kill(2)
##  if cli.v:
##    print "Killed 2"
##  time.sleep(2)
##  system.kill(3)
##  if cli.v:
##    print "Killed 3"
##  time.sleep(3)
##  p.terminate()
