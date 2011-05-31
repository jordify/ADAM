#!/usr/bin/python2

import socket as sock
import select, sys, signal, time, subprocess

class Logger(object):
  def __init__(self, logFile='./output.log'): #{{{
    self.logFile = logFile

  def log(self, msg):
    lf = open(self.logFile, 'a')
    lf.write(msg+'\n')
    lf.close()
  #}}}


class JobManager(object):
  """Manages the tasks"""
  def __init__(self, hamObj, logging, jdfLoc='missionDemo.jdf'): #{{{
    # Do you want to log
    if logging:
      self.logEnabled = True
      self.logger = logging()
    # Build a class for this
    self.hamObj = hamObj
    self.timeOut = 0.20
    self.lastBeat = time.time()
    self.checkBeat = False
    # Get job list from jdf
    jdf = open('missionDemo.jdf', 'r')
    self.jobs = []
    self.waiting = {}
    self.done = {}
    jobCount = 0
    for line in jdf.readlines():
      self.jobs.append(line.split())
      jobCount += 1
    jdf.close()
    self.jobs = dict(zip(range(1,jobCount+1), self.jobs))

  def launchJob(self, soc, jobID): #{{{
    """Sends the right msg to the JMA to launch a job on its device."""
    jobMsg = 'Job'
    jobMsg += lenStr(jobID)
    argv = self.jobs[jobID]
    argc = len(argv)
    jobMsg += lenStr(argc)
    for i in xrange(argc):
      jobMsg += lenStr(len(argv[i]))
    for i in xrange(argc):
      jobMsg += argv[i]
    HAM._send(self.hamObj, soc, jobMsg)
    lb = time.time()
    time.sleep(.2)
    self.lastBeat += time.time() - lb
    # }}}

  def msgHandler(self, client, msg): #{{{
    msg = msg.split(',')
    if msg[0].find('HB')==0:
      self.lastBeat = time.time()
    elif msg[0].find('Connected')==0:
      self.checkBeat = True
      self.lastBeat = time.time()
      for jobID in self.jobs.keys():
        self.launchJob(client, jobID)
      #LOG
      logMsg = 'Connected to JMA Pid: '+msg[1]
      print logMsg
      if self.logEnabled:
        self.logger.log(logMsg)
    elif msg[0].find('Started')==0:
      jobID = int(msg[1])
      self.waiting[jobID] = self.jobs.pop(jobID)
#       if self.jobs: 
#         self.launchJob(client, self.jobs.keys()[0]) 
      #LOG
      logMsg = 'Started Job id: %s Pid: %s' % (msg[1], msg[2])
      print logMsg
      if self.logEnabled:
        self.logger.log(logMsg)
    elif msg[0].find('Failed')==0:
      jobID = int(msg[1])
      self.jobs[jobID] = self.waiting.pop(jobID)
#       if self.jobs: 
#         self.launchJob(client, self.jobs.keys()[0]) 
      #LOG
      logMsg = 'Job %s Failed' % msg[1]
      print logMsg
      if self.logEnabled:
        self.logger.log(logMsg)
    elif msg[0].find('Done')==0:
      jobID = int(msg[1])
      self.done[jobID] = self.waiting.pop(jobID)
      if self.jobs:
        self.launchJob(client, self.jobs.keys()[0])
      elif not(self.waiting):
        HAM._send(self.hamObj, client, 'Done')
        self.jobs = self.done.copy()
      #LOG
      logMsg = 'Finished Job %s' % msg[1]
      print logMsg
      if self.logEnabled:
        self.logger.log(logMsg)
    else:
      #LOG
      logMsg = 'Got Unknown: %s' % msg[0]
      print logMsg
      if self.logEnabled:
        self.logger.log(logMsg)
      #}}}

  def jmaDeathHandler(self):
    for jobID in self.waiting.keys():
      self.jobs[jobID] = self.waiting.pop(jobID)
    self.checkBeat = False
    #LOG
    print 'JMA died'
    if self.logEnabled:
      self.logger.log('JMA disconnected')

  def checkTime(self):
    if self.checkBeat and time.time()-self.lastBeat > self.timeOut:
      self.jmaDeathHandler()
      return True
    else:
      return False
  #}}}

class HAM(object):
  """Handles incoming connections from JMAs"""
  def __init__(self, JM, log=None, port=54322, clientQueue=2): #{{{
    self.myJM = JM(self, log)
    self.clients = [] # Needed?
    self.srv = sock.socket(sock.AF_INET, sock.SOCK_STREAM)
    host = ''
    self.srv.bind((host,port))
    print 'Listening on port: ', port
    self.srv.listen(clientQueue)
    signal.signal(signal.SIGINT, self._sigHandler)

  def _sigHandler(self, signum, frame):#{{{
    """Shuts down the server gracefully on SIGINT"""
    print '\nCaught SIGINT, shutting down!'
    for c in self.clients:
      c.close()
    self.srv.close()#}}}

  def _getS(self, soc):  #{{{
    """Get size of incoming message"""
    return(intStr(soc.recv(4)))  #}}}

  def _get(self, soc, size):#{{{
    """Get incoming message"""
    return soc.recv(size)#}}}

  def _send(self, soc, msg):#{{{
    """Send msg to client"""
    msg = lenStr(len(msg)) + msg
    try:
      sent = soc.send(msg)
      print 'Sent msg: ', msg[3:]
    except sock.error, e:
      print 'Send Failed!'#}}}

  def serve(self):
    inputs = [self.srv]
    self.clients = []
    quit = False
    while not quit:
      try:
        readersReady, writersReady, errs = select.select(inputs, self.clients, [])
      except select.error, e:
        break
      except sock.error, e:
        break
      except KeyboardInterrupt:
        self.srv.close()
        break
      for s in readersReady:
        if s == self.srv:
          client, addr = self.srv.accept()
          self.clients.append(client)
          inputs.append(client)
          print 'Connected to JMA at ', s.getsockname()
        else:
          try:
            size = self._getS(s)
            if size:
              msg = self._get(s,size)
              self.myJM.msgHandler(s, msg)
            else:
              print 'Disconnected from JMA at ', s.getsockname()
              self.myJM.jmaDeathHandler()
              s.close()
              self.clients.remove(s)
              inputs.remove(s)
          except sock.error, e:
            inputs.remove(s)
            s.close()
            self.clients.remove(s)
      if self.clients and self.myJM.checkTime():
            s.close()
            self.clients.remove(s)
      time.sleep(0.001)
  #}}}

def lenStr(size):#{{{
  """Return a little endian 32-bit string corresponding to the integer size"""
  sizeStr = ''
  for i in range(4):
    rem = size%(256)
    sizeStr += chr(rem)
    size /= 256
  return sizeStr#}}}

def intStr(msg):  #{{{
  """Returns the integer described by the input little endian 32-bit string"""
  size = 0
  for c in msg[::-1]:
    size *= 256
    size += ord(c)
  return size  #}}}

if __name__ == '__main__':
  myHAM = HAM(JobManager, Logger)
  myHAM.serve()
