"""
webserver.py

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
import SimpleHTTPServer
import SocketServer
import urlparse
import socket
import os
import sys
from encube_m import EncubeMResource
import request_handle

'''
Authors: Owen Kaluza, Dany Vohl, 2015-2016.
'''

if len(sys.argv) == 2:
    config_file = sys.argv[1]

def connect(encube_m_obj):
    try:
        encube_m_obj.connect_to_encube_pr()
    except:
        print "Web server cannot connect to encube_pr"

PORT = 8000

# Initialise encube_m object
with EncubeMResource(config_file) as encube_m_obj:
    encube_m_obj.to_json_file(encube_m_obj.config["control_dir"] + '/' + encube_m_obj.config["output_json"])
    connect(encube_m_obj)

    os.chdir("control")

    class ServerHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
        def do_GET(self):
            SimpleHTTPServer.SimpleHTTPRequestHandler.do_GET(self)

        def do_POST(self):
            # Extract and print the contents of the POST
            length = int(self.headers['Content-Length'])
            post_data = urlparse.parse_qs(self.rfile.read(length).decode('utf-8'))

            #print post_data

            #for key, value in post_data.iteritems():
            #    print "KEY %s= VALUE %s" % (key, value[0])
            def socket_send(data):
                #create an INET, STREAMing socket
                s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                #now forward the data to the perl script
                s.connect(("localhost", 7777))
                s.send(data)
                s.close()

            """
            if (encube_m_obj.status == 0):
                print "Trying to connect to encube_pr"
                connect(encube_m_obj)
                if (encube_m_obj.status):
                    print "Connected to encube_pr"
                else:
                    print "I won't send anything this time. Ghost call."
                    send_json_back = False
            else:
            """
            send_json_back, json_data = act_on_post(post_data)

            if send_json_back:
                self.send_response(200)
                self.send_header("Content-type", "application/json")
                self.end_headers()
                self.wfile.write(json_data)
            else:
                self.send_response(200)
                self.send_header("Content-type", "text/html")
                self.end_headers()

    def act_on_post(post_data):
        if "data" in post_data:
            request_handle.data(encube_m_obj, post_data)
            return False, False

        if "moment" in post_data:
            request_handle.moment(encube_m_obj, post_data)
            return False, False

        if "range_filter" in post_data:
            request_handle.range_filter(encube_m_obj, post_data)
            return False, False

        if "sort" in post_data:
            request_handle.sort(encube_m_obj, post_data)
            return False, False

        if "region" in post_data:
            request_handle.region(encube_m_obj, post_data)
            return False, False

        if "minicaveReorder" in post_data:
            request_handle.minicaveReorder(encube_m_obj, post_data)
            return False, False

        if "reorder" in post_data:
            request_handle.reorder(encube_m_obj, post_data)
            return False, False

        if "loadOne" in post_data:
            request_handle.load_one(encube_m_obj, post_data)
            return False, False

        if "update_selected" in post_data:
            request_handle.update_selected(encube_m_obj, post_data)
            return False, False

        if "histogram" in post_data:
            json_data = request_handle.histogram(encube_m_obj, post_data)

            if json_data != False:
                return True, json_data
            else:
                return False, False

        if "moment" in post_data:
            json_data = request_handle.moment(encube_m_obj, post_data)

            if json_data != False:
                return True, json_data
            else:
                return False, False

        if "ntracks" in post_data:
            json_data = request_handle.ntracks(encube_m_obj, post_data)

            if json_data != False:
                return True, json_data
            else:
                return False, False

        if "state" in post_data:
            json_data = request_handle.state(encube_m_obj, post_data)

            if json_data != False:
                return True, json_data
            else:
                return False, False

        if "save_history" in post_data:
            request_handle.save_history(encube_m_obj)
            return False, False

        if "load_state" in post_data:
            json_data = request_handle.load_history_record(encube_m_obj, post_data)

            if json_data != False:
                return True, json_data
            else:
                return False, False

        if "autospin" in post_data:
            request_handle.autospin(encube_m_obj, post_data)
            return False, False

        if "zoomIn" in post_data:
            request_handle.zoom_in(encube_m_obj, post_data)
            return False, False

        if "zoomOut" in post_data:
            request_handle.zoom_out(encube_m_obj, post_data)
            return False, False

        if "showStats" in post_data:
            request_handle.show_stats(encube_m_obj, post_data)
            return False, False

        if "stopEncubePR" in post_data:
            encube_m_obj.send_to_all_slaves("KQ")
            return False, False

        if "startEncubePR" in post_data:
            # Would need to be started on a different thread I think. Not currently working from the browser.
            encube_m_obj.start_encube_pr()
            return False, False

        # Nothing was found/done.
        print "Nothing prepared for what was asked from client. Nothing done."
        return False, False

    Handler = ServerHandler

    httpd = SocketServer.TCPServer(("", PORT), Handler)

    print "serving at port", PORT
    httpd.serve_forever()
