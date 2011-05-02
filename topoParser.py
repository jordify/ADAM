#!/usr/bin/python2
"""
File: topoParser.py
Version: 0.1.0
Author: Jorge Gomez
Last Modified: Sun Apr 24, 2011 at 14:11

Description:
  This script will input a topology XML file and build the ADAM
  network it describes.
  Valid XML elements:
    topology: This is the parent element. Includes a name attribute.
    network: Contains a type and speed attribute. Wraps a network name.
    node: Has an id attribute. Contains the following elements:
      link: Attribute is network.
      cpu: Attributes are quantity and radHard.

To Do:
  Add visualization. For MAC check NodeBox.
"""

import sys
import xml.parsers.expat

class hamNode():
  def __init__(self, name):
    self.name = name
    self.links = []
    self.cpus = 1
    self.cpuRadHard = False

  def defCPUSet(self, quantity, radHard=False):
    self.cpus = quantity
    self.cpuRadHard = radHard

  def addLink(self, network):
    self.links.append(network)


class hamNetwork():
  def __init__(self, netType, speed):
    self.netType = netType
    self.speed = speed
    self.name = "Unnamed"

  def defName(self, name):
    self.name = name


class Parser():
  '''Class to parse the topology file.'''
  def __init__(self, filename):
    try:
      self.topoFile = open(filename, 'r')
    except:
      print 'Could not open topology file!'
      sys.exit(1)
    self.topoXML = self.topoFile.read()
    self.p = xml.parsers.expat.ParserCreate()
    self.p.StartElementHandler = self.start_element
    self.p.EndElementHandler = self.end_element
    self.p.CharacterDataHandler = self.char_data

    # HAM System
    self.topoName = ""
    self.nodeList = []
    self.networkList = []

  def start_element(self, name, attrs):
    if name=="topology":
      self.topoName=attrs.get("name")
    elif name=="network":
      self.networkList.append(hamNetwork(attrs.get("type"),int(attrs.get("speed"))))
    elif name=="node":
      self.nodeList.append(hamNode(attrs.get("id")))
    elif name=="link":
      self.nodeList[-1].addLink(attrs.get("network"))
    elif name=="cpu":
      self.nodeList[-1].defCPUSet(int(attrs.get("quantity")),(attrs.get("radHard")=="True"))

  def end_element(self, name):
    pass

  def char_data(self, data):
    possName = str(data).strip(' \t\n')
    if possName:
      self.networkList[-1].defName(possName)

  def parse(self):
    self.p.Parse(self.topoXML, 1)


if __name__=='__main__':
  testParser = Parser('iota.topo')
  testParser.parse()
  print "Topology of:", testParser.topoName
  print "\tNetworks: (%d total)" % (len(testParser.networkList))
  for network in testParser.networkList:
    print "\t\tNetwork Name:", network.name
    print "\t\tNetwork Type:", network.netType
    print "\t\tNetwork Speed:", network.speed
    print ""
  print "\tNodes: (%d total)" % (len(testParser.nodeList))
  for node in testParser.nodeList:
    print "\t\tNode Name:", node.name
    print "\t\tNode CPUs:", node.cpus
    print "\t\tNode Rad Hard?", node.cpuRadHard
    print "\t\tNode Links:", node.links
    print ""
