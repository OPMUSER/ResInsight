import grpc
import os
import sys
import socket
import logging

sys.path.insert(1, os.path.join(sys.path[0], '../generated'))

import RiaVersionInfo

from .App import App
from .Commands import Commands
from .Project import Project

class Instance:
    @staticmethod
    def is_port_in_use(port):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.settimeout(0.2)
            return s.connect_ex(('localhost', port)) == 0
    
    @staticmethod
    def launch(resInsightExecutable = ''):
        port = 50051
        portEnv = os.environ.get('RESINSIGHT_GRPC_PORT')
        if portEnv:
            port = int(portEnv)
        
        if not resInsightExecutable:
            resInsightExecutable = os.environ.get('RESINSIGHT_EXECUTABLE')
            if not resInsightExecutable:
                print('ERROR: Could not launch ResInsight because the environment variable'
                      ' RESINSIGHT_EXECUTABLE is not set')
                return None
        
        while Instance.is_port_in_use(port):
            port += 1

        print('Port ' + str(port))
        print('Trying to launch', resInsightExecutable)
        pid = os.spawnl(os.P_NOWAIT, resInsightExecutable, " --grpcserver " + str(port))
        print(pid)
        return Instance(port)
    
    @staticmethod
    def find(startPort = 50051, endPort = 50071):
        portEnv = os.environ.get('RESINSIGHT_GRPC_PORT')
        if portEnv:
            startPort = int(portEnv)		
            endPort   = startPort + 20 
        
        for tryPort in range(startPort, endPort):
            if Instance.is_port_in_use(tryPort):
                return Instance(tryPort)
                
        print('Error: Could not find any ResInsight instances responding between ports ' + str(startPort) + ' and ' + str(endPort))
        return None

    def __init__(self, port = 50051):
        logging.basicConfig()
        location = "localhost:" + str(port)
        self.channel = grpc.insecure_channel(location)

        # Main version check package
        self.app     = App(self.channel)
        try:
            majorVersionOk = self.app.majorVersion() == int(RiaVersionInfo.RESINSIGHT_MAJOR_VERSION)
            minorVersionOk = self.app.minorVersion() == int(RiaVersionInfo.RESINSIGHT_MINOR_VERSION)
            if not (majorVersionOk and minorVersionOk):
                raise Exception('Version of ResInsight does not match version of Python API')
        except grpc.RpcError as e:
            if e.code() == grpc.StatusCode.UNAVAILABLE:
                print('Info: Could not find any instances at port ' + str(port))
        except Exception as e:
            print('Error:', e)
                
        # Service packages
        self.commands   = Commands(self.channel)
        self.project    = Project(self.channel)
    