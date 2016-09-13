from __future__ import division

"""
encube_m.py

Copyright (c) 2015-2016 Dany Vohl, David G. Barnes, Christopher J. Fluke,
                   Yuri Benovitski, Tsz Ho Wong, Owen L Kaluza, Toan D. Nguyen.

This file is part of encube.

encube is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

encube is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with encube.  If not, see <http://www.gnu.org/licenses/>.

We would appreciate it if research outcomes using encube would
provide the following acknowledgement:

"Visual analytics of multidimensional data was conducted with encube."

and a reference to

Dany Vohl, David G. Barnes, Christopher J. Fluke, Govinda Poudel, Nellie Georgiou-Karistianis,
Amr H. Hassan, Yuri Benovitski, Tsz Ho Wong, Owen L Kaluza, Toan D. Nguyen, C. Paul Bonnington. (2016).
Large-scale comparative visualisation of sets of multidimensional data. PeerJ Computer Science, In Press.
"""

#!/usr/bin/python
import sys
from os import fork, system, path
from time import sleep
from pandas import DataFrame, read_csv, to_datetime
import json
import socket
import numpy as np
import threading
import cPickle as pickle
from uuid import uuid4
from os import environ as ENV
import subprocess
import copy
from binascii import a2b_base64

# Define false and true (as sent by javascript)
false = False
true = True

'''
(c) Dany Vohl, 2015-2016.
'''

class EncubeMResource:
    def __init__(self, config_file):

        self.config_file = config_file

    def __enter__(self):
        class EncubeM:
            def __init__(self, config_file):
                """
                Initialises the EncubeM object parameters

                :param config_file: path/to/config.json
                :return: nothing.
                """

                self.start_only = False

                # self.config
                try:
                    with open(config_file) as json_data_file:
                        self.config = json.load(json_data_file)
                        self.config_file_abs = path.abspath(config_file)
                except:
                    print "Cannot load configuration file " + config_file + " . Verify that the file is available. Aborting."
                    print "Usage:"
                    print ""
                    print "python webserver.py path/to/config_file.json"
                    sys.exit(0)

                # self.data
                try:
                    self.data = read_csv(self.config["input_csv"], sep=',')
                except:
                    print "Could not open encube file '" + self.config["input_csv"] + "'"
                # Store data's column names
                #  ( can be used for: sort by column name, get column name, ...)
                self.columns = self.data.columns

                # Communication variables
                self.master_pipe = None
                self.telnets = []
                self.sockets = []

                # Nodes/slaves ports
                self.nodes_indexes = {}
                self.slave_ports = []
                for i in range(len(self.config["slaves"])):
                    self.slave_ports.append(self.config["slave_port_base"] + i)
                    self.nodes_indexes[self.config["slaves"][i]] = i

                # The use of slaves_txt is deprecated. Will be removed.
                self.slaves_txt = []
                for i in range(len(self.config["slaves"])):
                    # The use of _i seems unused and unnecessary.
                    self.slaves_txt.append(self.config["slaves"][i] + ".txt")

                # Workflow documentation
                if self.config["save_workflow"]:
                    self.json_filename = "session_" + str(uuid4()) + '.json'
                    self.history = {}
                    self.history["last_id"] = 0
                    self.history["records"] = {}
                    try:
                        #self.database = sqlite3.connect(self.config["base_dir"] + '/session_data/encube.db')
                        self.database = read_csv(self.config["db"], sep=',')
                        # self.history = pickle.load(open(str(self.database["path_to_workflow"][0]) +
                        #                                 str(self.database["filename"][0]), "rb"))
                    except:
                        print "Database currently empty."
                        self.database = None

                # System's state dictionary.
                # Keeps track of the overall nodes and panels states
                #  (Can be used for : history, reload state, ...)
                self.status = 0
                self.state = {}
                self.init_system_state()


            def init_system_state(self):
                """
                Initialises the system state: nodes and related panels.

                :return: nothing.
                """
                # App_type
                self.state['app_type'] = self.config['app_type']

                # Database columns information
                self.state['columns_to_ignore'] = self.config['columns_to_ignore']

                # System's state
                self.state['status'] = self.status
                self.state['n_nodes'] = len(self.config["slaves"])
                self.state['n_per_node'] = self.config["n_per_slave"]
                self.state['save_workflow'] = self.config['save_workflow']

                # shareVol
                self.state['data'] = {}
                self.state['data']['sharevol'] = {}
                with open(self.config['shareVol_json']) as data_file:
                    share_vol_data = json.load(data_file)

                self.state['data']['sharevol']['url'] = share_vol_data['url']
                self.state['data']['sharevol']['res'] = share_vol_data['res']
                self.state['data']['sharevol']['scale'] = share_vol_data['scale']
                self.state['data']['sharevol']['volume'] = share_vol_data['volume']
                self.state['data']['sharevol']['slicer'] = share_vol_data['slicer']

                # IMAGE-HD
                if (self.config['app_type'] != "astro_fits"):
                    self.state['data']['imagehd'] = {}
                    self.state['data']['imagehd']['labels'] = False
                    self.state['data']['imagehd']['rotate'] = True         # Rotate (true) or translate (false)
                    # brain imagery related
                    self.state['data']['imagehd']['trackSamples'] = 0.001  # Track values are hard coded in control/main.js
                    self.state['data']['imagehd']['trackThickness'] = 2    # Track values are hard coded in control/main.js
                    self.state['data']['imagehd']['trackOpacity'] = 0.5    # Track values are hard coded in control/main.js
                else:
                    self.state['data']['astro'] = {}
                    self.state['data']['astro']['show'] = False
                    self.state['data']['astro']['moment'] = 0

                # Worflow history tracker (quick'n'dirty)
                # TODO: a better way would be to send id and action only through json in response.
                #       which will require a bit more development and restructuration of code
                #       both in encube_m.py, request_handle.py, and minicave.js
                if self.config["save_workflow"]:
                    self.state['workflow_data_dir'] = self.config['workflow_data_dir'].split('control')[1]
                    self.state['json_filename'] = self.json_filename

                # meta-viewer
                self.state['nodes'] = []
                for slave in self.config['slaves']:
                    self.init_node(slave, share_vol_data)

            def init_node(self, node_id, share_vol_data):
                """
                Initialises the node state.
                 Note : node and slave are used as synonyms.

                :param node_id: identifier coming from config file 'slaves'
                :param share_vol_data: shareVol information to initialise the state from config file
                :return: nothing.
                """
                node = {}
                node['node_id'] = node_id
                node['panels']=[]
                for panel_id in range(self.config['n_per_slave']):
                    panel = self.init_panel(panel_id, share_vol_data)
                    node['panels'].append(panel)

                self.state['nodes'].append(node)

            def init_panel(self, panel_id, share_vol_data):
                """
                Initialises the panel state.

                :param panel_id: identifier within the node it is part of.
                :param share_vol_data: shareVol information to initialise the state from config file
                :return: dictionary representing the panel
                """
                panel = {}

                # Panel_id not necessarily useful for now as it's a counter
                #   I include it for consistency in the design.
                #   (e.g. system > node_id > panel_id)
                panel['panel_id'] = panel_id

                # Loaded data
                panel['file_id'] = -1

                # Is panel selected for highlight / control
                panel['selection_state'] = 0

                # Individual shareVol control (when panel is selected)
                # not yet used
                # panel['url'] = share_vol_data['url']
                # panel['res'] = share_vol_data['res']
                # panel['scale'] = share_vol_data['scale']
                # panel['volume'] = share_vol_data['volume']
                # panel['slicer'] = share_vol_data['slicer']

                return panel

            def get_json_from_system_state(self):
                json_response = json.dumps(self.state)
                return json_response

            def autoincrement_last_id(self):
                id = self.history["last_id"]
                self.history["last_id"] = self.history["last_id"] + 1
                return id

            def save_uri_to_png(self, filename, uri):
                '''
                Takes the address of an image from the browser
                and saves it into an image on the server
                :param filename: name of the file, without extension
                :param uri: uri received from client
                :return: nothing.
                '''
                head, data = uri.split(',')
                extension = head.split('data:image/')[1].split(';')[0]
                with open(self.config['base_dir'] + '/' + filename + '.' + extension, 'wb') as f:
                    f.write(a2b_base64(data))

            def update_state(self, descriptor, request):
                """
                Updates the state array given an action.

                :param descriptor: name of the triggered action by controller
                :param request: json string sent by controller
                :return: nothing.
                """

                # Update global
                if 'url' in request:
                    self.state['data']['sharevol']['url'] = request['url']
                if 'res' in request:
                    self.state['data']['sharevol']['res'] = request['res']
                if 'scale' in request:
                    self.state['data']['sharevol']['scale'] = request['scale']
                if 'volume' in request:
                    self.state['data']['sharevol']['volume'] = request['volume']
                if 'slicer' in request:
                    self.state['data']['sharevol']['slicer'] = request['slicer']
                if 'labels' in request:
                    self.state['data']['imagehd']['labels'] = request['labels']
                if 'rotate' in request:
                    self.state['data']['imagehd']['rotate'] = request['rotate']
                if 'trackSamples' in request:
                    self.state['data']['imagehd']['trackSamples'] = request['trackSamples']
                if 'trackThickness' in request:
                    self.state['data']['imagehd']['trackThickness'] = request['trackThickness']
                if 'trackOpacity' in request:
                    self.state['data']['imagehd']['trackOpacity'] = request['trackOpacity']
                if 'show' in request:
                    self.state['data']['astro']['show'] = request['show']
                if 'moment' in request:
                    self.state['data']['astro']['moment'] = request['moment']

                # Update meta-view
                if 'reorder' in descriptor:
                    for line in request:
                        self.state['nodes'][self.nodes_indexes[line['node']]]['panels'][int(line['panel'])]['file_id'] = line['id']

                if 'loadOne' in descriptor:
                    self.state['nodes'][self.nodes_indexes[request[0]['node']]]['panels'][int(request[0]['panel'])]['file_id'] = str(request[0]['id'])

                if 'update_selected' in descriptor:
                    self.state['nodes'][self.nodes_indexes[request[0]['node']]]['panels'][int(request[0]['panel'])]['selection_state'] = str(request[0]['selected'])


            def save_to_history(self, descriptor, request, requires_json_back):
                """
                Save a request from controller to workflow history. This workflow history will be
                saved to disk at the end of a session.

                :param descriptor: name of the triggered action by controller
                :param request: json string sent by controller
                :return: nothing.
                """
                self.history["records"][self.autoincrement_last_id()] = {"descriptor": descriptor,
                                                                         "request": request,
                                                                         "requires_json_back": requires_json_back,
                                                                         "state": copy.deepcopy(self.state)}

                #self.state['workflow_history'] = self.history
                # Save into json file (will likely need some optimization at some point
                # Keeping it simple for now.
                with open(self.config["workflow_data_dir"] + self.json_filename, 'w+') as f:
                    json.dump(self.history, f)

            def load_history(self):
                pass

            def receive(self, socket):
                data = socket.recv(4096)
                print 'Received', repr(data)

            def connect_to_encube_pr(self):
                try:
                    for idx in range(len(self.config["slaves"])):
                        #self.telnets.append(Telnet(str(self.config["slaves[idx]), str(self.slave_ports[idx])))
                        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                        print self.config["slaves"][idx], self.slave_ports[idx]
                        s.connect((self.config["slaves"][idx], self.slave_ports[idx]))
                        self.sockets.append(s)

                    self.status = 1

                except:
                    print "Cannot connect to encube_pr."
                    print "Usage:"
                    print ""
                    print "python encube_m.py path/to/config_file.json with_master"
                    self.status = 0
                    #Do something here...?

            def connect_to_master(self):
                try:
                    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    s.connect((self.config["master"], self.config["master_port"]))
                    self.sockets.append(s)

                    self.status = 1

                except:
                    print "Cannot connect to Master."
                    print "Usage:"
                    print ""
                    print "python encube_m.py path/to/config_file.json with_master"
                    self.status = 0
                    #Do something here...?

            def tidy_up(self):
                try:
                    print "Cleaning temporary txt files"
                    system("rm master_0.txt")
                    for filename in self.slaves_txt:
                        system("rm " + filename)
                except:
                    print "All clean."

            def send_to_all_slaves(self, command):
                """
                Send a command to all available nodes (slaves)

                :param command: command to be sent to slaves.
                :return: nothing.
                """

                """
                for node_telnet in self.telnets:
                    print "sending slave ..."
                    node_telnet.write(command + '\n') # for use with Telnet
                """

                try:
                    if self.status:
                        threads = []
                        for node_socket in self.sockets:
                            t = threading.Thread(target=self.send, args=(node_socket, command))
                            threads.append(t)
                            t.start()

                        for t in threads:
                            t.join()

                except:
                    print "Could not send to all slaves"
                    self.status = 0

            def send(self, node_socket, command):
                """
                Send command to node_socket (called from thread)

                :param node_socket: opened socket to node
                :param command: command to be sent to node through socket
                :return: nothing.
                """
                try:
                    node_socket.send(command + "\n") # for use with Telnet
                    node_socket.recv(1024)
                except:
                    print "Could not send to node"
                    self.status = 0

            def send_to_slave_by_node_id(self, node_id, command):
                """
                Send command to a node (slave) given its id.

                :param node_id: node's id.
                :param command: command to be sent to node.
                :return: nothing.
                """
                try:
                    if self.status:
                        print "by_node_id:", node_id
                        node_socket = self.sockets[node_id]
                        node_socket.send(command + "\n")
                        node_socket.recv(1024)
                except:
                    print "Could not send to slave", str(node_id)
                    self.status = 0

            def send_to_slave_by_node_name(self, node_name, command):
                """
                Send command to a node (slave) given its name.

                :param node_name: node's name.
                :param command: command to be sent to node.
                :return:
                """
                try:
                    if self.status:
                        node_socket = self.sockets[self.config["slaves"].index(node_name)]
                        node_socket.send(command + "\n")
                        node_socket.recv(1024)
                except:
                    print "Could not send to slave", node_name
                    self.status = 0

            def send_to_master(self, command, master):
                """
                Send command to master window.

                :param command: command to be sent.
                :param master: master socket.
                :return: nothing.
                """
                try:
                    if self.status:
                        print "sending master ..."
                        #master.write(data + '\n') # for use with Telnet
                        master.send(command + "\n")
                except:
                    self.status = 0

            def request_to_slave(self, node, command):
                """
                Request information to a node (e.g. Histogram)

                :param command: command to be sent to node.
                :return: json string comprising the requested information if it went well. False if not.
                """
                try:
                    if self.status:
                        node_socket = self.sockets[self.config["slaves"].index(node)]
                        node_socket.send(command + "\n")

                        if command.split()[0] == "H":
                            bufsize = 4000
                            response = node_socket.recv(bufsize)

                            #print response

                            data = response.split('#')[:-1]
                            #print data

                            nbins = data[0]
                            dmin = data[1]
                            dmax = data[2]
                            mean = data[3]
                            stand_dev = data[4]
                            min_filter = data[5]
                            max_filter = data[6]
                            counts = data[7:]
                            del data

                            #print "Min: ", np.min(np.float(counts)), "Max:", np.max(np.float(counts))
                            f_counts = np.float32(counts)
                            panel = command.split()[1]

                            incr = (np.float(dmax)-np.float(dmin))/np.int(nbins)
                            bin = np.float(dmin)
                            first = True

                            """
                            # This would be cleaner - but I haven't finished it -- currently makes webpage crash...!

                            hist_json = {}
                            hist_json['name']= node
                            hist_json["panel"] = int(panel)+1
                            hist_json["nbins"]= nbins
                            hist_json["dmin"]= dmin
                            hist_json["dmax"]= dmax
                            hist_json["incr"]= incr
                            hist_json["data"]=[]
                            for count in counts:
                                hist_json["data"].append({"bin":bin, "count":count})
                                bin += incr
                            """

                            hist_json = '{"node":"' + node + '", "panel":' + str(int(panel)+1) + \
                                        ', "nbins":'+ nbins +', "dmin":'+ dmin +', "dmax":'+ dmax +\
                                        ', "mean":' + mean + ', "stand_dev": ' + stand_dev + \
                                        ', "min_filter":' + min_filter + ', "max_filter": ' + max_filter + \
                                        ', "incr":'+ str(incr) +', "data":['

                            i = 0

                            with open('histogram.csv','w+') as f:
                                # Add header line
                                f.write('bin,count\n')

                                for count in counts:
                                    if first:
                                        hist_json += '{"bin":' + str(bin) + ',"count":'+ count +'}'
                                        first=False
                                    else:
                                        hist_json += ',{"bin":' + str(bin) + ',"count":'+ count +'}'

                                    f.write(str(bin) + ',' + count + '\n')

                                    print i, bin, count
                                    i += 1
                                    bin += incr
                                print ""
                                hist_json += ']}'

                            json_response = json.dumps(hist_json)


                            #print json_response

                            return json_response
                        elif command.split()[0] == "M":
                            bufsize = 40000
                            print "before x"
                            x = node_socket.recv(bufsize)

                            if not x:
                                print "what?"
                            else:
                                print x
                            print "before y"
                            y = node_socket.recv(bufsize)

                            print y
                            print "before data"

                            data = node_socket.recv(4*int(x)*int(y))

                            print data

                            # while True:
                            #     data = node_socket.recv(bufsize)
                            #     print data
                            #     if not data:
                            #         break


                            # mom1 = node_socket.recv(bufsize)
                            # mom2 = node_socket.recv(bufsize)

                            json_response = json.dumps({'x':x, 'y':y})#, 'mom0':mom0, 'mom1':mom1, 'mom2':mom2})
                            return json_response
                except:
                    self.status = 0

                return False

            def request_to_all_slaves(self, command):
                """
                Request information to all node (e.g. State)

                :param command: command to be sent to node.
                :return: json string comprising the requested information if it went well. False if not.
                """
                try:
                    if self.status:
                        if command.split()[0] == "S":
                            bufsize = 1024

                            node = 0
                            for node_socket in self.sockets:
                                node_socket.send(command + "\n")
                                response = node_socket.recv(bufsize)

                                # S2PLOT returns state in the form of "id#selection_state#id#selection_state#..."
                                data = response.split('#')[:-1]

                                #node = self.config["slaves"][i]

                                print data

                                panel = 0
                                for j in range(0,len(data),2):
                                    self.state['nodes'][node]['panels'][panel]['file_id'] = data[j]
                                    self.state['nodes'][node]['panels'][panel]['selection_state'] = data[j+1]
                                    panel += 1

                                node += 1

                            json_response = json.dumps(self.state)
                            return json_response

                        if command.split()[0] == "T":
                            bufsize = 1024
                            json_response = {}
                            json_response['labels'] = []
                            json_response['colours'] = []

                            with open('ntracks.csv','w+') as f:
                                # Add header line
                                f.write('Subject_ID,Age (years),Number of tracks,Type\n')

                                node = 0
                                for node_socket in self.sockets:
                                    node_socket.send(command + "\n")
                                    response = node_socket.recv(bufsize)


                                    print "response", response

                                    # S2PLOT returns state in the form of "id#ntracks#id#ntracks#..."
                                    data = response.split('#')[:-1]

                                    # node = self.config["slaves"][i]
                                    panel = 0
                                    for j in range(0, len(data), 2):
                                        current_id = data[j]
                                        if current_id != '-1':
                                            current_ntracks = data[j + 1]
                                            try:
                                                # DV: I changed data_dir to Subject_ID in csv.
                                                # It generalizes better than Subject_ID.
                                                # Lookup should be dynamic... Not sure how yet.
                                                current_age = self.data[self.data.Subject_ID == int(current_id)].Age
                                                current_age = current_age.iloc[0]
                                                current_type = self.data[self.data.Subject_ID == int(current_id)].Type
                                                current_type = current_type.iloc[0]

                                                if current_type not in json_response['labels']:
                                                    json_response['labels'].append(current_type)
                                                    json_response['colours'].append(self.config["colours"][current_type])

                                            except:
                                                print "Age or Type value not retrieved properly"

                                            print current_id, current_ntracks, current_age, current_type

                                            current_string = str(current_id) + ',' + \
                                                             str(current_age) + ',' + \
                                                             str(current_ntracks) + ',' + \
                                                             str(current_type) + '\n'

                                            print current_string

                                            f.write(current_string)
                                        panel += 1

                                    node += 1

                            json_response = json.dumps(json_response)
                            print json_response

                            return json_response

                        # if command.split()[0] == "H":
                        #     """
                        #         Requests an histogram to all columns and all screens
                        #         and pool the answers into a csv file. The histogram
                        #         will be created on the client.
                        #     """
                        #     bufsize = 4000
                        #
                        #     node = 0
                        #     histograms_data = []
                        #
                        #     for node_socket in self.sockets:
                        #         for screen in range(int(self.config['n_per_slave'])):
                        #             node_socket.send(command + "\n")
                        #             response = node_socket.recv(bufsize)
                        #
                        #             #print response
                        #
                        #             data = response.split('#')[:-1]
                        #             #print data
                        #
                        #             nbins = data[0]
                        #             dmin = data[1]
                        #             dmax = data[2]
                        #             counts = data[3:]
                        #             del data
                        #
                        #             panel = command.split()[1]
                        #
                        #             incr = (np.float(dmax)-np.float(dmin))/np.int(nbins)
                        #
                        #             bin = np.float(dmin)
                        #
                        #             hist_json = '{"node":"' + node + '", "panel":' + str(int(panel)+1) + \
                        #                         ', "nbins":'+ nbins +', "dmin":'+ dmin +', "dmax":'+ dmax \
                        #                         +', "incr":'+ str(incr) +', "data":['
                        #
                        #             i = 0
                        #
                        #             for count in counts:
                        #                 if first:
                        #                     hist_json += '{"bin":' + str(bin) + ',"count":'+ count +'}'
                        #                     first=False
                        #                 else:
                        #                     hist_json += ',{"bin":' + str(bin) + ',"count":'+ count +'}'
                        #
                        #                 print i, bin, count
                        #                 i += 1
                        #                 bin += incr
                        #             print ""
                        #             hist_json += ']}'
                        #
                        #             histograms_data.append({"name": str(node) + "-" + str(int(panel)+1),
                        #                                     "counts":counts,
                        #                                     "dmin": dmin,
                        #                                     "dmax":dmax,
                        #                                     "incr":incr})
                        #
                        #     with open('histograms.csv','w+') as f:
                        #         f.write('bin')
                        #         # Add header line
                        #         for rec in histograms_data:
                        #             f.write(',' + rec["name"])
                        #         f.write('\n')
                        #
                        #         # Add data
                        #         for rec in histograms_data:
                        #
                        #             f.write(',' + rec["name"])
                        #
                        #
                        #     #print json_response
                        #
                        # return json_response

                    return False
                except:
                    self.status = 0

                return False

            def sort(self, sort_cols):
                """
                Sorts data relative to sort_cols (an array of columns)
                sends data to nodes as it goes along.

                :param sort_cols: Column(s) on which to sort, either indexes or names.
                :return: nothing.
                """
                try: # If sort_cols is an array of strings (e.g. ['Age', 'Type', ..., 'STAP'])
                    self.data = self.data.sort(sort_cols)
                except:
                    # If sort_cols is an array of numerical index
                    # Converts these index into an array of strings with proper column names.
                    columns = [self.data.columns[int(sort_cols[i])] for i in range(len(sort_cols))]
                    self.data = self.data.sort(columns)

                self.data = self.data.reset_index(drop=True)

                idx = 0
                for node in self.config["slaves"]:
                    for j in range(self.config["n_per_slave"]):
                        # l : new function that replace KL.
                        # Command in the form of "l 0 22 c MeanImage.xrv"
                        command = 'l ' + str(j) + ' ' + \
                                  str(self.data['Subject_ID'][idx]) + ' ' + \
                                  str(self.data['Type'][idx][0]) + ' ' + \
                                  "MeanImage.xrw"

                        print "send_to_slave_by_node_id", node, command
                        self.send_to_slave_by_node_name(node, command)
                        idx = (idx + 1) % (len(self.data)+1)

            def sort_and_rewrite(self, sort_cols):
                """
                Sorts data relative to sort_cols (an array of columns)
                Re-writes all files about panel data for all nodes
                Needs to call send_to_all_slaves("KL") to work.
                - Will be deprecated soon.

                :param sort_cols: Column(s) on which to sort.
                :return: nothing.
                """

                # Sort on sort_cols
                try: # If sort_cols is an array of strings (e.g. ['Age', 'Type', ..., 'STAP'])
                    self.data = self.data.sort(sort_cols)
                except:
                    # If sort_cols is an array of numerical index
                    # Converts these index into an array of strings with proper column names.
                    columns = [self.data.columns[sort_cols[i]] for i in range(len(sort_cols))]
                    self.data = self.data.sort(columns)

                self.data = self.data.reset_index(drop=True)

                idx = 0
                i = 0
                j = 0

                with open(self.config["data_dir"] + "/master_0.txt", 'w') as f:
                    print "writing to master_0.txt"
                    for j in range(self.config["n_per_slave"]):
                        # HARDCODE: subject id is in second column
                        f.write(str(self.data['Subject_ID'][idx]))

                        # HARDCODE: subject type in first column
                        # HARDCODE: subject type has unique first letter
                        f.write("," + str(self.data['#Type'][idx][0]) + "\n")
                        # if more panels than data, data will be repeated
                        idx = (idx + 1) % (len(self.data)+1)

                    for j in range(self.config["n_per_slave"]):
                        f.write("MeanImage.xrw\n")

                    idx = 0
                    i = 0
                    j = 0

                    print "writing to ", self.slaves_txt[i]
                    with open(self.config["data_dir"] + "/" + self.slaves_txt[i], 'w') as f:
                        for j in range(self.config["n_per_slave"]):
                            # HARDCODE: subject id is in second column
                            f.write(str(self.data['Subject_ID'][idx]))

                            # HARDCODE: subject type in first column
                            # HARDCODE: subject type has unique first letter
                            f.write("," + str(self.data['#Type'][idx][0]) + "\n")
                            # if more panels than data, data will be repeated
                            idx = (idx + 1) % (len(self.data)+1)

                        for j in range(self.config["n_per_slave"]):
                            f.write("MeanImage.xrw\n")

            def rewrite_and_send_to_specific(self, new_order) :
                """
                Unloads data on affected nodes,
                Re-write file for those affected nodes
                Loads data back on affected nodes
                DEPRECATED.

                :param new_order: List of nodes and list of tuples about what to display on each screen (id,first letter).
                :return: nothing.
                """
                for node_and_data in new_order:
                    node = node_and_data[0]
                    data = node_and_data[1]

                    #node_telnet = self.telnets[fidx]
                    node_sockets = self.sockets[self.config["slaves"].index(node)]

                    # Unloads data on this node
                    #node_telnet.write("KU" + "\n")
                    node_sockets.send("KU" + "\n")

                    # Re-write file relative to this node
                    with open(self.config["data_dir"] + "/" +  node + ".txt", 'w') as f:
                        for new_text in data:
                            f.write(new_text + "\n")
                        for j in range(self.config["n_per_slave"]):
                            f.write("MeanImage.xrw\n")

                    # Load new order for this node
                    node_sockets.send("KL" + "\n")

            # Initialise screens selection state
            # (Deprecated with new self.state / init_system_state
            def init_screens_deprecated(self):
                for node in self.config["slaves"]:
                    init_screen_dict = {'object_id':'-1', 'selection_state':0}
                    self.screen_state[node] = {'screen_zero':init_screen_dict,
                                               'screen_one':init_screen_dict,
                                               'screen_two':init_screen_dict,
                                               'screen_three':init_screen_dict}

            # Deprecated.
            def get_json_from_server_state_deprecated(self):
                screens_json = []
                for node in self.config["slaves"]:
                    screens_json.append({"node": str(node),
                                        "ids":{"screen_zero": str(self.screen_state[node]['screen_zero']['object_id']),
                                                "screen_one": str(self.screen_state[node]['screen_one']['object_id']),
                                                "screen_two": str(self.screen_state[node]['screen_two']['object_id']),
                                                "screen_three": str(self.screen_state[node]['screen_three']['object_id'])
                                             },"selection_state": {"screen_zero":str(self.screen_state[node]['screen_zero']['selection_state']),
                                                "screen_one": str(self.screen_state[node]['screen_one']['selection_state']),
                                                "screen_two": str(self.screen_state[node]['screen_two']['selection_state']),
                                                "screen_three": str(self.screen_state[node]['screen_three']['selection_state'])
                                             }})

                json_data = {"status": str(self.status), "screens_per_node": str(self.config["n_per_slave"]), "columns": screens_json}

                json_response = json.dumps(json_data)
                return json_response

            # GENERIC csv to jsonp function (currently used for encube.)
            def to_json(self):
                # initialise the jsonp file
                print self.data.to_json(orient='records')
                #return self.data.to_json()

            def to_json_file(self, filename):
                with open(filename, 'w+') as f:
                    f.write("json_data=" + str(self.data.to_json(orient='records')))

            def start_encube_pr(self, with_master):
                self.start_only = True

                print self.config["slaves"][0], "\n"
                print "data_dir:", self.config["data_dir"]
                print "base_dir:", self.config["base_dir"]
                print "self.slave_ports", self.slave_ports

                # e.g. index sort list on age
                #self.sort_and_rewrite(['Age'])

                slave_pids = []
                for idx in range(len(self.config["slaves"])):
                    pid = fork()

                    if (pid == 0):
                        slave_pids.append(pid)
                        #if self.config["slaves"][idx] == "localhost":
                        if self.config["slaves"][idx] == "localhost":# \
                            # or self.config["slaves"][idx] == "n01" \
                            # or self.config["slaves"][idx] == "n02":
                            # Might need to be replaced with subprocess module for stability and readability (instead of system)
                            system("echo 'setenv S2PLOT_REMOTEPORT " + str(self.slave_ports[idx]) +
                                   " ; setenv OMP_NUM_THREADS " + str(self.config["num_omp_threads"]) +
                                   " && setenv S2PLOT_WIDTH " + str(self.config["slave_x"]) +
                                   " && setenv S2PLOT_HEIGHT " + str(self.config["slave_y"]) +
                                   " && setenv S2INTERSCANBUMP "+ str(self.config["slave_interscanbump"]) +
                                   " && cd " + str(self.config["base_dir"]) +
                                   " && echo " + str(self.config["slave_dev"]) + " | " + self.config["exenv"] + " "
                                   + self.config["exe"] + " 0 " + self.config_file_abs + " > /dev/null' | tcsh -s")

                            #system("echo 'export S2PLOT_REMOTEPORT=$slave_port && export S2PLOT_WIDTH=$slave_x && export S2PLOT_HEIGHT=$slave_y && $slave_interscanbump && cd $data_dir echo $slave_dev | $exenv $exe 0 $base_dir/$slave_file > /dev/null' | bash -s")
                        else:
                            system("ssh " + str(self.config["slaves"][idx]) + " 'setenv S2PLOT_REMOTEPORT " + str(self.slave_ports[idx]) +
                                   " ; setenv OMP_NUM_THREADS " + str(self.config["num_omp_threads"]) +
                                   " ; setenv S2PLOT_WIDTH " + str(self.config["slave_x"]) +
                                   " ; setenv S2PLOT_HEIGHT " + str(self.config["slave_y"]) +
                                   " ; " + str(self.config["slave_interscanbump"]) + " ; cd " + str(self.config["base_dir"]) +
                                   " ; echo " + str(self.config["slave_dev"]) + " | " +
                                   self.config["exenv"] + " " + self.config["exe"] + " 0 " + self.config_file_abs + " > /dev/null'")
                            #system("ssh $slave 'export S2PLOT_REMOTEPORT=$slave_port export S2PLOT_WIDTH=$slave_x export S2PLOT_HEIGHT=$slave_y $slave_interscanbump  cd $data_dir echo $slave_dev | $exenv $exe 0 $base_dir/$slave_file > /dev/null'")

                        #exit(0)
                        sys.exit()

                sleep(5)
                self.connect_to_encube_pr()
                self.send_to_all_slaves('L encube')
                #self.sort(['Age'])

                """
                sleep(5) # wait for telnet ports to be available on slaves
                for idx in range(len(self.config["slaves)):
                    #self.telnets.append(Telnet(str(self.config["slaves[idx]), str(self.config["slave_ports[idx])))
                    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    s.connect((self.config["slaves[idx], self.config["slave_ports[idx]))
                    self.sockets.append(s)
                sleep(5)
                self.send_to_all_slaves("L encube")
                self.receive(s)
                self.status = 1 # not currently useful in this function, but logical!
                """

                if (with_master):
                    pid = fork()
                    if (pid == 0):
                        print "child"
                        sys.exit()
                    else:
                        print "parent, child = " + str(pid)

                    ENV["S2PLOT_REMOTEPORT"] = str(self.config['master_port'])
                    ENV["S2PLOT_WIDTH"] = str(self.config['master_x'])
                    ENV["S2PLOT_HEIGHT"] = str(self.config['master_y'])

                    echo_MASTER = subprocess.Popen(['echo', self.config['master_dev']], stdout=subprocess.PIPE)
                    self.master_pipe = subprocess.Popen([self.config['exenv'] + self.config['exe'], str(1), self.config_file_abs],
                                              stdin=echo_MASTER.stdout, stdout=subprocess.PIPE)
                    echo_MASTER.stdout.close()

                    sleep(5)
                    self.connect_to_master()

                    self.send_to_all_slaves('L encube')


                    while True:
                        #print "in True"
                        line_all = ''
                        for line in self.master_pipe.stdout.readline():
                            if '\n' in line:
                                #print line
                                if 'K' in line_all or 'C' in line_all:
                                    self.send_to_all_slaves(line_all)
                                if 'Q' in line_all:
                                    return None
                                line_all = ''
                            else:
                                line_all += line
                else:
                    self.send_to_all_slaves('L encube')
                    # We should avoid sorting by a name that may not be in other csv files (e.g. image-hd vs astro)
                    #self.sort(['Age'])

        self.encube_m_obj = EncubeM(self.config_file)
        return self.encube_m_obj

    def __exit__(self, exc_type, exc_value, traceback):
        if self.encube_m_obj.start_only == False:
            if self.encube_m_obj.config["save_workflow"]:
                # ------------------------------------------------------------------
                # Dump session history into unique pickle file
                # ------------------------------------------------------------------
                if self.encube_m_obj.config["save_as_pickle"]:
                    pickle_filename = str(uuid4()) + '.p'
                    path_to_pickle = self.encube_m_obj.config["workflow_data_dir"]

                    pickle.dump(self.encube_m_obj.history,
                                 open(path_to_pickle + pickle_filename, "wb"),
                                 protocol=pickle.HIGHEST_PROTOCOL)

                # ------------------------------------------------------------------
                # save how to retrieve this session pickle in csv
                #   (I'm keeping it lightweight for now -- could be a full db)
                #   This csv will be loaded into pandas to query sessions.
                # ------------------------------------------------------------------
                if self.encube_m_obj.config["save_as_pickle"]:
                    line = str(to_datetime('now')) + ',' + \
                           self.encube_m_obj.config["workflow_data_dir"] + ',' + \
                           pickle_filename + '\n'
                else:
                    line = str(to_datetime('now')) + ',' + \
                           self.encube_m_obj.config["workflow_data_dir"] + ',' + \
                           self.encube_m_obj.json_filename + '\n'

                if path.isfile(self.encube_m_obj.config["db"]):
                    with open(self.encube_m_obj.config["db"], "ab") as f:
                        f.write(line)
                else:
                    # Create database
                    with open(self.encube_m_obj.config["db"], "wb") as f:
                        f.write('datetime,path_to_workflow,filename\n')
                        f.write(line)



    def stopencube_pr(self):
        if len(self.telnets) == 0:
            for idx in range(len(self.config["slaves"])):
                #self.telnets.append(Telnet(str(self.config["slaves[idx]), str(self.config["slave_ports[idx])))
                s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                s.connect((self.config["slaves"][idx], self.slave_ports[idx]))
                self.sockets.append(s)
            sleep(5)

        print 'killing s2plot!'
        #self.tidy_up()
        self.send_to_all_slaves("KQ")
        self.status = 0

if __name__ == '__main__':
    # TODO assert that the json file is of correct format.
    if len(sys.argv) == 3:
        config_file = sys.argv[1]
        assert (int(sys.argv[2]) == 1 or int(sys.argv[2]) == 0), "with_master should be 0 or 1"
        with_master = int(sys.argv[2])
        with EncubeMResource(config_file) as encube_m:
            encube_m.start_encube_pr(with_master)
            print "All slaves started."
    else:
        print "Usage:"
        print ""
        print "python encube_m.py path/to/config_file.json with_master"
