"""
request_handle.py

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
import json
from deepdiff import DeepDiff
import numpy as np
import ast
#from pprint import pprint

'''
Author:  Dany Vohl, 2015-2016.
'''

def data(encube_pr_obj, post_data, skip_history=False):

    value = post_data["data"]
    data = json.loads(value[0])
    #socket_send("encube_pr_obj J " + value[0])\

    print data

    try:
        if "colourmap_url" in data['volume']['properties']:
            encube_pr_obj.save_uri_to_png('colourmap', data['volume']['properties']['colourmap_url'])
    except:
        pass

    encube_pr_obj.update_state("data", data)

    if skip_history==False:
        if encube_pr_obj.config["save_workflow"]:
            encube_pr_obj.save_to_history("data", value[0], 0)

    encube_pr_obj.send_to_all_slaves("J " + str(value[0]).replace(' ', ''))

def autospin(encube_pr_obj, post_data, skip_history=False):
    # encube_pr_obj.update_state("data", data)

    # if skip_history == False:
    #     if encube_pr_obj.config["save_workflow"]:
    #         encube_pr_obj.save_to_history("data", value[0], 0)

    encube_pr_obj.send_to_all_slaves("a")

def zoom_in(encube_pr_obj, post_data, skip_history=False):
    # encube_pr_obj.update_state("data", data)

    # if skip_history == False:
    #     if encube_pr_obj.config["save_workflow"]:
    #         encube_pr_obj.save_to_history("data", value[0], 0)

    encube_pr_obj.send_to_all_slaves("+")

def zoom_out(encube_pr_obj, post_data, skip_history=False):
    # encube_pr_obj.update_state("data", data)

    # if skip_history == False:
    #     if encube_pr_obj.config["save_workflow"]:
    #         encube_pr_obj.save_to_history("data", value[0], 0)

    encube_pr_obj.send_to_all_slaves("-")

def show_stats(encube_pr_obj, post_data, skip_history=False):
    # encube_pr_obj.update_state("data", data)

    # if skip_history == False:
    #     if encube_pr_obj.config["save_workflow"]:
    #         encube_pr_obj.save_to_history("data", value[0], 0)

    encube_pr_obj.send_to_all_slaves("d")

def moment(encube_pr_obj, post_data, skip_history=False):
    value = post_data["moment"]
    data = json.loads(value[0])
    #socket_send("encube_pr_obj J " + value[0])\

    encube_pr_obj.update_state("moment", data)

    if skip_history==False:
        if encube_pr_obj.config["save_workflow"]:
            encube_pr_obj.save_to_history("moment", value[0], 0)

    if data['show'] is True:
        data['show'] = 1
    else:
        data['show'] = 0

    if data['mip'] is True:
        data['mip'] = 1
    else:
        data['mip'] = 0

    data['moment'] = int(data['moment'])

    print "Moment (after):"
    print str(json.dumps(data)).replace(' ', '')
    print ""

    encube_pr_obj.send_to_all_slaves("J " + str(json.dumps(data)).replace(' ', ''))

def range_filter(encube_pr_obj, post_data, skip_history=False):
    value = post_data["range_filter"]
    data = json.loads(value[0])
    #socket_send("encube_pr_obj J " + value[0])\

    encube_pr_obj.update_state("range_filter", data)

    if skip_history==False or data['skip_history']==0:
        if encube_pr_obj.config["save_workflow"]:
            encube_pr_obj.save_to_history("range_filter", value[0], 0)

    # The print of the node is +1 (e.g. 1 to 4) instead of 0 to 3.
    # This is risky business...
    data['panel'] = data['panel']-1

    data['min_filter'] = float(data['min_filter'])
    data['max_filter'] = float(data['max_filter'])

    encube_pr_obj.send_to_slave_by_node_name(data['node'], "J " + str(json.dumps(data)).replace(' ', ''))

def sort(encube_pr_obj, post_data, skip_history=False):
    value = post_data["sort"]
    data = json.loads(value[0])
    #print data["sort1"]
    #print data["sort2"]
    #socket_send("encube_pr_obj sort " + str(data["sort1"]) + " " + str(data["sort2"]))

    encube_pr_obj.update_state("sort", data)

    if skip_history==False:
        if encube_pr_obj.config["save_workflow"]:
            encube_pr_obj.save_to_history("sort", data, 0)

    sort_on_columns = [str(data["sort1"]), str(data["sort2"])]
    encube_pr_obj.sort(sort_on_columns)
    
def region(encube_pr_obj, post_data, skip_history=False):
        value = post_data["region"]
        data = json.loads(value[0])

        encube_pr_obj.update_state("region", data)

        if skip_history==False:
            if encube_pr_obj.config["save_workflow"]:
                encube_pr_obj.save_to_history("region", post_data, 0)

        #socket_send("encube_pr_obj roi1 " + str(data["region1"]))
        encube_pr_obj.send_to_all_slaves("O 1 " + str(data["region1"]))

        #socket_send("encube_pr_obj roi2 " + str(data["region2"]))
        encube_pr_obj.send_to_all_slaves("O 2 " + str(data["region2"]))

        #socket_send("# " + str(data["r1colour"][0] / 255.0) +
        encube_pr_obj.send_to_all_slaves("# " + str(data["r1colour"][0] / 255.0) +
                     " " + str(data["r1colour"][1] / 255.0) +
                     " " + str(data["r1colour"][2] / 255.0) +
                     " " + str(data["r1alpha"]) +
                     " " + str(data["r2colour"][0] / 255.0) +
                     " " + str(data["r2colour"][1] / 255.0) +
                     " " + str(data["r2colour"][2] / 255.0) +
                     " " + str(data["r2alpha"])
                     )

# Deprecated.
def minicaveReorder(encube_pr_obj, post_data, skip_history=False):
        value = post_data["minicaveReorder"]
        data = json.loads(value[0])

        encube_pr_obj.update_state("minicaveReorder", data)

        if skip_history==False:
            if encube_pr_obj.config["save_workflow"]:
                encube_pr_obj.save_to_history("minicaveReorder", post_data, 0)

        # construct the command in the form:
        #     n02 32,C 31,C 13,C 14,S n03 ...
        command_to_send = []
        for node in data:
            command_to_send.append([node["node"], [line['line'] for line in node["data"]]])

        encube_pr_obj.rewrite_and_send_to_specific(command_to_send)

def reorder(encube_pr_obj, post_data, skip_history=False):
    value = post_data["reorder"]
    data = json.loads(value[0])

    encube_pr_obj.update_state("reorder", data)

    if skip_history==False:
        if encube_pr_obj.config["save_workflow"]:
            encube_pr_obj.save_to_history("reorder", post_data, 0)

    for line in data:
        if (line['id'] == '-1'):
            command = 'u ' + line['panel']
            encube_pr_obj.send_to_slave_by_node_name(line['node'], command)
        else:
            if encube_pr_obj.config["app_type"] == 'brain_xrw':
                type = encube_pr_obj.data.loc[encube_pr_obj.data['Subject_ID']==int(line['id'])].Type.iloc[0]

                # TODO : pool these two commands together (should probably simply use JSON for everything)
                command = 'l ' + line['panel'] + ' ' + line['id'] + ' ' + type[0] + ' MeanImage.xrw'
            elif encube_pr_obj.config["app_type"] == 'astro_fits':
                command = 'l ' + line['panel'] + ' ' + line['id']

            encube_pr_obj.send_to_slave_by_node_name(line['node'], command)

        # Keep track of highlight states
        command = 'h ' + line['panel'] + ' ' + line['selected']
        encube_pr_obj.send_to_slave_by_node_name(line['node'], command)

def load_one(encube_pr_obj, post_data, skip_history=False):
    print post_data

    value = post_data["loadOne"]
    data = json.loads(value[0])

    encube_pr_obj.update_state("loadOne", data)

    if skip_history==False:
        if encube_pr_obj.config["save_workflow"]:
            encube_pr_obj.save_to_history("loadOne", post_data, 0)

    if encube_pr_obj.config["app_type"] == 'brain_xrw':
        type = encube_pr_obj.data.loc[encube_pr_obj.data['Subject_ID']==int(data[0]['id'])].Type.iloc[0]
        command = 'l ' + data[0]['panel'] + ' ' + data[0]['id'] + ' ' + type[0] + ' MeanImage.xrw'
    elif encube_pr_obj.config["app_type"] == 'astro_fits':
        command = 'l ' + data[0]['panel'] + ' ' + data[0]['id']

    encube_pr_obj.send_to_slave_by_node_name(data[0]['node'], command)

    # Keep track of highlight states
    command = 'h ' + data[0]['panel'] + ' ' + data[0]['selected']
    encube_pr_obj.send_to_slave_by_node_name(data[0]['node'], command)

def update_selected(encube_pr_obj, post_data, skip_history=False):
    value = post_data["update_selected"]
    data = json.loads(value[0])

    encube_pr_obj.update_state("update_selected", data)

    if skip_history==False:
        if encube_pr_obj.config["save_workflow"]:
            encube_pr_obj.save_to_history("update_selected", post_data, 0)

    # Highlight selected in S2PLOT
    command = 'h ' + data[0]['panel'] + ' ' + data[0]['selected']
    encube_pr_obj.send_to_slave_by_node_name(data[0]['node'], command)

def histogram(encube_pr_obj, post_data):
    value = post_data["histogram"]
    data = json.loads(value[0])

    json_data = encube_pr_obj.request_to_slave(data[0]['node'], "H "+ data[0]['panel'])

    if encube_pr_obj.config["save_workflow"]:
        encube_pr_obj.save_to_history("histogram", post_data, 1)

    return json_data

# def moment(encube_pr_obj, post_data):
#     value = post_data["moment"]
#     data = json.loads(value[0])
#
#     json_data = encube_pr_obj.request_to_slave(data[0]['node'], "M "+ data[0]['panel'])
#
#     if encube_pr_obj.config["save_workflow"]:
#         encube_pr_obj.save_to_history("moment", post_data, 1)
#
#     return json_data

def all_histograms(encube_pr_obj, post_data):
    value = post_data["all_histogram"]
    data = json.loads(value[0])

    json_data = encube_pr_obj.request_to_all_slaves(data[0]['node'], "H ")

    if encube_pr_obj.config["save_workflow"]:
        encube_pr_obj.save_to_history("all_histogram", post_data, 1)

    return json_data


def ntracks(encube_pr_obj, post_data):
    json_data = encube_pr_obj.request_to_all_slaves("T")

    if encube_pr_obj.config["save_workflow"]:
        encube_pr_obj.save_to_history("ntracks", post_data, 0)

    return json_data

def state(encube_pr_obj, post_data):
    if encube_pr_obj.status:
        json_data = encube_pr_obj.request_to_all_slaves("S")
    else:
        json_data = encube_pr_obj.get_json_from_system_state()

    return json_data

def save_history(encube_pr_obj):
    encube_pr_obj.save_history()

def load_history_record(encube_pr_obj, post_data):
    value = post_data["load_state"]
    history_id = json.loads(value[0])

    hist_state = encube_pr_obj.history["records"][history_id[0]['id']]['state']

    # For 'data', we only need to know if there's been a change or not.
    # If there is a change, we send the dict directly to encube-PR through data(...)
    # ------------------------------------------------------------------------------
    # Change related to (sharevol related) 'data'?
    if compare_states(hist_state['data']['sharevol']['url'],
                      encube_pr_obj.state['data']['sharevol']['url']) != False or \
        compare_states(hist_state['data']['sharevol']['res'],
                       encube_pr_obj.state['data']['sharevol']['res']) != False or \
        compare_states(hist_state['data']['sharevol']['scale'],
                       encube_pr_obj.state['data']['sharevol']['scale']) != False or \
        compare_states(hist_state['data']['sharevol']['volume'],
                       encube_pr_obj.state['data']['sharevol']['volume']) != False or \
        compare_states(hist_state['data']['sharevol']['slicer'],
                       encube_pr_obj.state['data']['sharevol']['slicer']) != False:

        # send to encube-PR through request_handle.data(...)
        #data(encube_pr_obj, {'data': [json.dumps(hist_state['data']['sharevol'])]}, skip_history=True)
        post_request = {'data': [json.dumps(hist_state['data']['sharevol'])]}
        data(encube_pr_obj, post_request, skip_history=True)

    # Change related to (imagehd related) 'data'?
    if compare_states(hist_state['data']['imagehd']['labels'],
                      encube_pr_obj.state['data']['imagehd']['labels']) != False or \
        compare_states(hist_state['data']['imagehd']['rotate'],
                       encube_pr_obj.state['data']['imagehd']['rotate']) != False or \
        compare_states(hist_state['data']['imagehd']['trackSamples'],
                       encube_pr_obj.state['data']['imagehd']['trackSamples']) != False or \
        compare_states(hist_state['data']['imagehd']['trackThickness'],
                       encube_pr_obj.state['data']['imagehd']['trackThickness']) != False or \
        compare_states(hist_state['data']['imagehd']['trackOpacity'],
                       encube_pr_obj.state['data']['imagehd']['trackOpacity']) != False:

        # send to encube-PR through request_handle.data(...)
        post_request = {'data': [json.dumps(hist_state['data']['imagehd'])]}
        data(encube_pr_obj, post_request, skip_history=True)

    # Change in nodes requires more details. We need to know if there is a change, and what kind of change it is.
    # -----------------------------------------------------------------------------------------------------------

    if compare_states(hist_state['moment'],
                      encube_pr_obj.state['moment']) != False:
        post_request = {'moment': [json.dumps(hist_state['moment'])]}
        moment(encube_pr_obj, post_request, skip_history=True)

    # Change in nodes' states
    i_node = 0
    for node in hist_state['nodes']:
        for i_panel in range(encube_pr_obj.config['n_per_slave']):
            if node['panels'][i_panel]['selection_state'] != encube_pr_obj.state['nodes'][i_node]['panels'][i_panel]['selection_state']:
                # Update state for that panel
                # We now know:
                #   which node it is
                #   which panel
                #   if it's selected or not

                '''
                Example of 'post_data' in update_selected(encube_pr_obj, post_data)
                    {u'update_selected': [u'[{"node":"localhost","panel":"2","selected":"0"}]']}
                '''
                post_data = {"update_selected": [json.dumps([{"node":encube_pr_obj.config['slaves'][i_node],
                                                 "panel":str(i_panel),
                                                 "selected":str(node['panels'][i_panel]['selection_state'])}])]}
                update_selected(encube_pr_obj, post_data, skip_history=True)

            if node['panels'][i_panel]['file_id'] != encube_pr_obj.state['nodes'][i_node]['panels'][i_panel]['file_id']:
                # Update file_id for that panel
                # We now know:
                #   which node it is
                #   which panel
                #   the file_id
                '''
                Example of 'post_data' in load_one(encube_pr_obj, post_data)
                    {u'loadOne': [u'[{"node":"localhost","panel":"3","id":"2","selected":"0"}]']}
                '''
                post_data = {"loadOne": [json.dumps([{"node":encube_pr_obj.config['slaves'][i_node],
                                         "panel":str(i_panel),
                                         "id":str(node['panels'][i_panel]['file_id']),
                                         "selected":str(node['panels'][i_panel]['selection_state'])}])]}
                load_one(encube_pr_obj, post_data, skip_history=True)

        i_node += 1

    return False

def compare_states(hist_state, current_state):
        '''
        Compares regions of the dicts to check for differences
        :param hist_state: history_state dict to return to
        :param current_state: current_state to be compared to
        :return: ?
        '''
        diff = DeepDiff(hist_state, current_state)
        #pprint (diff, indent=2)
        if len(diff) > 0:
            return diff
        else:
            return False