# -*- coding: utf-8 -*-
"""
Methods for querying and connecting ALSA and Jack ports.

The ways things are done in this file are about as fundametally wrong as you 
could possibly do them.  The reason for this is that it takes a lot less time
to hack the output of CLI utilities than it does to decipher terrible 
documentation that lacks basic examples of how to do tasks fundamental
to use of the API.  


"""

from commands import getoutput

class alsa_port:
    def __init__(self, a_client_number, a_client_name, a_client_type, a_port_number, a_port_name):
        self.client_number = a_client_number.strip()
        self.client_name = a_client_name.strip()
        self.client_type = a_client_type.strip()
        self.port_number = a_port_number.strip()
        self.port_name = a_port_name.strip()
        self.fqname = self.client_name + " " + self.client_number + " " + self.port_name + " " + self.port_number
        
class alsa_ports:
    def parse_aconnect(self, a_string):
        arr = a_string.split("\n")
        f_result = []
        client_type = ""
        client_name = ""
        client_number = ""
        for line in arr:
            if line[:len("client")] == "client":
                client_type = line.split("[")[1].split("]")[0]
                client_name = line.split("'")[1]
                client_number = line.split(" ")[1]
            else:
                port_name = line.split("'")[1]
                port_number = line.strip().split(" ")[0]
                f_result.append(alsa_port(client_number, client_name, client_type, port_number, port_name))
        return f_result
        
    def get_input_fqnames(self):
        f_result = []
        for port in self.input_ports:
            f_result.append(port.fqname)
        return f_result
        
    def get_output_fqnames(self):
        f_result = []
        for port in self.output_ports:
            f_result.append(port.fqname)
        return f_result
        
    def connect_ports(a_port_out_name, a_port_in_name):
        #command will be aconnect [client#]:[port#] [client#]:[port#], like aconnect 23:0 44:1 , where the first is sender, and 2nd receiver     
        pass
        
    def __init__(self):
        input_str = getoutput("aconnect -i")        
        self.input_ports = self.parse_aconnect(input_str)        
        output_str = getoutput("aconnect -o")        
        self.output_ports = self.parse_aconnect(output_str)



class jack_ports:
    def __init__(self):
        jack_ports = getoutput("jack_lsp")

if __name__ == "__main__":
    this_alsa_ports = alsa_ports()
    for port in this_alsa_ports.input_ports:
        print(port.fqname)
    for port in this_alsa_ports.output_ports:
        print(port.fqname)