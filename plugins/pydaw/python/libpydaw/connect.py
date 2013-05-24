# -*- coding: utf-8 -*-
"""
This file is part of the PyDAW project, Copyright PyDAW Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
"""

from commands import getoutput
import subprocess
from time import sleep
import unicodedata

def launch_jack_oscrolloscope():
    f_processes = getoutput("ps -ef")
    if not "pydaw_jack_oscrolloscope" in f_processes:
        subprocess.Popen(['/usr/lib/pydaw3/pydaw_jack_oscrolloscope', '-n', '2', '-x', '960', '-y', '360'])
        sleep(2)
    getoutput("jack_connect PyDAW:'PyDAW out_1' jack_oscrolloscope:in_1")
    getoutput("jack_connect PyDAW:'PyDAW out_2' jack_oscrolloscope:in_2")

class alsa_port:
    def __init__(self, a_client_number, a_client_name, a_client_type, a_port_number, a_port_name):
        self.client_number = sanitize_unicode_str(a_client_number)
        self.client_name = sanitize_unicode_str(a_client_name)
        self.client_type = sanitize_unicode_str(a_client_type)
        self.port_number = sanitize_unicode_str(a_port_number)
        self.port_name = sanitize_unicode_str(a_port_name)
        self.fqname = self.client_name + " - " + self.port_name + " ~ " + self.client_number + self.port_number


def sanitize_unicode_str(a_str):
    f_result = a_str
    try:
        f_result = unicodedata.normalize('', a_str).encode('ascii', 'ignore')
    except:
        pass
    return f_result.strip()

class alsa_ports:
    def parse_aconnect(self, a_string):
        arr = a_string.split("\n")
        f_result = []
        client_type = ""
        client_name = ""
        client_number = ""
        for line in arr:
            try:
                if line[:len("client")] == "client":
                    client_type = line.split("[")[1].split("]")[0]
                    client_name = line.split(":")[1].split("[")[0]
                    client_number = line.split(" ")[1]
                else:
                    port_name = line.split("'")[1]
                    port_number = line.strip().split(" ")[0]
                    if not "System" in client_name:
                        f_result.append(alsa_port(client_number, client_name, client_type, port_number, port_name))
            except:
                print("Error parsing '" + line + "', this device will not be available, please report this bug at https://pydaw.org/forum")
        return f_result

    def connect_to_pydaw(self, a_string):
        f_string = str(a_string)
        print("Attempting to connect ALSA port " + f_string + " to PyDAW...")
        print(getoutput("aconnect -x"))
        if f_string == "None":
            return
        for f_alsa_port in self.input_ports:
            if "PyDAW" in f_alsa_port.client_name:
                f_out_port = f_string.split("~")[1]
                f_cmd = "aconnect " + f_out_port + " " + str(f_alsa_port.client_number) + str(f_alsa_port.port_number)
                print(f_cmd)
                print(getoutput(f_cmd))
                break

    def get_input_fqnames(self):
        f_result = []
        for port in self.input_ports:
            f_result.append(port.fqname)
        return f_result

    def get_output_fqnames(self):
        f_result = ["None"]
        for port in self.output_ports:
            f_result.append(port.fqname)
        return f_result

    def connect_ports(a_port_out_name, a_port_in_name):
        #command will be aconnect [client#]:[port#] [client#]:[port#], like aconnect 23:0 44:1 , where the first is sender, and 2nd receiver
        pass

    def __init__(self):
        input_str = getoutput("aconnect -o")
        self.input_ports = self.parse_aconnect(input_str)
        output_str = getoutput("aconnect -i")
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