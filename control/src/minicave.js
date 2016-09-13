/* @preserve
 *
 * minicave.js
 *
 * Copyright (c) 2015-2016 Dany Vohl, David G. Barnes, Christopher J. Fluke,
 *                         Yuri Benovitski, Tsz Ho Wong, Owen L Kaluza, Toan D. Nguyen.
 *
 * This file is part of encube.
 *
 * encube is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * encube is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with encube.  If not, see <http://www.gnu.org/licenses/>.
 *
 * We would appreciate it if research outcomes using encube would
 * provide the following acknowledgement:
 *
 * "Visual analytics of multidimensional data was conducted with encube."
 *
 * and a reference to
 *
 * Dany Vohl, David G. Barnes, Christopher J. Fluke, Govinda Poudel, Nellie Georgiou-Karistianis,
 * Amr H. Hassan, Yuri Benovitski, Tsz Ho Wong, Owen L Kaluza, Toan D. Nguyen, C. Paul Bonnington. (2016).
 * Large-scale comparative visualisation of sets of multidimensional data. PeerJ Computer Science, In Press.
 *
 */

/* @preserve
 *
 * encube: meta-controller
 * Miniature CAVE2 - CAVE2's screens representation for global data manipulation
 *
 * Copyright (c) 2015, Dany Vohl. All rights reserved.
 * Author: Dany Vohl - dvohl [at] astro [dot] swin [dot] edu
 *
 */

var $j = jQuery.noConflict(); // To avoid conflict with other libraries that also use $('some_variable').
var SERVER = "http://" + window.location.host;
var TOTAL_SCREENS = 0;
var N_NODES = 0;
var N_PER_NODE = 0;
var NODES;
var BEFORESORT = [];        // Used to keep track of previous order before reordering.
var STATUS=0;
var NODES=[];
var SELECTED_STAMPS = [];
var TOGGLE_VIZ = 1;
var WORKFLOW_FILE = "";
var SAVE_WORKFLOW = 0;
var MINICAVE;
var HISTORY_DATA;
var APP_TYPE = "";
var DATA;

$j(function()
{
    DATA = json_data;
    var minicave = function(data)
    {
        // DATA just currently used as quick fix -- we have access to json_data everywhere
        // as it is in imagehd.json (which should be renamed!)
        this._data = DATA;

        // Currently sorts by Age to mimic imagehd...
        // TODO: Needs to be synced (not just mimic) so that it starts with real initial setup
        this._data.sort(
            function(a,b)
            {
                //return parseFloat(a.Age) - parseFloat(b.Age)
                return parseFloat(a.Subject_ID) - parseFloat(b.Subject_ID)
            }
        );

        this._init();
    };

    // PROTOTYPE
    minicave.prototype =
    {
        _init: function()
        {
            var self = this;
            this._screensUl = $j('#screensUl');
            this._choicesUl = $j('#choicesUl');
            this._getHistogram = $j('#getHistogram');

            $j( "#screensUl" ).resizable({
              aspectRatio: 768 / 200
            });

            $j("#divHistory").hide();
            $j("#divTag").hide();
            //$j("#divMoment").hide();

            /*$j("#toggle_history_request").click(function(){
                if (TOGGLE_VIZ)
                {
                    $j("#toggle_history_request").html('show history');
                    TOGGLE_VIZ = false;
                }
                else
                {
                    $j("#toggle_history_request").html('show viz');
                    TOGGLE_VIZ = true;
                }

                $j("#divSVG").toggle("slow");
                $j("#divHistory").toggle("slow");

            });*/

            ///////
            $j("#toggle_history_request").change(function() {
                if ($j("#toggle_history_request").val() == 'visualisation') {
                    $j("#divSVG").show();//.toggle("slow");
                    //$j("#divMoment").hide();
                    $j("#divHistory").hide();
                    $j("#divTag").hide();
                }

                /*if ($j("#toggle_history_request").val() == 'moment') {
                    $j("#divSVG").hide();
                    $j("#divMoment").show();
                    $j("#divHistory").hide();//.toggle("slow");
                    $j("#divTag").hide();
                }*/

                if ($j("#toggle_history_request").val() == 'history') {
                    $j("#divSVG").hide();
                    //$j("#divMoment").hide();
                    $j("#divHistory").show();//.toggle("slow");
                    $j("#divTag").hide();
                }

                if ($j("#toggle_history_request").val() == 'tags') {
                    $j("#divSVG").hide();
                    //$j("#divMoment").hide();
                    $j("#divHistory").hide();
                    $j("#divTag").show();//.toggle("slow");
                }
            });
            ///////


            this._createPanels();
            //this._createChoices();

            $j("#stopEncubePR").click(function()
            {
                if (confirm('Are you sure you want to stop encube-PR?')) {
                    var JSONstring = [];
                    JSONstring.push({'node':$j(".ui-selected").attr('node'),
                                     'panel':$j(".ui-selected").attr('panelScreenNumber')});

                    $j.ajax({
                        type: "POST",
                        url: SERVER + "/update",
                        data: "stopEncubePR="+ encodeURIComponent(JSON.stringify(JSONstring))});

                    $j("startstopEncubePR").text = "Start encube-PR";
                    $j("startstopEncubePR").attr('Action', 'Start');
                }
            });

            $j("#request_from_host").change(function()
            {
                if ($j("#request_from_host").val() == 'ntracks') {
                    var JSONstring = [];
                    JSONstring.push({'dummy':'data'});
                    console.log('In ntracks');

                    var start_time = new Date().getTime();

                    // TODO say result in a div that pops up.
                    $j.ajax({
                        type: "POST",
                        url: SERVER + "/update",
                        data: "ntracks="+ encodeURIComponent(JSON.stringify(JSONstring)),
                        success: function(json){
                            var request_time = new Date().getTime() - start_time;
                            console.log("Query ntracks -- request time (ms): " + request_time);
                            minicave.prototype._ntracks(json);
                        }
                    });
                }

                if ($j("#request_from_host").val() == 'histogram') {
                    console.log('In histogram');
                    if ($j(".ui-selected, .ui-selecting").length == 1)
                    {
                        if ($j(".ui-selected").attr('Subject_ID') != '-1')
                        {
                            var JSONstring = [];
                            JSONstring.push({'node':$j(".ui-selected").attr('node'),
                                             'panel':$j(".ui-selected").attr('panelScreenNumber')});

                            var start_time = new Date().getTime();

                            $j.ajax({
                                type: "POST",
                                url: SERVER + "/update",
                                data: "histogram="+ encodeURIComponent(JSON.stringify(JSONstring)),
                                success: function(json){
                                    var request_time = new Date().getTime() - start_time;
                                    console.log("Query histogram -- request time (ms): " + request_time);
                                    minicave.prototype._histogram(json, 'histogram');
                                }
                            });
                        }
                    }
                    else
                    {
                        // TODO shouldn't be an alert... Should simply be a div that pops up.
                        alert("To request an histogram, select one screen, and one screen only.")
                    }
                }

                if ($j("#request_from_host").val() == 'area') {
                    console.log('In histogram');
                    if ($j(".ui-selected, .ui-selecting").length == 1)
                    {
                        if ($j(".ui-selected").attr('Subject_ID') != '-1')
                        {
                            var JSONstring = [];
                            JSONstring.push({'node':$j(".ui-selected").attr('node'),
                                             'panel':$j(".ui-selected").attr('panelScreenNumber')});

                            $j.ajax({
                                type: "POST",
                                url: SERVER + "/update",
                                data: "histogram="+ encodeURIComponent(JSON.stringify(JSONstring)),
                                success: function(json){
                                    minicave.prototype._histogram(json, 'area');
                                }
                            });
                        }
                    }
                    else
                    {
                        // TODO shouldn't be an alert... Should simply be a div that pops up.
                        alert("To request an histogram, select one screen, and one screen only.")
                    }
                }

                if ($j("#request_from_host").val() == 'moment') {
                    console.log('In moment');
                    if ($j(".ui-selected, .ui-selecting").length == 1)
                    {
                        if ($j(".ui-selected").attr('Subject_ID') != '-1')
                        {
                            var JSONstring = [];
                            JSONstring.push({'node':$j(".ui-selected").attr('node'),
                                             'panel':$j(".ui-selected").attr('panelScreenNumber')});

                            $j.ajax({
                                type: "POST",
                                url: SERVER + "/update",
                                data: "moment="+ encodeURIComponent(JSON.stringify(JSONstring)),
                                success: function(json){
                                    minicave.prototype._moment(json, 'moment');
                                }
                            });
                        }
                    }
                    else
                    {
                        // TODO shouldn't be an alert... Should simply be a div that pops up.
                        alert("To request an histogram, select one screen, and one screen only.")
                    }
                }
            });

            $j("#swap").click(function()
            {
                if ($j(".ui-selected").length == 2)
                {
                    self._swap(SELECTED_STAMPS);

                    $j('#screensUl .ui-selected').effect("shake",{distance:3});//.removeClass('ui-selected');
                    //SELECTED_STAMPS = [];
                }
                else
                {
                    alert('You need to select 2 screens before swaping.')
                }
            });

            $j("#snapshot").click(function()
            {
                /* To take a 'snapshot' of the browser's state */
                console.log("In snapshot");
                html2canvas(document.body, {
                  onrendered: function(canvas) {
                    $j("#divSVG").appendChild(
                          '<div class="scroll-content-item ui-widget-header">'+ canvas + '</div>'
                        );

                    console.log(canvas);
                    //var img = canvas.toDataURL();
                  }
                });
            });

            $j("#unloadSelected").click(function()
            {
                self._unload(SELECTED_STAMPS);

                $j('#screensUl .ui-selected').effect("shake",{distance:3}).removeClass('ui-selected');
                SELECTED_STAMPS = [];

                var i=0;
                //console.log('Update BEFORESORT in unloadSelected');
                $j('#screensUl').find('li').each(function () {
                    BEFORESORT[i]=$j(this).attr('Subject_ID');
                    //console.log(i + ' ' + BEFORESORT[i]);
                    i++;
                })

            });

            $j("#loadChecked").click(function()
            {
                var i=0;
                $j('.choices').find('tr').each(function ()
                {
                    if (i<TOTAL_SCREENS)
                    {
                        var row = $j(this);
                        if (row.find('input[type="checkbox"]').is(':checked'))
                        {
                            var Subject_ID = row.find('.td_subject_id').text();

                            $j("#screens-li-" + i).attr("Subject_ID", Subject_ID);
                            //$j("#screens-li-" + i).removeClass('ui-selected');
                            $j("#screens-li-" + i).html("<div class='handle'><span class='ui-icon ui-icon-carat-2-n-s'></span></div>" +
                                Subject_ID);

                            if ($j("#screens-li-" + i).hasClass('ui-selected'))
                                var is_selected = "1";
                            else
                                var is_selected = "0";

                            var JSONstring = [];
                            JSONstring.push({'node':$j("#screens-li-" + i).attr('node'),
                                'panel':$j("#screens-li-" + i).attr('panelScreenNumber'),
                                'id':Subject_ID,
                                'selected': is_selected});

                            console.log('');
                            console.log('node', $j("#screens-li-" + i).attr('node'));
                            console.log('panel', $j("#screens-li-" + i).attr('panelScreenNumber'));
                            console.log('id', Subject_ID);
                            console.log('selected', is_selected);
                            console.log('');

                            $j.ajax({
                                type: "POST",
                                url: SERVER + "/update",
                                data: "loadOne="+ encodeURIComponent(JSON.stringify(JSONstring))
                            });
                            if (SAVE_WORKFLOW == 1)
                            {
                                minicave.prototype._createWorkflowHistory();
                            }
                            i++;
                        }
                    }
                });
            });

            /*******************************************************************************
            * _screensUl : list of screens (ul, li)
            *
            *   What is being done here:
            *       .sortable : jQuery function enabling sortable (drag and order)
            *
            *       .find.addClass.prepend: add necessary info for the handle used with sortable
            *
            *       .bind : enables multi-select by forcing 'ctrl' to be locked
            *       .selectable : jQuery function that lets select specific screen
            *******************************************************************************/
            this._screensUl
                .sortable({
                    revert: true,
                    handle: ".handle",
                    //items: "li:not(.unsortable)",
                     // On stop, sorts the text list
                     // Should also then send info to S2PLOT.
                    stop: function()
                    {
                        self._updateNodes();
                        self._reorder();
                    }
                });


            this._screensUl
                .bind("mousedown", function(e) {
                        e.metaKey = true;
                    })
                .selectable({
                    filter: "li",
                    cancel: ".handle",
                    selecting: function(event, ui)
                    {

                    },
                    selected: function(event, ui)
                    {
                        console.log($j(ui.selected).attr('id'));
                        console.log(SELECTED_STAMPS);

                        // If SWAP, enable swapping functionality.
                        var index = SELECTED_STAMPS.indexOf($j(ui.selected).attr('id'));
                        if (index > -1)
                        {
                            console.log($j(ui.selected).attr('id') + " is selected.");
                            SELECTED_STAMPS.splice(index, 1);
                            $j(ui.selecting).prop("selected", false);
                            var select = '0';
                        }
                        else
                        {
                            console.log($j(ui.selected).attr('id') + " is not selected.");
                            SELECTED_STAMPS.push($j(ui.selected).attr('id'));
                            var select = '1';
                        }

                        var JSONstring = [];

                        JSONstring.push({'node':$j(ui.selected).attr('node'),
                                         'panel':$j(ui.selected).attr('panelScreenNumber'),
                                         'selected':select})

                        $j.ajax({
                            type: "POST",
                            url: SERVER + "/update",
                            data: "update_selected="+ encodeURIComponent(JSON.stringify(JSONstring))
                        });

                        if (SAVE_WORKFLOW == 1)
                        {
                            minicave.prototype._createWorkflowHistory();
                        }
                    },
                    unselecting: function(event, ui)
                    {
                        console.log("unselecting: " + $j(ui.unselecting).attr('id'));
                        // Remove unselected item's id from "SELECTED_STAMPS"
                        var index = SELECTED_STAMPS.indexOf($j(ui.unselecting).attr('id'));
                        if (index > -1)
                        {
                            SELECTED_STAMPS.splice(index, 1);
                        }

                        var JSONstring = [];

                        JSONstring.push({'node':$j(ui.unselecting).attr('node'),
                                         'panel':$j(ui.unselecting).attr('panelScreenNumber'),
                                         'selected':'0'})

                        $j.ajax({
                            type: "POST",
                            url: SERVER + "/update",
                            data: "update_selected="+ encodeURIComponent(JSON.stringify(JSONstring))
                        });

                        if (SAVE_WORKFLOW == 1)
                        {
                            minicave.prototype._createWorkflowHistory();
                        }
                    },
                    unselected: function(event, ui)
                    {
                        if ($j(".ui-selected").length > 0)
                        {}
                    },
                });

            /*$j(':checkbox[id=selectAll]').click(function(){
                $j(':checkbox[class=inputChkbox]').prop('checked', this.checked);
            });*/
        },

        _histogram: function(json, type)
        {
            //console.log(json);
            //var a = ay_histogram('histogram', data, {margin: [10, 10], bin_width: 100});

            // transform data that is already binned into data
            // that is better for use in D3
            // we want to create something like this:
            // [
            //  { "x": 0,  "y": 30000 },
            //  { "x": 10, "y": 80000 },
            //  ...
            // ]
            //
            // Could eventually be used as
            if (typeof json === 'string')
                json = jQuery.parseJSON(json);

            // use the name of the group to initialize the array
            var title = 'Node: ' + json.node + ', Panel:' + json.panel;
            var nbins = json.nbins;
            var dmin = json.dmin;
            var dmax = json.dmax;
            var incr = json.incr;
            var data = [];

            // we have a max bin for our histogram, must ensure
            // that any bins > maximum bin are rolled into the
            // last bin that we have

            var binCounts = {};
            for(var i = 0; i < json.data.length; i++) {
                var xValue = json.data[i].bin;
                // bin cannot exceed the maximum bin
                xValue = ( xValue > dmax ? dmax : xValue);
                var yValue = json.data[i].count;

                if(binCounts[xValue] === undefined) {
                    binCounts[xValue] = 0;
                }
                binCounts[xValue] += yValue;
            }

            // add the bin counts in
            for(var bin in binCounts) {
                data.push({"x": bin, "y": binCounts[bin]});
            }

            if (type == 'histogram')
            {
                createHistogram(data, nbins, dmin, dmax, incr, title,
                                json.node, json.panel,
                                json.mean, json.stand_dev,
                                json.min_filter, json.max_filter);
            }
            else
            {
                linked_areas(title,
                            json.node, json.panel,
                            json.mean, json.stand_dev,
                            json.min_filter, json.max_filter);
            }

            if (SAVE_WORKFLOW == 1)
            {
                minicave.prototype._createWorkflowHistory();
            }
        },

        _moment: function(json, type)
        {

        },

        _ntracks: function(json)
        {
            if (typeof json === 'string')
              json = jQuery.parseJSON(json);
            console.log(json);
            createScatterTracks(json.labels, json.colours);

            if (SAVE_WORKFLOW == 1)
            {
                minicave.prototype._createWorkflowHistory();
            }
        },

        // Swaps two items that have been manually selected
        _swap: function(SELECTED_STAMPS)
        {
            // Update IDs
            $j("#" + SELECTED_STAMPS[0]).attr('id',"tempId0");
            $j("#" + SELECTED_STAMPS[1]).attr('id',"tempId1");
            $j("#tempId0").attr('id',SELECTED_STAMPS[1]);
            $j("#tempId1").attr('id',SELECTED_STAMPS[0]);

            // Update Subject_ID
            var Subject_ID = $j("#" + SELECTED_STAMPS[0]).attr('Subject_ID');
            $j("#" + SELECTED_STAMPS[0]).attr('Subject_ID',$j("#" + SELECTED_STAMPS[1]).attr('Subject_ID'));
            $j("#" + SELECTED_STAMPS[1]).attr('Subject_ID',Subject_ID);

            // Update innerHTML
            var value1 = document.getElementById(SELECTED_STAMPS[0]).innerHTML;
            document.getElementById(SELECTED_STAMPS[0]).innerHTML = document.getElementById(SELECTED_STAMPS[1]).innerHTML;
            document.getElementById(SELECTED_STAMPS[1]).innerHTML = value1;

            var JSONstring = [];

            JSONstring.push({'node':$j("#" + SELECTED_STAMPS[0]).attr('node'),
                             'panel':$j("#" + SELECTED_STAMPS[0]).attr('panelScreenNumber'),
                             'id':$j("#" + SELECTED_STAMPS[0]).attr('Subject_ID'),
                             'selected': '1'})
            JSONstring.push({'node':$j("#" + SELECTED_STAMPS[1]).attr('node'),
                             'panel':$j("#" + SELECTED_STAMPS[1]).attr('panelScreenNumber'),
                             'id':$j("#" + SELECTED_STAMPS[1]).attr('Subject_ID'),
                             'selected': '1'})

            $j.ajax({
                type: "POST",
                url: SERVER + "/update",
                data: "reorder=" + encodeURIComponent(JSON.stringify(JSONstring))
            });

            if (SAVE_WORKFLOW == 1)
            {
                minicave.prototype._createWorkflowHistory();
            }
            //ajaxPost(SERVER + "/update", "minicaveReorder=" + encodeURIComponent(JSON.stringify(JSONstring)));

            // Update "BEFORESORT" to maintain the state of the minicave' order.
            var afterSort = $j('#screensUl').sortable('toArray', {attribute:'Subject_ID'});
            for (i=0; i<afterSort.length; i++)
                BEFORESORT[i] = afterSort[i];
        },

        _unload: function(SELECTED_STAMPS)
        {
            var JSONstring = [];
            for (var i=0; i<SELECTED_STAMPS.length; i++)
            {
                $j("#" + SELECTED_STAMPS[i]).attr('Subject_ID',"-1");
                document.getElementById(SELECTED_STAMPS[i]).innerHTML = "<div class='handle'><span class='ui-icon ui-icon-carat-2-n-s'></span></div>";

                /*if ($j('#' + SELECTED_STAMPS[i]).hasClass('ui-selected'))
                    var is_selected = "1";
                else
                    var is_selected = "0";
                */
                var is_selected = "0";

                JSONstring.push({'node':$j("#" + SELECTED_STAMPS[i]).attr('node'),
                                 'panel':$j("#" + SELECTED_STAMPS[i]).attr('panelScreenNumber'),
                                 'id':$j("#" + SELECTED_STAMPS[i]).attr('Subject_ID'),
                                 "selected": is_selected});
            }

            $j.ajax({
                type: "POST",
                url: SERVER + "/update",
                data: "reorder=" + encodeURIComponent(JSON.stringify(JSONstring))
            });

            if (SAVE_WORKFLOW == 1)
            {
                minicave.prototype._createWorkflowHistory();
            }
            //ajaxPost(SERVER + "/update", "minicaveReorder=" + encodeURIComponent(JSON.stringify(JSONstring)));
        },

        // Re-orders the text list when dragging of image stops (stop handler in _init)
        // TODO: update code to make use of "node" property
        _reorder: function()
        {
            var afterSort_ids = this._screensUl.sortable('toArray');
            var afterSort = this._screensUl.sortable('toArray', {attribute:'Subject_ID'});
            data = [];

            var firstFound = -1;
            var lastFound = -1;
            var JSONstring = [];

            // Could be cleaner, but should be fast. Should not repeat myself...
            for (var i = 0; i< afterSort.length; i++)
            {
                if ($j('#' + afterSort_ids[i]).hasClass('ui-selected'))
                    var is_selected = "1";
                else
                    var is_selected = "0";

                //console.log(BEFORESORT[i] + ' ' + afterSort[i]);
                if (firstFound == -1)
                {
                    if (BEFORESORT[i] != afterSort[i])
                    {
                        if (firstFound == -1)
                            firstFound = 1
                        //console.log("BEFORE node: " + $j('#' + afterSort_ids[i]).attr('node'));
                        //console.log("panelScreenNumber: " + $j('#' + afterSort_ids[i]).attr('panelScreenNumber'));
                        JSONstring.push({'node':$j('#' + afterSort_ids[i]).attr('node'),
                                         'panel':$j('#' + afterSort_ids[i]).attr('panelScreenNumber'),
                                         'id':$j('#' + afterSort_ids[i]).attr('Subject_ID'),
                                         'selected': is_selected});
                    }
                }
                else
                {
                    if (lastFound == -1)
                    {
                        if (BEFORESORT[i] != afterSort[i])
                        {
                            //console.log("AFTER node: " + $j('#' + afterSort_ids[i]).attr('node'));
                            //console.log("panelScreenNumber: " + $j('#' + afterSort_ids[i]).attr('panelScreenNumber'));

                            JSONstring.push({'node':$j('#' + afterSort_ids[i]).attr('node'),
                                             'panel':$j('#' + afterSort_ids[i]).attr('panelScreenNumber'),
                                             'id':$j('#' + afterSort_ids[i]).attr('Subject_ID'),
                                             'selected': is_selected});
                        }
                        else
                            lastFound = 1;
                    }
                    else
                        break;
                }
            }

            //console.log(JSONstring);

            if (JSONstring.length != 0)
            {
                $j.ajax({
                    type: "POST",
                    url: SERVER + "/update",
                    data: "reorder="+ encodeURIComponent(JSON.stringify(JSONstring))
                });

                if (SAVE_WORKFLOW == 1)
                {
                    minicave.prototype._createWorkflowHistory();
                }
            }

            // Update BEFORESORT for next round.
            for (i=0; i<afterSort.length; i++)
                BEFORESORT[i] = afterSort[i];
        },

        _updateNodes: function()
        {
            // Updates node name and panelScreenNumber
            var node = 0;
            var panelScreenNumber = 0;
            var i=0;

            $j('ul.screens li').each(function(i)
            {
                //nodeName = 'n' + ("0" + Number(node)).slice(-2);

                // NOTE: the use of both Subject_ID and panelScreenNumber is somewhat redundant.
                $j(this).attr('node', NODES[node].node_id);
                $j(this).attr('panelScreenNumber', NODES[node].panels[panelScreenNumber].panel_id);
                $j(this).attr('id', 'screens-li-' + i);

                if (!((i+1) % N_PER_NODE))
                    node += 1;

                if (((i+1)%N_PER_NODE))
                    panelScreenNumber += 1;
                else
                    panelScreenNumber = 0;

                i++;
            });

        },

        // List all objects from dataset (json file)
        _createChoices: function()
        {
            // Add columns dynamically based on 'data' columns
            var name = [];
            name.push(this._createHeader(Object.keys(DATA[0])));
            $j("#thHead").html(name.join(''));

            // Create table rows with data
            name = [];
            for (var i = 0; i < DATA.length; i++)
            {
                name.push(this._createList(DATA[i], i));
            }
            // Fill table with data
            $j("#choicesTableBody").html(name.join(''));

            // Make table sortable
            $j("#choicesTable").tablesorter({headers:{0:{sorter:false}}});
        },

        _createHeader: function(keys, i)
        {
            // Checkbox
            var table_header = '<th class="th_checkbox"><input type="checkbox" id="selectAll" /></th>';

            for (var i=0; i<keys.length; i++)
            {
                if (!COLUMNS_TO_IGNORE.includes(i))
                {
                    table_header += '<th class="th_sortable th_' + keys[i] + '">' + keys[i] + '</th>';
                }
            }

            return table_header;
        },

        _createList: function(panel, i)
        {
            //console.log(panel);

            var table_line = '<tr class="tr_body" id="table_line_'+i+'">';
            table_line += '<td class="td_checkbox"><input type="checkbox" class="inputChkbox" id="chkbox_'+i+'" /></td>';

            console.log("panel: " + panel);

            var key = "";

            for (var j=0; j<Object.keys(panel).length; j++)
            {
                if (!COLUMNS_TO_IGNORE.includes(j))
                {
                    key = Object.keys(panel)[j];
                    table_line += '<td class="td_'+ key.toLowerCase(key) + ' td_sortable">' + panel[key] + '</td>';
                }
            }

            table_line += '</tr>';
            return table_line;
        },

        /* Serialized workflow (history) */

        // List all objects from dataset (json file)
        _createWorkflowHistory: function()
        {
            $j.getJSON(WORKFLOW_FILE, function(history_data) {
                // Create table rows with data
                HISTORY_DATA = history_data;
                var name = [];


                for (var i = history_data.last_id-1; i >= 0 ; i--)
                {
                    name.push(minicave.prototype._createHistoryList(history_data.records[i], i));
                }
                // Fill table with data
                $j("#historyTableBody").html(name.join(''));
            });
        },

        _createHistoryList: function(workflow_item, i)
        {
            //console.log(panel);

            var table_line = '<tr class="tr_body" id="table_line_'+i+'">';
            //table_line += '<td class="td_checkbox"><input type="checkbox" class="inputChkbox" id="chkbox_'+i+'" /></td>';

            table_line += '<td class="td_history_id td_sortable">' + i + '</td>'
            table_line += '<td class="td_history_event td_sortable">' + workflow_item.descriptor + '</td>'
            table_line += '<td class="td_sortable"><button class="btn_workflow"  onclick="MINICAVE.prototype._loadState(' + i + ')">reload</button></td>'

            table_line += '</tr>'
            return table_line;
        },

        _loadState: function(i)
        {
            var JSONstring = [];
            JSONstring.push({'id':i});

            if (HISTORY_DATA.records[i].requires_json_back == 0)
            {
                $j.ajax({
                    type: "POST",
                    url: SERVER + "/update",
                    data: "load_state="+ encodeURIComponent(JSON.stringify(JSONstring))
                });
            }
            else
            {
                $j.ajax({
                    type: "POST",
                    url: SERVER + "/update",
                    data: "load_state="+ encodeURIComponent(JSON.stringify(JSONstring)),
                    success: function(json){
                        if (HISTORY_DATA.records[i].descriptor == "histogram")
                        {
                            minicave.prototype._histogram(json, 'histogram');
                        }
                        if (HISTORY_DATA.records[i].descriptor == "ntracks")
                        {
                            minicave.prototype._ntracks(json);
                        }
                    }
                });
            }

            if (HISTORY_DATA.records[i].descriptor == "data")
            {
                HISTORY_DATA.records[i].state.data.sharevol.volume.rotate = HISTORY_DATA.records[i].state.data.sharevol.volume.rotate_quat4;
                resetFromData(HISTORY_DATA.records[i].state.data.sharevol, false);

                // General properties
                /*
                props.url = HISTORY_DATA.records[i].state.data.sharevol.url;
                props.res = HISTORY_DATA.records[i].state.data.sharevol.res;
                props.scale = HISTORY_DATA.records[i].state.data.sharevol.scale;
                */

                if (APP_TYPE != 'astro_fits')
                {
                    for (var key in HISTORY_DATA.records[i].state.data.imagehd)
                        imagehd[key] = HISTORY_DATA.records[i].state.data.imagehd[key];
                }
                else
                {
                    for (var key in HISTORY_DATA.records[i].state.data.astro)
                        astro[key] = HISTORY_DATA.records[i].state.data.astro[key];
                }

                for (var i in gui.__controllers) {
                    gui.__controllers[i].updateDisplay();
                }

                /*console.log(gui.__folders);
                console.log("");
                var key = Object.keys(gui.__folders)[0];
                console.log(key + " \n");
                console.log(gui.__folders[key]);
                */
                for (var i = 0; i < Object.keys(gui.__folders).length; i++) {
                    var key1 = Object.keys(gui.__folders)[i];
                    for (var j = 0; j < gui.__folders[key1].__controllers.length; j++ )
                    {
                        gui.__folders[key1].__controllers[j].updateDisplay();
                    }
                }
            }

            minicave.prototype._createPanels();

            if (SAVE_WORKFLOW == 1)
            {
                minicave.prototype._createWorkflowHistory();
            }
        },

        /**/

        _createPanels: function()
        {
            var stamps  = [];
            var JSONstring = [];
            JSONstring.push({'creating':'panels'})
            $j.ajax({
                type: "POST",
                url: SERVER + "/update",
                data: "state="+ encodeURIComponent(JSON.stringify(JSONstring)),
                success: function(state_json)
                {
                    if (typeof state_json === 'string')
                        state_json = jQuery.parseJSON(state_json);

                    STATUS = state_json.status;
                    TOTAL_SCREENS = state_json.n_nodes*state_json.n_per_node;
                    N_NODES = state_json.n_nodes;
                    N_PER_NODE = state_json.n_per_node;
                    NODES = state_json.nodes;
                    SAVE_WORKFLOW = state_json.save_workflow;
                    APP_TYPE = state_json.app_type;
                    COLUMNS_TO_IGNORE = state_json.columns_to_ignore;

                    if (SAVE_WORKFLOW == 1)
                    {
                        WORKFLOW_FILE = state_json.workflow_data_dir + state_json.json_filename;
                    }

                    // Needs to update volume and menu...
                    this.volume = state_json.volume;

                    //console.log(state_json);

                    /*
                    STATUS = state_json.status;
                    console.log("STATUS:" + STATUS);

                    if (STATUS)
                    {
                        $j("#stopEncubePR").text = "Stop encube-PR";

                        $j("#stopEncubePR").prop('disabled', false);
                    }
                    else
                    {
                        $j("startstopEncubePR").text = "Stop encube-PR";
                        $j("#stopEncubePR").prop('disabled', true);
                    }
                    */

                    cpt = 0;
                    for (var i = 0; i < state_json.n_nodes; i++)
                    {
                        for (var j=0; j < state_json.n_per_node; j++)
                        {
                            // Associating the file_id to the corresponding panel of the current node.
                            stamps.push('<li id="screens-li-' + cpt + '" '+
                                            'node="'+ state_json.nodes[i].node_id +'" '+
                                            'panelScreenNumber="'+ state_json.nodes[i].panels[j].panel_id +'" '+
                                            'Subject_ID="'+state_json.nodes[i].panels[j].file_id+'"></li>');
                            BEFORESORT[cpt] = state_json.nodes[i].panels[j].file_id;
                            cpt++;
                        }
                    }

                    $j('#screensUl').html(stamps.join(''));

                    for (var i = 0; i < state_json.n_nodes; i++)
                    {
                        for (var j=0; j < state_json.n_per_node; j++)
                        {
                            var current_li = $j("li[node='"+ state_json.nodes[i].node_id +"'][panelscreennumber$='"+ j +"']");
                            // Add file_id text for user to see and handle.
                            if (state_json.nodes[i].panels[j].file_id != '-1')
                            {
                                current_li.html(
                                    state_json.nodes[i].panels[j].file_id);
                            }

                            current_li.addClass('ui-droppable');
                            current_li.addClass("ui-corner-all")
                            current_li.prepend(
                                "<div class='handle'><span class='ui-icon ui-icon-carat-2-n-s'></span></div>")

                            if (state_json.nodes[i].panels[j].selection_state == 1)
                            {
                                current_li.addClass(
                                    'ui-selected');
                                SELECTED_STAMPS.push(
                                    current_li.attr('id'));
                            }
                        }
                    }

                    $j("#screensUl li").droppable({
                        accept: "#choicesTable tbody tr",
                        drop: function (event, ui) {
                            //var dropped = ui.draggable.find(".td_subject_id").text();

                            //console.log('reached droppable.');

                            if ($j(this).hasClass('ui-droppable')){
                                var txt = "<div class='handle'><span class='ui-icon ui-icon-carat-2-n-s'></span></div>" +
                                    ui.draggable.find(".td_subject_id").text();

                                if (this.innerHTML != txt)
                                {
                                    if ($j(this).hasClass('ui-selected'))
                                        var is_selected = "1";
                                    else
                                        var is_selected = "0";

                                    // NOTE: there is a difference between this and $j(this). It is done on purpose.
                                    this.innerHTML = txt;
                                    $j(this).attr('Subject_ID', ui.draggable.find(".td_subject_id").text());

                                    var afterSort = $j('#screensUl').sortable('toArray', {attribute:'Subject_ID'});
                                    for (i=0; i<afterSort.length; i++)
                                        BEFORESORT[i] = afterSort[i];

                                    var JSONstring = [];

                                    JSONstring.push({'node':$j(this).attr('node'),
                                                     'panel':$j(this).attr('panelScreenNumber'),
                                                     'id':ui.draggable.find(".td_subject_id").text(),
                                                     'selected': is_selected});

                                    $j.ajax({
                                        type: "POST",
                                        url: SERVER + "/update",
                                        data: "loadOne="+ encodeURIComponent(JSON.stringify(JSONstring))
                                    });
                                    if (SAVE_WORKFLOW == 1)
                                    {
                                        minicave.prototype._createWorkflowHistory();
                                    }
                                }
                            }
                        }

                        /*if (state_json.history_ids.length > 0)
                        {
                            // fill history items.
                        }*/
                    });


                    /*
                    <select id="request_from_host" class="leftButtons select">

                      <option value="histogram">histogram</option>
                      <option value="area">area</option>
                      <option value="moment">moment</option>
                      <option value="ntracks">ntracks vs age</option>
                    </select>
                    */
                    if (APP_TYPE != 'astro_fits')
                    {
                        $j("#request_from_host").append($j('<option>',
                            {
                                value: "ntracks",
                                text: "ntracks vs age",
                            }
                        ));
                    }
                    else
                    {
                        // this eventually, this should be determined with the config file.
                        $j("#request_from_host").append($j('<option>',
                            {
                                value: "histogram",
                                text: "histogram",
                            }
                        ));

                        $j("#request_from_host").append($j('<option>',
                            {
                                value: "area",
                                text: "area",
                            }
                        ));

                        $j("#request_from_host").append($j('<option>',
                            {
                                value: "moment",
                                text: "moment",
                            }
                        ));
                    }

                    /*
                    this._screensUl
                        .find("li")
                        //.addClass("ui-corner-all ui-disabled")
                        .addClass("ui-corner-all")
                        //.removeClass("ui-droppable")
                        .prepend("<div class='handle'><span class='ui-icon ui-icon-carat-2-n-s'></span></div>")
                    */

                    /*******************************************************************************
                    * #choicesTable : Table list of different choices that can be added to a screen
                    *
                    *   What is being done here:
                    *       .draggable : jQuery function that lets drag and drop to a screen
                    *******************************************************************************/
                    var selectedChoices = [];

                    var lastPlace;
                    $j("#choicesTable tbody tr").draggable(
                    {
                        //revert: false,
                        helper: function(ev, ui)
                        {
                            return '<div>' + $j(this).find(".td_subject_id").text() + '</div>';
                        },
                        cursorAt: { top: 0, left: 0 },
                        //zIndex: 10,
                        snap: "#screensUl li .ui-droppable",
                        snapMode: "inner",
                        snapTolerance: 40,

                        start: function (event, ui)
                        {
                            lastPlace = $j(this).parent();
                        }
                    });

                    /*if (SAVE_WORKFLOW == 1)
                    {
                        minicave.prototype._createWorkflowHistory();
                    }*/

                    minicave.prototype._createChoices();

                    $j(':checkbox[id=selectAll]').click(function(){
                        $j(':checkbox[class=inputChkbox]').prop('checked', this.checked);
                    });

                },
                error: function()
                {
                    console.log("Nothing returned on load: s2plot not online.")
                }
            });
            //this._screensUl.html(stamps.join(''));
        }

    // END OF minicave.PROTOTYPE
    };

    MINICAVE = minicave;

    var take_snapshot = function()
    {
        /* To take a 'snapshot' of the browser's state */
        html2canvas(document.body, {
          onrendered: function(canvas) {
            $j("#scroll-content").appendChild(
                  '<div class="scroll-content-item ui-widget-header">'+ canvas + '</div>'
                );
            //var img = canvas.toDataURL();
          }
        });
    };


    /* ********** */

    /*
       ---- MAIN OBJECT: data
        new minicave(someDataFromJSON); --> initialises a variable called "data"
       ----
    */

    //new minicave(things_data);
    //new minicave(sami_data);
    new minicave(json_data);
});
