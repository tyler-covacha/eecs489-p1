#!/usr/bin/env python
"""
Measurement topology for EECS489, Winter 2024, Assignment 1
"""

from mininet.cli import CLI
from mininet.net import Mininet
from mininet.link import TCLink
from mininet.topo import Topo
from mininet.log import setLogLevel

class AssignmentNetworks(Topo):
    def __init__(self, **opts):
        Topo.__init__(self, **opts)
        # This part adds each individual host
        # and returns an object of each one
        h1 = self.addHost('h1')
        h2 = self.addHost('h2')
        h3 = self.addHost('h3')
        h4 = self.addHost('h4')
        h5 = self.addHost('h5')
        h6 = self.addHost('h6')
        h7 = self.addHost('h7')
        h8 = self.addHost('h8')

        # This part adds each switch in the network
        s1 = self.addSwitch('s1')
        s2 = self.addSwitch('s2')
        s3 = self.addSwitch('s3')
        s4 = self.addSwitch('s4')
        s5 = self.addSwitch('s5')

        # This part specifies the links from host to switch
        self.addLink(h1, s1)
        self.addLink(h2, s2)
        self.addLink(h8, s2)
        self.addLink(h3, s3)
        self.addLink(h7, s3)
        self.addLink(h4, s4)
        self.addLink(h5, s5)
        self.addLink(h6, s5)

        # This specifies the links between switches, with additional
        # information for the bandwidth and delay specified
        self.addLink(s1, s3, bw=50, delay='10ms')
        self.addLink(s2, s3, bw=20, delay='30ms')
        self.addLink(s3, s4, bw=30, delay='20ms')
        self.addLink(s4, s5, bw=25, delay='5ms')
        
        
if __name__ == '__main__':
    setLogLevel( 'info' )

    # Create data network
    topo = AssignmentNetworks()
    net = Mininet(topo=topo, link=TCLink, autoSetMacs=True,
           autoStaticArp=True)

    # Run network
    net.start()
    CLI( net )
    net.stop()

