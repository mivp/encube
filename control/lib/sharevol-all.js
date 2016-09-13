/* @preserve
 *
 * encube: meta-controller
 * Miniature CAVE2 - CAVE2's screens representation for global data manipulation
 *
 * Copyright (c) 2015, Dany Vohl. All rights reserved.
 * Author: Dany Vohl - dvohl [at] astro [dot] swin [dot] edu
 *
 * Licensed under the GNU Lesser General Public License
 * https://www.gnu.org/licenses/lgpl.html
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
            $j("#divMoment").hide();

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
                    $j("#divMoment").hide();
                    $j("#divHistory").hide();
                    $j("#divTag").hide();
                }

                if ($j("#toggle_history_request").val() == 'moment') {
                    $j("#divSVG").hide();
                    $j("#divMoment").show();
                    $j("#divHistory").hide();//.toggle("slow");
                    $j("#divTag").hide();
                }

                if ($j("#toggle_history_request").val() == 'history') {
                    $j("#divSVG").hide();
                    $j("#divMoment").hide();
                    $j("#divHistory").show();//.toggle("slow");
                    $j("#divTag").hide();
                }

                if ($j("#toggle_history_request").val() == 'tags') {
                    $j("#divSVG").hide();
                    $j("#divMoment").hide();
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
/** @preserve
 * ShareVol
 * Lightweight WebGL volume viewer/slicer
 *
 * Copyright (c) 2014, Monash University. All rights reserved.
 * Author: Owen Kaluza - owen.kaluza ( at ) monash.edu
 *
 * Licensed under the GNU Lesser General Public License
 * https://www.gnu.org/licenses/lgpl.html
 *
 */
//TODO: colourmaps per slicer/volume not shared (global shared list of selectable maps?)
var volume;
var slicer;
var colours;
//Windows...
var info, colourmaps;
var props = {};
var reset;
var filename;
var mobile;
var gui;

function initPage() {
  window.onresize = autoResize;

  //Create tool windows
  info = new Toolbox("info");
  info.show();
  colourmaps = new Toolbox("colourmap", 400, 200);

  //Yes it's user agent sniffing, but we need to attempt to detect mobile devices so we don't over-stress their gpu...
  mobile = (screen.width <= 760 || /Android|webOS|iPhone|iPad|iPod|BlackBerry/i.test(navigator.userAgent));

  //Colour editing and palette management
  colours = new GradientEditor($('palette'), updateColourmap);

  //Load json data?
  var json = getSearchVariable("data");
  console.log(json);

  if (!json) {
    //Saved settings for this image...
    filename = getSearchVariable("img");

    if (filename) {
      loadStoredData(filename);
      //Use props from url or defaults
      props = {
        "url" : decodeURI(filename),
        "res" : [getSearchVariable("nx", 256), getSearchVariable("ny", 256), getSearchVariable("nz", 256)],
        "scale" : [getSearchVariable("dx", 1), getSearchVariable("dy", 1), getSearchVariable("dz", 1)]
      }

      if (props["url"]) loadTexture();
    } else {
      //Attempt to load default.json
      json = "default.json";
    }
  }

  //Try and load json parameters file (no cache)
  if (json) {
    $('status').innerHTML = "Loading params...";
    ajaxReadFile(decodeURI(json), loadData, true);
  }

  //ImageHD
  ajaxReadFile("masklist.txt", loadImageHD, true);
}

var imagehd = {"rotate" : true, "labels" : false, "trackSamples" : 0.001, "trackThickness" : 2, "trackOpacity" : 0.5};
var astro = {"show": false, "mip":false, "moment": false};
var sort = {"sort1" : 0, "sort2" : 0};
var region = {
               "region1" : -1, "region2" : -1,
               "r1alpha" : 1.0, "r2alpha" : 1.0, 
               "r1colour" : [214, 188, 86], "r2colour" : [214, 188, 86]
              };
var regions = {"None" : -1};
var sortfields = {"Type" : 0,"Subject_ID" : 1,"Age" : 2,"UHDRS" : 3,"DBS" : 4,"SDMT" : 5,"STAP" : 6};
var minicave = {"showMinicave": false};
var virtual_observatory = {"show": false};

function loadImageHD(data) {
  //Parse mask list
  var newline = /\r\n?|\n/;
  var lines = data.split(newline);
  for (var l in lines) {
    var line = lines[l];
    if (line.charAt(0) == '#') continue;
    var id = parseInt(line);
    if (id > 0) {
      var label = line.substr(line.indexOf("\"")+1);
      var len = label.length-1;
      if (len > 28) len = 28;
      label = label.substr(0, len);
      regions[label] = id;
    }
  }
  //console.log(JSON.stringify(regions, null, 2));
}

var server;
var fi, fs, fr;

function addImageHDGUI() {
    //Set the server location
    server = "http://" + window.location.host

    //Custom UI for ImageHD
    fi = gui.addFolder('ImageHD');
    fs = gui.addFolder('Sort');
    fr = gui.addFolder('Regions');

    fi.add(imagehd, 'labels');
    fi.add(imagehd, 'trackSamples', 0.001, 1.0, 0.01);
    fi.add(imagehd, 'trackThickness', 0.1, 10.0, 0.1);
    fi.add(imagehd, 'trackOpacity', 0.1, 1.0, 0.01);
    fi.open();

    fs.add(sort, 'sort1', sortfields);
    fs.add(sort, 'sort2', sortfields);
    fs.add({"Update" : updateServerSort}, 'Update');
    fs.open();

    fr.add(region, 'region1', regions);
    fr.add(region, 'region2', regions);
    fr.add(region, 'r1alpha', 0.0, 1.0, 0.01);
    fr.add(region, 'r2alpha', 0.0, 1.0, 0.01);
    fr.addColor(region, 'r1colour');
    fr.addColor(region, 'r2colour');
    fr.add({"Update" : updateServerRegion}, 'Update');
    fr.open();


    // Iterate over all controllers and set change function
    for (var i in fi.__controllers)
        fi.__controllers[i].onChange(updateServerOther);

    for (var i in fs.__controllers)
        fs.__controllers[i].onChange(updateServerSort);

    for (var i in fr.__controllers)
        fr.__controllers[i].onChange(updateServerRegion);
}

function addAstroGUI()
{
    // Let's forget about the following for now!

    fa = gui.addFolder('Moment maps');
    fa.add(astro, 'show');
    fa.add(astro, 'mip');
    fa.add(astro, 'moment', [0,1]);
    fa.open();

    for (var i in fa.__controllers)
        fa.__controllers[i].onChange(updateServerOtherAstro);
}

var serverData = null;
function updateServer() {
  var temp = volume.properties.samples;
  if (volume.properties.samples < 256) volume.properties.samples = 256; //Override sample rate
  data = getData(true, true);
  if (serverData != data) {
    serverData = data;
    ajaxPost(server + "/update", "data=" + encodeURIComponent(data));
  }
  volume.properties.samples = temp;

    // SAVE_WORKFLOW declared in minicave.js
    if (SAVE_WORKFLOW == 1)
    {
        MINICAVE.prototype._createWorkflowHistory();
    }
}

function updateServerOther() {
  ajaxPost(server + "/update", "data=" + encodeURIComponent(JSON.stringify(imagehd)));

    // SAVE_WORKFLOW declared in minicave.js
    if (SAVE_WORKFLOW == 1)
    {
        MINICAVE.prototype._createWorkflowHistory();
    }
  return false;
}

function updateServerOtherAstro() {
  // Reset to be face on. Always... Should block rotation.
  console.log('in updateServerOtherAstro');

  //resetFromData(reset, true);

  ajaxPost(server + "/update", "moment=" + encodeURIComponent(JSON.stringify(astro)));

    // SAVE_WORKFLOW declared in minicave.js
    if (SAVE_WORKFLOW == 1)
    {
        MINICAVE.prototype._createWorkflowHistory();
    }
  return false;
}

function updateServerSort() {
  ajaxPost(server + "/update", "sort=" + encodeURIComponent(JSON.stringify(sort)));
  try
  {
    // in minicave.js
    console.log("in updateServerSort()");
    syncScreens();
  }
  catch(err)
  {
    console.log(err.message);
  }

    // SAVE_WORKFLOW declared in minicave.js
    if (SAVE_WORKFLOW == 1)
    {
        MINICAVE.prototype._createWorkflowHistory();
    }
  return false;
}

function updateServerRegion() {
  ajaxPost(server + "/update", "region=" + encodeURIComponent(JSON.stringify(region)));

    // SAVE_WORKFLOW declared in minicave.js
    if (SAVE_WORKFLOW == 1)
    {
        MINICAVE.prototype._createWorkflowHistory();
    }

  return false;
}

function loadStoredData(key) {
  return; //No local storage (this is buggy anyway)
  // if (localStorage[key]) {
  //   try {
  //     var parsed = JSON.parse(localStorage[key]);
  //     props = parsed;
  //   } catch (e) {
  //     //if erroneous data in local storage, delete
  //     //console.log("parse error: " + e.message);
  //     alert("parse error: " + e.message);
  //     localStorage[key] = null;
  //   }
  // }
}

function toggleMinicave()
{
    var e = document.getElementById("divMinicave");
    if(e.style.display == 'block')
        e.style.display = 'none';
    else
        e.style.display = 'block';
}

function toggleAutospin(value)
{
    var JSONstring = [];
        JSONstring.push({'autospin':value});

        $j.ajax({
            type: "POST",
            url: SERVER + "/update",
            data: "autospin="+ encodeURIComponent(JSON.stringify(JSONstring))});
}

function zoomIn()
{
    var JSONstring = [];
        JSONstring.push({'dummy':"data"});

        $j.ajax({
            type: "POST",
            url: SERVER + "/update",
            data: "zoomIn="+ encodeURIComponent(JSON.stringify(JSONstring))});
}

function zoomOut()
{
    var JSONstring = [];
        JSONstring.push({'dummy':"data"});

        $j.ajax({
            type: "POST",
            url: SERVER + "/update",
            data: "zoomOut="+ encodeURIComponent(JSON.stringify(JSONstring))});
}

function showStats()
{
    var JSONstring = [];
        JSONstring.push({'dummy':"data"});

        $j.ajax({
            type: "POST",
            url: SERVER + "/update",
            data: "showStats="+ encodeURIComponent(JSON.stringify(JSONstring))});
}

function StopEncubePR() {
    // This could be done in a div to be a bit nicer.
    if (confirm('Are you sure you want to stop encube-PR?')) {
        var JSONstring = [];
        JSONstring.push({'node':$j(".ui-selected").attr('node'),
                         'panel':$j(".ui-selected").attr('panelScreenNumber')});

        $j.ajax({
            type: "POST",
            url: SERVER + "/update",
            data: "stopEncubePR="+ encodeURIComponent(JSON.stringify(JSONstring))});
    }
}

/*function toggleAladinLite()
{
    var e = document.getElementById("aladin-lite-div");
    if(e.style.display == 'block')
        e.style.display = 'none';
    else
        e.style.display = 'block';
}*/

function loadData(src, fn) {
  props = JSON.parse(src);
  reset = props; //Store orig for reset
  //Storage reset?
  if (getSearchVariable("reset")) {localStorage.removeItem(fn); console.log("Storage cleared");}
  //Load any stored presets for this file
  filename = fn;
  loadStoredData(fn);

  //Setup default props from original data...
  props.url = reset.url;
  props.res = reset.res || [256, 256, 256];
  props.scale = reset.scale || [1.0, 1.0, 1.0];

  //Load the image
  loadTexture();
}

function loadDataFromJson(src) {
  props = JSON.parse(src);
  reset = props; //Store orig for reset
  //Storage reset?
  if (getSearchVariable("reset")) {localStorage.removeItem(fn); console.log("Storage cleared");}
  //Load any stored presets for this file
  filename = fn;
  loadStoredData(fn);

  //Setup default props from original data...
  props.url = reset.url;
  props.res = reset.res || [256, 256, 256];
  props.scale = reset.scale || [1.0, 1.0, 1.0];

  //Load the image
  loadTexture();
}

function saveData() {
  try {
    localStorage[filename] = getData();
  } catch(e) {
    //data wasn’t successfully saved due to quota exceed so throw an error
    console.log('LocalStorage Error: Quota exceeded? ' + e);
  }
}

function getData(compact, matrix) {
  var data = {};
  data.url = props.url;
  data.res = props.res;
  data.scale = props.scale;
  if (volume) data.volume = volume.get(matrix);
  if (slicer) data.slicer = slicer.get();

  //Return compact json string
  if (compact) return JSON.stringify(data);
  //Otherwise return indented json string
  return JSON.stringify(data, null, 2);
}

function exportData() {
  window.open('data:text/json;base64,' + window.btoa(getData()));
}

function resetFromData(src, send_to_server) {
  //Restore data from saved props
  if (src.volume && volume) {
    volume.load(src.volume);
    volume.draw(send_to_server);
  }
  if (src.slicer && slicer) {
    slicer.load(src.slicer);
    slicer.draw();
  }
}

function loadTexture() {
  $('status').innerHTML = "Loading image data... ";
  var image;

  loadImage(props["url"], function () {
    image = new Image();

    var headers = request.getAllResponseHeaders();
    var match = headers.match( /^Content-Type\:\s*(.*?)$/mi );
    var mimeType = match[1] || 'image/png';
    var blob = new Blob([request.response], {type: mimeType} );
    image.src =  window.URL.createObjectURL(blob);
    var imageElement = document.createElement("img");

    image.onload = function () {
      console.log("Loaded image: " + image.width + " x " + image.height);
      imageLoaded(image);
    }
  }
  );
}

function imageLoaded(image) {
  //Create the slicer
  if (props.slicer) {
    if (mobile) props.slicer.show = false; //Start hidden on small screen
    slicer = new Slicer(props, image, "linear");
  }

  //Create the volume viewer
  if (props.volume) {
    volume = new Volume(props, image, mobile);
    volume.slicer = slicer; //For axis position
    volume.samples = 64; //ImageHD (reduce load on tablet)
  }

  //Volume draw on mouseup to apply changes from other controls (including slicer)
  document.addEventListener("mouseup", function(ev) {if (volume) volume.delayedRender(250, true);}, false);
  document.addEventListener("wheel", function(ev) {if (volume) volume.delayedRender(250, true);}, false);

  //Update colours (and draw objects)
  updateColourmap();

  info.hide();  //Status

  /*/Draw speed test
  frames = 0;
  testtime = new Date().getTime();
  info.show();
  volume.draw(false, true);*/

  if (!props.nogui) {
    gui = new dat.GUI({"width": 400 });
    gui.add({"Reset" : function() {resetFromData(reset, true);}}, 'Reset');
    gui.add({"Stop encube-PR" : function() {StopEncubePR();}}, 'Stop encube-PR');

    //gui.add({"Restore" : function() {resetFromData(props);}}, 'Restore');
    //gui.add({"Export" : function() {exportData();}}, 'Export');
    //gui.add({"loadFile" : function() {document.getElementById('fileupload').click();}}, 'loadFile'). name('Load Image file');

    //Data options for ImageHD

    gui.add(imagehd, 'rotate').onChange(function(value) {volume.flipbuttons = !value;});
    var m_autospin = gui.addFolder('S2PLOT options');
    m_autospin.add({"Autospin": false},"Autospin").onChange(function(value) {toggleAutospin(value);});
    m_autospin.add({"Zoom in" : function() {zoomIn();}}, 'Zoom in');
    m_autospin.add({"Zoom out" : function() {zoomOut();}}, 'Zoom out');
    m_autospin.add({"Show stats" : function() {showStats();}}, 'Show stats');
    m_autospin.open();

    var fm = gui.addFolder('Miniature CAVE2');
    fm.add(minicave,"showMinicave").onChange(toggleMinicave);
    //fm.add(minicave,"showAladinLite").onChange(toggleAladinLite);
    fm.open();

    if (APP_TYPE != "astro_fits")
        addImageHDGUI();
    else
    {
        gui.add({"ColourMaps" : function() {window.colourmaps.toggle();}}, 'ColourMaps');
        addAstroGUI();
    }

    if (volume) volume.addGUI(gui);
    if (slicer) slicer.addGUI(gui);
  }

  //Save props on exit
  window.onbeforeunload = saveData;
}

/////////////////////////////////////////////////////////////////////////
//File upload handling
function fileSelected(files) {
  filesProcess(files);
}
function filesProcess(files, callback) {
  window.URL = window.webkitURL || window.URL; // Vendor prefixed in Chrome.
  for (var i = 0; i < files.length; i++) {
    var file = files[i];
    props["url"] = window.URL.createObjectURL(file);
    loadTexture();
  }
}

function autoResize() {
  if (volume) {
    volume.width = 0; //volume.canvas.width = window.innerWidth;
    volume.height = 0; //volume.canvas.height = window.innerHeight;
    volume.draw(true);
  }
}

function setColourMap(filename) {
  var data = readURL("colourmaps/" + filename);
  colours.read(data);
  updateColourmap();
}

function updateColourmap() {
  if (!colours) return;
  var gradient = $('gradient');
  colours.palette.draw(gradient, false);

  if (volume && volume.webgl) {
    // Get colourmap canvas url to ship to server.
    volume.properties.colourmap_url = gradient.toDataURL();

    volume.webgl.updateTexture(volume.webgl.gradientTexture, gradient, volume.gl.TEXTURE1);  //Use 2nd texture unit
    volume.applyBackground(colours.palette.background.html());
    //volume.applyBackground(colours.palette_plots.background.html());
    volume.draw(true);
  }

  if (slicer) {
    slicer.updateColourmap();
    slicer.draw();
  }
}

var request, progressBar;

    function loadImage(imageURI, callback)
    {
        request = new XMLHttpRequest();
        request.onloadstart = showProgressBar;
        request.onprogress = updateProgressBar;
        request.onload = callback;
        request.onloadend = hideProgressBar;
        request.open("GET", imageURI, true);
        request.responseType = 'arraybuffer';
        request.send(null);
    }
    
    function showProgressBar()
    {
        progressBar = document.createElement("progress");
        progressBar.value = 0;
        progressBar.max = 100;
        progressBar.removeAttribute("value");
        document.getElementById('status').appendChild(progressBar);
    }
    
    function updateProgressBar(e)
    {
        if (e.lengthComputable)
            progressBar.value = e.loaded / e.total * 100;
        else
            progressBar.removeAttribute("value");
    }
    
    function showImage()
    {
        var headers = request.getAllResponseHeaders();
        var match = headers.match( /^Content-Type\:\s*(.*?)$/mi );
        var mimeType = match[1] || 'image/png';
        var blob = new Blob([request.response], {type: mimeType} );
        var imageElement = document.createElement("img");
        imageElement.src = window.URL.createObjectURL(blob);
        document.body.appendChild(imageElement);
    }
    
    function hideProgressBar()
    {
      document.getElementById('status').removeChild(progressBar);
    }

/**
 * @constructor
 */
function Toolbox(id, x, y) {
  //Mouse processing:
  this.el = $(id);
  this.mouse = new Mouse(this.el, this);
  this.mouse.moveUpdate = true;
  this.el.mouse = this.mouse;
  this.style = $S(id);
  if (x && y) {
    this.style.left = x + 'px';
    this.style.top = y + 'px';
  } else {
    this.style.left = ((window.innerWidth - this.el.offsetWidth) * 0.5) + 'px';
    this.style.top = ((window.innerHeight - this.el.offsetHeight) * 0.5) + 'px';
  }
  this.drag = false;
}

Toolbox.prototype.toggle = function() {
  if (this.style.visibility == 'visible')
    this.hide();
  else
    this.show();
}

Toolbox.prototype.show = function() {
  this.style.visibility = 'visible';
}

Toolbox.prototype.hide = function() {
  this.style.visibility = 'hidden';
}

//Mouse event handling
Toolbox.prototype.click = function(e, mouse) {
  this.drag = false;
  return true;
}

Toolbox.prototype.down = function(e, mouse) {
  //Process left drag only
  this.drag = false;
  if (mouse.button == 0 && e.target.className.indexOf('scroll') < 0 && ['INPUT', 'SELECT', 'OPTION', 'RADIO'].indexOf(e.target.tagName) < 0)
    this.drag = true;
  return true;
}

Toolbox.prototype.move = function(e, mouse) {
  if (!mouse.isdown) return true;
  if (!this.drag) return true;

  //Drag position
  this.el.style.left = parseInt(this.el.style.left) + mouse.deltaX + 'px';
  this.el.style.top = parseInt(this.el.style.top) + mouse.deltaY + 'px';
  return false;
}

Toolbox.prototype.wheel = function(e, mouse) {
}
/*
 * ShareVol
 * Lightweight WebGL volume viewer/slicer
 *
 * Copyright (c) 2014, Monash University. All rights reserved.
 * Author: Owen Kaluza - owen.kaluza ( at ) monash.edu
 *
 * Licensed under the GNU Lesser General Public License
 * https://www.gnu.org/licenses/lgpl.html
 *
 */

  function Slicer(props, image, filter, parentEl) {
    this.image = image;
    this.res = props.res;
    this.dims = [props.res[0] * props.scale[0], props.res[1] * props.scale[1], props.res[2] * props.scale[2]];
    this.slices = [0.5, 0.5, 0.5];

    // Set properties
    this.properties = {};
    this.properties.show = true;
    this.properties.X = Math.round(this.res[0] / 2);
    this.properties.Y = Math.round(this.res[1] / 2);
    this.properties.Z = Math.round(this.res[2] / 2);
    this.properties.brightness = 0.0;
    this.properties.contrast = 1.0;
    this.properties.power = 1.0;
    this.properties.usecolourmap = false;
    this.properties.layout = "xyz";
    this.flipY = false;
    this.properties.zoom = 1.0;

    this.container = document.createElement("div");
    this.container.style.cssText = "position: absolute; bottom: 10px; left: 10px; margin: 0px; padding: 0px; pointer-events: none;";
    if (!parentEl) parentEl = document.body;
    parentEl.appendChild(this.container);

    //Load from local storage or previously loaded file
    if (props.slicer) this.load(props.slicer);

    this.canvas = document.createElement("canvas");
    this.canvas.style.cssText = "position: absolute; bottom: 0px; margin: 0px; padding: 0px; border: none; background: rgba(0,0,0,0); pointer-events: none;";

    this.doLayout();

    this.canvas.mouse = new Mouse(this.canvas, this);

    this.webgl = new WebGL(this.canvas);
    this.gl = this.webgl.gl;

    this.filter = this.gl.NEAREST; //Nearest-neighbour (default)
    if (filter == "linear") this.filter = this.gl.LINEAR;

    //Use the default buffers
    this.webgl.init2dBuffers(this.gl.TEXTURE2);

    //Compile the shaders
    this.program = new WebGLProgram(this.gl, 'texture-vs', 'texture-fs');
    if (this.program.errors) OK.debug(this.program.errors);
    this.program.setup(["aVertexPosition"], ["palette", "texture", "colourmap", "cont", "bright", "power", "slice", "dim", "res", "axis", "select"]);

    this.gl.clearColor(0, 0, 0, 0);
    this.gl.enable(this.gl.BLEND);
    this.gl.blendFunc(this.gl.SRC_ALPHA, this.gl.ONE_MINUS_SRC_ALPHA);
    this.gl.enable(this.gl.SCISSOR_TEST);

    //Load the textures
    this.loadImage(this.image);

    //Hidden?
    if (!this.properties.show) this.toggle();
  }

  Slicer.prototype.toggle = function() {
    if (this.container.style.visibility == 'hidden')
      this.container.style.visibility = 'visible';
    else
      this.container.style.visibility = 'hidden';
  }

  Slicer.prototype.addGUI = function(gui) {
    this.gui = gui;
    var that = this;
    //Add folder
    var f1 = this.gui.addFolder('Slices');
    f1.add(this.properties, 'show').onFinishChange(function(l) {that.toggle();});
    //["hide/show"] = function() {};
    f1.add(this.properties, 'layout').onFinishChange(function(l) {that.doLayout(); that.draw();});
    //f1.add(this.properties, 'X', 0, this.res[0], 1).listen();
    //f1.add(this.properties, 'Y', 0, this.res[1], 1).listen();
    //f1.add(this.properties, 'Z', 0, this.res[2], 1).listen();
    f1.add(this.properties, 'zoom', 0.01, 4.0, 0.1).onFinishChange(function(l) {that.doLayout(); that.draw();});

    f1.add(this.properties, 'brightness', -1.0, 1.0, 0.01);
    f1.add(this.properties, 'contrast', 0.0, 3.0, 0.01);
    f1.add(this.properties, 'power', 0.01, 5.0, 0.01);
    f1.add(this.properties, 'usecolourmap');
    f1.open();

    var changefn = function(value) {that.draw();};
    for (var i in f1.__controllers)
      f1.__controllers[i].onChange(changefn);
  }

  Slicer.prototype.get = function() {
    var data = {};
    //data.colourmap = colours.palette.toString();
    data.properties = this.properties;
    return data;
  }

  Slicer.prototype.load = function(src) {
    //colours.read(data.colourmap);
    //colours.update();
    for (var key in src.properties)
      this.properties[key] = src.properties[key]
  }

  Slicer.prototype.setX = function(val) {this.properties.X = val * this.res[0]; this.draw();}


  Slicer.prototype.setY = function(val) {this.properties.Y = val * this.res[1]; this.draw();}
  Slicer.prototype.setZ = function(val) {this.properties.Z = val * this.res[2]; this.draw();}

  Slicer.prototype.doLayout = function() {
    this.viewers = [];

    var x = 0;
    var y = 0;
    var xmax = 0;
    var ymax = 0;
    var rotate = 0;
    var alignTop = true;

    removeChildren(this.container);

    var that = this;
    var buffer = "";
    var rowHeight = 0, rowWidth = 0;
    var addViewer = function(idx) {
      var mag = 1.0;
      if (buffer) mag = parseFloat(buffer);
      var v = new SliceView(that, x, y, idx, rotate, mag);
      that.viewers.push(v);
      that.container.appendChild(v.div);

//      x += v.viewport.width + 5; //Offset by previous width
//      var h = v.viewport.height + 5;
//      if (h > rowHeight) rowHeight = h;
//      if (x > xmax) xmax = x;

      y += v.viewport.height + 5; //Offset by previous height
      var w = v.viewport.width + 5;
      if (w > rowWidth) rowWidth = w;
      if (y > ymax) ymax = y;
    }

    //Process based on layout
    this.flipY = false;
    for (var i=0; i<this.properties.layout.length; i++) {
      var c = this.properties.layout.charAt(i);
      rotate = 0;
      switch (c) {
        case 'X':
          rotate = 90;
        case 'x':
          addViewer(0);
          break;
        case 'Y':
          rotate = 90;
        case 'y':
          addViewer(1);
          break;
        case 'Z':
          rotate = 90;
        case 'z':
          addViewer(2);
          break;
        case '|':
//          x = 0;
//          y += rowHeight; //this.viewers[this.viewers.length-1].viewport.height + 5; //Offset by previous height
//          rowHeight = 0;

          y = 0;
          x += rowWidth;
          rowWidth = 0;
          break;
        case '_':
          this.flipY = true;
          break;
        case '-':
          alignTop = false;
          break;
        default:
          //Add other chars to buffer, if a number will be used as zoom
          buffer += c;
          continue;
      }
      //Clear buffer
      buffer = "";
    }

//    this.width = xmax;
//    this.height = y + rowHeight; //this.viewers[this.viewers.length-1].viewport.height;

    this.width = x + rowWidth;
    this.height = ymax;

    //Restore the main canvas
    this.container.appendChild(this.canvas);

    //Align to top or bottom?
    //console.log(this.height);
    //console.log(this.height + " : top? " + alignTop);
    if (alignTop) {
      this.container.style.bottom = "";
      this.container.style.top = (this.height + 10) + "px";
    } else {
      this.container.style.top = undefined;
      this.container.style.bottom = 10 + "px";
    }
  }

  Slicer.prototype.loadImage = function(image) {
    //Texture load
    for (var i=0; i<3; i++)
      this.webgl.loadTexture(image, this.filter);
    this.reset();
  }

  Slicer.prototype.reset = function() {
    this.dimx = this.image.width / this.res[0];
    this.dimy = this.image.height / this.res[1];
    //console.log(this.res[0] + "," + this.res[1] + "," + this.res[2] + " -- " + this.dimx + "x" + this.dimy);
  }

  Slicer.prototype.updateColourmap = function() {
    this.webgl.updateTexture(this.webgl.gradientTexture, $('gradient'), this.gl.TEXTURE2);  //Use 2nd texture unit
    this.draw();
  }

  Slicer.prototype.draw = function() {
    this.slices = [(this.properties.X-1)/(this.res[0]-1), 
                   (this.properties.Y-1)/(this.res[1]-1),
                   (this.properties.Z-1)/(this.res[2]-1)];

    if (this.width != this.canvas.width || this.height != this.canvas.height) {
      this.canvas.width = this.width;
      this.canvas.height = this.height;
      this.canvas.setAttribute("width", this.width);
      this.canvas.setAttribute("height", this.height);
      if (this.webgl) {
        this.gl.viewportWidth = this.width;
        this.gl.viewportHeight = this.height;
        this.webgl.viewport = new Viewport(0, 0, this.width, this.height);
      }
    }
    //console.log(this.gl.viewportWidth + " x " + this.gl.viewportHeight);
    //console.log(this.width + " x " + this.height);

    this.webgl.use(this.program);

    //Uniform variables
    this.gl.uniform4fv(this.program.uniforms["background"], colours.palette.colours[0].colour.rgbaGL());

    //Gradient texture
    this.gl.activeTexture(this.gl.TEXTURE0);
    this.gl.bindTexture(this.gl.TEXTURE_2D, this.webgl.gradientTexture);
    this.gl.uniform1i(this.program.uniforms["palette"], 0);

    //Options
    this.gl.uniform1i(this.program.uniforms["colourmap"], this.properties.usecolourmap);

    // brightness and contrast
    this.gl.uniform1f(this.program.uniforms["bright"], this.properties.brightness);
    this.gl.uniform1f(this.program.uniforms["cont"], this.properties.contrast);
    this.gl.uniform1f(this.program.uniforms["power"], this.properties.power);

    //Image texture
    this.gl.activeTexture(this.gl.TEXTURE1);
    this.gl.bindTexture(this.gl.TEXTURE_2D, this.webgl.textures[0]);
    this.gl.uniform1i(this.program.uniforms["texture"], 1);

    //Clear all
    this.gl.scissor(0, 0, this.width, this.height);
    this.gl.clear(this.gl.COLOR_BUFFER_BIT | this.gl.DEPTH_BUFFER_BIT);

    //Draw each slice viewport
    for (var i=0; i<this.viewers.length; i++)
      this.drawSlice(i);
  }

  Slicer.prototype.drawSlice = function(idx) {
    var view = this.viewers[idx];
    var vp = view.viewport;

    //Set selection crosshairs
    var sel;
    if (view.rotate == 90)
      sel = [1.0 - this.slices[view.j], this.slices[view.i]];
    else
      sel = [this.slices[view.i], this.slices[view.j]];
    
    //Swap y-coord
    if (!this.flipY) sel[1] = 1.0 - sel[1];

    this.webgl.viewport = vp;
    this.gl.scissor(vp.x, vp.y, vp.width, vp.height);
    //console.log(JSON.stringify(vp));

    //Apply translation to origin, any rotation and scaling (inverse of zoom factor)
    this.webgl.modelView.identity()
    this.webgl.modelView.translate([0.5, 0.5, 0])
    this.webgl.modelView.rotate(-view.rotate, [0, 0, 1]);

    //Apply zoom and flip Y
    var scale = [1.0/2.0, -1.0/2.0, -1.0];
    if (this.flipY) scale[1] = -scale[1];
    this.webgl.modelView.scale(scale);

    //Texturing
    //this.gl.uniform1i(this.program.uniforms["slice"], ));
    this.gl.uniform3f(this.program.uniforms['slice'], this.slices[0], this.slices[1], this.slices[2]);
    this.gl.uniform2f(this.program.uniforms["dim"], this.dimx, this.dimy);
    this.gl.uniform3i(this.program.uniforms["res"], this.res[0], this.res[1], this.res[2]);
    this.gl.uniform1i(this.program.uniforms["axis"], view.axis);
    //Convert [0,1] selection coords to pixel coords
    this.gl.uniform2i(this.program.uniforms["select"], vp.width * sel[0] + vp.x, vp.height * sel[1] + vp.y);

    this.webgl.initDraw2d();

    this.gl.enable(this.gl.BLEND);

    //Draw, single pass
    this.gl.blendFunc(this.gl.SRC_ALPHA, this.gl.ONE_MINUS_SRC_ALPHA);
    this.gl.drawArrays(this.gl.TRIANGLE_STRIP, 0, this.webgl.vertexPositionBuffer.numItems);
  }

  function SliceView(slicer, x, y, axis, rotate, magnify) {
    this.axis = axis;
    this.slicer = slicer;

    this.magnify = magnify || 1.0;
    this.origin = [0.5,0.5];
    this.rotate = rotate || 0;

    //Calc viewport
    this.i = 0;
    this.j = 1;
    if (axis == 0) this.i = 2;
    if (axis == 1) this.j = 2;

    var w = Math.round(slicer.dims[this.i] * slicer.properties.zoom * this.magnify);
    var h = Math.round(slicer.dims[this.j] * slicer.properties.zoom * this.magnify);

    if (this.rotate == 90)
      this.viewport = new Viewport(x, y, h, w);
    else
      this.viewport = new Viewport(x, y, w, h);
  
    //Border and mouse interaction element
    this.div = document.createElement("div");
    this.div.style.cssText = "padding: 0px; margin: 0px; outline: 2px solid rgba(64,64,64,0.5); position: absolute; display: inline-block; pointer-events: auto;";
    this.div.id = "slice-div-" + axis;

    this.div.style.left = x + "px";
    this.div.style.bottom = y + "px";
    this.div.style.width = this.viewport.width + "px";
    this.div.style.height = this.viewport.height + "px";

    this.div.mouse = new Mouse(this.div, this);
  }

  SliceView.prototype.click = function(event, mouse) {
    if (this.slicer.flipY) mouse.y = mouse.element.clientHeight - mouse.y;

    var coord;

    //Rotated?
    if (this.rotate == 90)
      coord = [mouse.y / mouse.element.clientHeight, 1.0 - mouse.x / mouse.element.clientWidth];
    else 
      coord = [mouse.x / mouse.element.clientWidth, mouse.y / mouse.element.clientHeight];

    var A = Math.round(this.slicer.res[this.i] * coord[0]);
    var B = Math.round(this.slicer.res[this.j] * coord[1]);

    if (this.axis == 0) {
      slicer.properties.Z = A;
      slicer.properties.Y = B;
    } else if (this.axis == 1) {
      slicer.properties.X = A;
      slicer.properties.Z = B;
    } else {
      slicer.properties.X = A;
      slicer.properties.Y = B;
    }

    this.slicer.draw();
  }

  SliceView.prototype.wheel = function(event, mouse) {
    if (this.axis == 0) slicer.properties.X += event.spin;
    if (this.axis == 1) slicer.properties.Y += event.spin;
    if (this.axis == 2) slicer.properties.Z += event.spin;
    this.slicer.draw();
  }

  SliceView.prototype.move = function(event, mouse) {
    if (mouse.isdown) this.click(event, mouse);
  }


/*
 * ShareVol
 * Lightweight WebGL volume viewer/slicer
 *
 * Copyright (c) 2014, Monash University. All rights reserved.
 * Author: Owen Kaluza - owen.kaluza ( at ) monash.edu
 *
 * Licensed under the GNU Lesser General Public License
 * https://www.gnu.org/licenses/lgpl.html
 *
 */
//BUGS:
//Canvas Y slightly too large, scroll bar appearing
//
//Improvements:
//Separate Opacity gradient
//Data min, data max - masked or clamped
//Timestepping
//Superimposed volumes

function Volume(props, image, mobile, parentEl) {
  this.image = image;
  this.canvas = document.createElement("canvas");
  this.canvas.style.cssText = "width: 100%; height: 100%; z-index: 0; margin: 0px; padding: 0px; background: black; border: none; display:block;";
  if (!parentEl) parentEl = document.body;
  parentEl.appendChild(this.canvas);

  //canvas event handling
  this.canvas.mouse = new Mouse(this.canvas, this);
  this.canvas.mouse.moveUpdate = true; //Continual update of deltaX/Y

  this.background = new Colour(0xff404040);
  this.borderColour = new Colour(0xffbbbbbb);

  this.width = this.height = 0; //Auto-size

  this.webgl = new WebGL(this.canvas);
  this.gl = this.webgl.gl;

  this.flipbuttons = false;
  this.rotating = false;
  this.translate = [0,0,4];
  this.rotate = quat4.create();
  quat4.identity(this.rotate);
  this.focus = [0,0,0];
  this.centre = [0,0,0];
  this.modelsize = 1;
  this.scale = [1, 1, 1];
  this.orientation = 1.0; //1.0 for RH, -1.0 for LH
  this.fov = 45.0;
  this.focalLength = 1.0 / Math.tan(0.5 * this.fov * Math.PI/180);
  this.resolution = props["res"];

  //Calculated scaling
  this.scaling = [props["res"][0] * props["scale"][0], 
                  props["res"][1] * props["scale"][1],
                  props["res"][2] * props["scale"][2]];
  this.tiles = [this.image.width / props["res"][0],
                this.image.height / props["res"][1]];
  var maxn = props["res"][2];
  this.scaling = [maxn / this.scaling[0], maxn / this.scaling[1], maxn / this.scaling[2]]

  //Set dims
  //Inverse the scaling factors, used to correct focus/centre of rotation
  this.centre = [0.5/this.scaling[0], 0.5/this.scaling[1], 0.5/this.scaling[2]];
  //this.centre = [0.5, 0.5, 0.5];
  this.modelsize = Math.sqrt(3);
  this.focus = this.centre;

  this.translate[2] = -this.modelsize*1.25;

  OK.debug("New model size: " + this.modelsize + ", Focal point: " + this.focus[0] + "," + this.focus[1] + "," + this.focus[2]);

    //Setup 3D rendering
    this.linePositionBuffer = this.gl.createBuffer();
    this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.linePositionBuffer);
    var vertexPositions = [-1.0,  0.0,  0.0,
                            1.0,  0.0,  0.0,
                            0.0, -1.0,  0.0, 
                            0.0,  1.0,  0.0, 
                            0.0,  0.0, -1.0, 
                            0.0,  0.0,  1.0];
    this.gl.bufferData(this.gl.ARRAY_BUFFER, new Float32Array(vertexPositions), this.gl.STATIC_DRAW);
    this.linePositionBuffer.itemSize = 3;
    this.linePositionBuffer.numItems = 6;

    this.lineColourBuffer = this.gl.createBuffer();
    this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.lineColourBuffer);
    var vertexColours =  [1.0, 0.0, 0.0, 1.0,
                          1.0, 0.0, 0.0, 1.0,
                          0.0, 1.0, 0.0, 1.0,
                          0.0, 1.0, 0.0, 1.0,
                          0.0, 0.0, 1.0, 1.0,
                          0.0, 0.0, 1.0, 1.0];
    this.gl.bufferData(this.gl.ARRAY_BUFFER, new Float32Array(vertexColours), this.gl.STATIC_DRAW);
    this.lineColourBuffer.itemSize = 4;
    this.lineColourBuffer.numItems = 6;


  //Setup two-triangle rendering
  this.webgl.init2dBuffers(this.gl.TEXTURE1); //Use 2nd texture unit

  //Override texture params set in previous call
    this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_WRAP_S, this.gl.CLAMP_TO_EDGE);
    this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_WRAP_T, this.gl.CLAMP_TO_EDGE);

  this.webgl.loadTexture(image, this.gl.LINEAR);

  //Compile the shaders
  var IE11 = !!window.MSInputMethodContext;  //More evil user-agent sniffing, broken WebGL on windows forces me to do this
  this.lineprogram = new WebGLProgram(this.gl, 'line-vs', 'line-fs');
  if (this.lineprogram.errors) OK.debug(this.lineprogram.errors);
  this.lineprogram.setup(["aVertexPosition", "aVertexColour"], ["uColour", "uAlpha"]);
    this.gl.vertexAttribPointer(this.lineprogram.attributes["aVertexPosition"], this.linePositionBuffer.itemSize, this.gl.FLOAT, false, 0, 0);
    this.gl.vertexAttribPointer(this.lineprogram.attributes["aVertexColour"], this.lineColourBuffer.itemSize, this.gl.FLOAT, false, 0, 0);

  if (!props.volume.properties.disabled) {
    var defines = "precision highp float; const highp vec2 slices = vec2(" + this.tiles[0] + "," + this.tiles[1] + ");\n";
    defines += (IE11 ? "#define IE11\n" : "#define NOT_IE11\n");
    var maxSamples = mobile ? 256 : 1024;
    defines += "const int maxSamples = " + maxSamples + ";\n\n\n\n\n\n"; //Extra newlines so errors in main shader have correct line #
    OK.debug(defines);

    var fs = getSourceFromElement('ray-fs');
    this.program = new WebGLProgram(this.gl, 'ray-vs', defines + fs);
     //console.log(defines + fs);
    if (this.program.errors) OK.debug(this.program.errors);
    this.program.setup(["aVertexPosition"], 
                       ["uBackCoord", "uVolume", "uTransferFunction", "uEnableColour", "uFilter",
                        "uDensityFactor", "uPower", "uBrightness", "uContrast", "uSamples",
                        "uFocalLength", "uWindowSize", "uBBMin", "uBBMax", "uResolution",
                        "uIsoValue", "uIsoColour", "uIsoSmooth", "uIsoWalls"]);
  }

  this.gl.enable(this.gl.DEPTH_TEST);
  this.gl.clearColor(0, 0, 0, 0);
  //this.gl.clearColor(this.background.red/255, this.background.green/255, this.background.blue/255, 0.0);
    this.gl.enable(this.gl.BLEND);
    this.gl.blendFunc(this.gl.SRC_ALPHA, this.gl.ONE_MINUS_SRC_ALPHA);
  this.gl.depthFunc(this.gl.LEQUAL);

  //Set default properties
  this.properties = {};

  this.properties.samples = 256;
  this.properties.isovalue = 0.0;
  this.properties.drawWalls = false;
  this.properties.isoalpha = 0.75;
  this.properties.isosmooth = 1.0;
  //this.properties.isocolour = [214, 188, 86];
  this.properties.isocolour = [255,0,0];

  this.properties.Xmin = this.properties.Ymin = this.properties.Zmin = 0.0;
  this.properties.Xmax = this.properties.Ymax = this.properties.Zmax = 1.0;

  this.properties.density = 10.0;
  this.properties.brightness = 0.0;
  this.properties.contrast = 1.0;
  this.properties.power = 1.0;
  this.properties.usecolourmap = false;
  this.properties.colourmap_url = "";
  this.properties.tricubicFilter = false;
  this.properties.lowPowerDevice = false;
  this.properties.disabled = false;
  this.properties.axes = true;
  this.properties.border = true;

  //Load from local storage or previously loaded file
  if (props.volume) this.load(props.volume);

  //Setup bounding box
  this.clipChanged();

  if (mobile) //Low power can be enabled in props by default but not switched off
    this.properties.lowPowerDevice = true;
}

Volume.prototype.box = function(min, max) {
  var vertices = new Float32Array(
        [
          min[0], min[1], max[2],
          min[0], max[1], max[2],
          max[0], max[1], max[2],
          max[0], min[1], max[2],
          min[0], min[1], min[2],
          min[0], max[1], min[2],
          max[0], max[1], min[2],
          max[0], min[1], min[2]
        ]);


  if (!this.boxPositionBuffer) this.boxPositionBuffer = this.gl.createBuffer();
  this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.boxPositionBuffer);
  this.gl.bufferData(this.gl.ARRAY_BUFFER, vertices, this.gl.STATIC_DRAW);
  this.boxPositionBuffer.itemSize = 3;

  if (!this.boxIndexBuffer) {
    var indices = new Uint16Array(
        [
          0, 1, 1, 2, 2, 3, 3, 0,
          4, 5, 5, 6, 6, 7, 7, 4,
          0, 4, 3, 7, 1, 5, 2, 6
        ]
       );
    this.boxIndexBuffer = this.gl.createBuffer();
    this.gl.bindBuffer(this.gl.ELEMENT_ARRAY_BUFFER, this.boxIndexBuffer); 
    this.gl.bufferData(this.gl.ELEMENT_ARRAY_BUFFER, indices, this.gl.STATIC_DRAW);
    this.boxIndexBuffer.numItems = 24;
  }
}

Volume.prototype.addGUI = function(gui) {
  if (this.gui) this.gui.destroy(); //Necessary/valid?
  var that = this;

  this.gui = gui;

  var f = this.gui.addFolder('Volume');
  f.add(this.properties, 'lowPowerDevice');
  f.add(this.properties, 'usecolourmap');
  this.properties.samples = Math.floor(this.properties.samples);
  if (this.properties.samples % 32 > 0) this.properties.samples -= this.properties.samples % 32;
  f.add(this.properties, 'samples', 32, 1024, 32);
  f.add(this.properties, 'density', 0.0, 50.0, 1.0);
  f.add(this.properties, 'brightness', -1.0, 1.0, 0.05);
  f.add(this.properties, 'contrast', 0.0, 3.0, 0.05);
  f.add(this.properties, 'power', 0.01, 5.0, 0.05);
  f.add(this.properties, 'axes');
  f.add(this.properties, 'border');
  f.add(this.properties, 'tricubicFilter');
  f.open();
  //this.gui.__folders.f.controllers[1].updateDisplay();  //Update samples display

  //Clip planes folder
  var boxchangefn = function() {that.clipChanged();};
  var f0 = this.gui.addFolder('Clip planes');
  f0.add(this.properties, 'Xmin', 0.0, 1.0, 0.01).onFinishChange(boxchangefn);//.onFinishChange(function(l) {if (slicer) slicer.setX(l);});
  f0.add(this.properties, 'Xmax', 0.0, 1.0, 0.01).onFinishChange(boxchangefn);//.onFinishChange(function(l) {if (slicer) slicer.setX(l);});
  f0.add(this.properties, 'Ymin', 0.0, 1.0, 0.01).onFinishChange(boxchangefn);//.onFinishChange(function(l) {if (slicer) slicer.setY(l);});
  f0.add(this.properties, 'Ymax', 0.0, 1.0, 0.01).onFinishChange(boxchangefn);//.onFinishChange(function(l) {if (slicer) slicer.setY(l);});
  f0.add(this.properties, 'Zmin', 0.0, 1.0, 0.01).onFinishChange(boxchangefn);//.onFinishChange(function(l) {if (slicer) slicer.setZ(l);});
  f0.add(this.properties, 'Zmax', 0.0, 1.0, 0.01).onFinishChange(boxchangefn);//.onFinishChange(function(l) {if (slicer) slicer.setZ(l);});
  //f0.open();

  //Isosurfaces folder
  var f1 = this.gui.addFolder('Isosurface');
  f1.add(this.properties, 'isovalue', 0.0, 1.0, 0.01);
  f1.add(this.properties, 'drawWalls');
  f1.add(this.properties, 'isoalpha', 0.0, 1.0, 0.01);
  f1.add(this.properties, 'isosmooth', 0.1, 3.0, 0.1);
  f1.addColor(this.properties, 'isocolour');
  f1.open();

  // Iterate over all controllers and set change function
  var changefn = function(value) {that.delayedRender(250);};  //Use delayed high quality render for faster interaction
  for (var i in f.__controllers)
    f.__controllers[i].onChange(changefn);
  for (var i in f0.__controllers)
    f0.__controllers[i].onChange(changefn);
  for (var i in f1.__controllers)
    f1.__controllers[i].onChange(changefn);
}

Volume.prototype.clipChanged = function() {
  //Bounding box update
  this.box([this.properties.Xmin, this.properties.Ymin, this.properties.Zmin],
           [this.properties.Xmax, this.properties.Ymax, this.properties.Zmax]);
}

Volume.prototype.load = function(src) {
  colours.read(src.colourmap);
  colours.update();
  for (var key in src.properties)
    this.properties[key] = src.properties[key]

  if (src.translate) this.translate = src.translate;
  //Initial rotation (Euler angles or quaternion accepted)
  if (src.rotate) {
    if (src.rotate.length == 3) {
      this.rotateZ(-src.rotate[2]);
      this.rotateY(-src.rotate[1]);
      this.rotateX(-src.rotate[0]);
    } else if (src.rotate[3] != 0)
      this.rotate = quat4.create(src.rotate);    
  }
  //this.focus = src.focus;
  //this.centre = src.centre;
}

Volume.prototype.get = function(matrix) {
  var data = {};
  if (matrix) {
    //Include the rotation matrix as array
    //data.modelview = this.webgl.modelView.toArray();
    var r = quat4.toMat3(this.rotate);
    data.rotate = [r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], r[8]];
    // to simplify load from enc
    data.rotate_quat4 = this.rotate;
  } else {
    //Rotation quaternion
    data.rotate = [this.rotate[0], this.rotate[1], this.rotate[2], this.rotate[3]];
  }
  data.translate = this.translate;
  //data.focus = this.focus;
  //data.centre = this.centre;
  data.colourmap = colours.palette.toString();
  data.properties = this.properties;
  return data;
}

var frames = 0;
var testtime;

Volume.prototype.draw = function(send_to_server, lowquality, testmode) {
  if (!this.properties || !this.webgl) return; //Getting called before vars defined, TODO:fix
  //this.time = new Date().getTime();
  if (this.width == 0 || this.height == 0) {
    //Get size from window
    this.width = window.innerWidth;
    this.height = window.innerHeight;
  }

  if (this.width != this.canvas.width || this.height != this.canvas.height) {
    //Get size from element
    this.canvas.width = this.width;
    this.canvas.height = this.height;
    this.canvas.setAttribute("width", this.width);
    this.canvas.setAttribute("height", this.height);
    if (this.gl) {
      this.gl.viewportWidth = this.width - 300; //ImageHD offset for controls!
      this.gl.viewportHeight = this.height;
      this.webgl.viewport = new Viewport(0, 0, this.width - 300, this.height);
    }
  }
  //Reset to auto-size...
  //this.width = this.height = 0;
  //console.log(this.width + "," + this.height);

  this.camera();

      this.gl.clear(this.gl.COLOR_BUFFER_BIT | this.gl.DEPTH_BUFFER_BIT);
      this.gl.viewport(this.webgl.viewport.x, this.webgl.viewport.y, this.webgl.viewport.width, this.webgl.viewport.height);

  if (this.properties.axes) this.drawAxis(1.0);
  if (this.properties.border) this.drawBox(1.0);

  this.camera();

  //Volume render (skip while interacting if lowpower device flag is set)
  if (!(lowquality && this.properties.lowPowerDevice) && !this.properties.disabled) {
    this.webgl.use(this.program);
    this.webgl.modelView.scale(this.scaling);  //Apply scaling
      this.gl.disableVertexAttribArray(this.program.attributes["aVertexColour"]);

    this.gl.activeTexture(this.gl.TEXTURE0);
    this.gl.bindTexture(this.gl.TEXTURE_2D, this.webgl.textures[0]);

    this.gl.activeTexture(this.gl.TEXTURE1);
    this.gl.bindTexture(this.gl.TEXTURE_2D, this.webgl.gradientTexture);

    //Only render full quality when not interacting
    //this.gl.uniform1i(this.program.uniforms["uSamples"], this.samples);
    this.gl.uniform1i(this.program.uniforms["uSamples"], lowquality ? this.properties.samples * 0.5 : this.properties.samples);
    this.gl.uniform1i(this.program.uniforms["uVolume"], 0);
    this.gl.uniform1i(this.program.uniforms["uTransferFunction"], 1);
    this.gl.uniform1i(this.program.uniforms["uEnableColour"], this.properties.usecolourmap);
    this.gl.uniform1i(this.program.uniforms["uFilter"], lowquality ? false : this.properties.tricubicFilter);
    this.gl.uniform1f(this.program.uniforms["uFocalLength"], this.focalLength);
    this.gl.uniform2fv(this.program.uniforms["uWindowSize"], new Float32Array([this.gl.viewportWidth, this.gl.viewportHeight]));

    var bbmin = [this.properties.Xmin, this.properties.Ymin, this.properties.Zmin];
    var bbmax = [this.properties.Xmax, this.properties.Ymax, this.properties.Zmax];
    this.gl.uniform3fv(this.program.uniforms["uBBMin"], new Float32Array(bbmin));
    this.gl.uniform3fv(this.program.uniforms["uBBMax"], new Float32Array(bbmax));
    this.gl.uniform3fv(this.program.uniforms["uResolution"], new Float32Array(this.resolution));

    this.gl.uniform1f(this.program.uniforms["uDensityFactor"], this.properties.density);

    // brightness and contrast
    this.gl.uniform1f(this.program.uniforms["uBrightness"], this.properties.brightness);
    this.gl.uniform1f(this.program.uniforms["uContrast"], this.properties.contrast);
    this.gl.uniform1f(this.program.uniforms["uPower"], this.properties.power);

    this.gl.uniform1f(this.program.uniforms["uIsoValue"], this.properties.isovalue);
    var colour = new Colour(this.properties.isocolour);
    colour.alpha = this.properties.isoalpha;
    this.gl.uniform4fv(this.program.uniforms["uIsoColour"], colour.rgbaGL());
    this.gl.uniform1f(this.program.uniforms["uIsoSmooth"], this.properties.isosmooth);
    this.gl.uniform1i(this.program.uniforms["uIsoWalls"], this.properties.drawWalls);

    //Clip Plane
    //this.gl.uniform4fv(this.program.uniforms["uClipPlane"], new Float32Array([0, 1, 0, 7]));
    //this.gl.uniform3fv(this.program.uniforms["uScaling"], new Float32Array(this.scaling));
    //this.gl.uniform3fv(this.program.uniforms["uScaling"], new Float32Array([1,1,1]));

    //Draw two triangles
    this.webgl.initDraw2d();
    //this.gl.enableVertexAttribArray(this.program.attributes["aVertexPosition"]);
    //this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.webgl.vertexPositionBuffer);
    //this.gl.vertexAttribPointer(this.program.attributes["aVertexPosition"], this.webgl.vertexPositionBuffer.itemSize, this.gl.FLOAT, false, 0, 0);
    //this.webgl.setMatrices();

    this.gl.drawArrays(this.gl.TRIANGLE_STRIP, 0, this.webgl.vertexPositionBuffer.numItems);

  } else {
    //Always draw axis even if turned off to show interaction
    if (!this.properties.axes) this.drawAxis(1.0);
    //Bounding box
    this.drawBox(1.0);
  }

  //this.timeAction("Render", this.time);

  if (this.properties.axes) {
    this.gl.clear(this.gl.DEPTH_BUFFER_BIT);
    this.camera();
    this.drawAxis(0.2);
  }

  if (this.properties.border) {
    //Bounding box
    this.camera();
    this.drawBox(0.2);
  }

  //Running speed test?
  if (testmode) {
    frames++;
    $('status').innerHTML = "Speed test: frame " + frames;
    if (frames == 5) {
      var elapsed = new Date().getTime() - testtime;
      console.log("5 frames in " + (elapsed / 1000) + " seconds");
      //Reduce quality for slower device
      if (elapsed > 1000) {
        this.properties.samples = Math.floor(this.properties.samples * 1000 / elapsed);
        if (this.properties.samples < 32) this.properties.samples = 32;
        $('status').innerHTML = "5 frames in " + (elapsed / 1000) + " seconds, Reduced quality to " + this.properties.samples;
        //Hide info window in 2 sec
        setTimeout(function() {info.hide()}, 2000);
      } else {
        info.hide();
      }
    } else {
      this.draw(true, true, true);
    }
  }

  //Update server display for ImageHD
  if (send_to_server){
    if (!lowquality){
      updateServer();
    }
  }
}

Volume.prototype.camera = function() {
  //Apply translation to origin, any rotation and scaling
  this.webgl.modelView.identity()
  this.webgl.modelView.translate(this.translate)
  // Adjust centre of rotation, default is same as focal point so this does nothing...
  adjust = [-(this.focus[0] - this.centre[0]), -(this.focus[1] - this.centre[1]), -(this.focus[2] - this.centre[2])];
  this.webgl.modelView.translate(adjust);

  // rotate model 
  var rotmat = quat4.toMat4(this.rotate);
  this.webgl.modelView.mult(rotmat);
  //this.webgl.modelView.mult(this.rotate);

  // Adjust back for rotation centre
  adjust = [this.focus[0] - this.centre[0], this.focus[1] - this.centre[1], this.focus[2] - this.centre[2]];
  this.webgl.modelView.translate(adjust);

  // Translate back by centre of model to align eye with model centre
  this.webgl.modelView.translate([-this.focus[0], -this.focus[1], -this.focus[2] * this.orientation]);

  //Perspective matrix (not required for volume render pass)
  this.webgl.setPerspective(this.fov, this.gl.viewportWidth / this.gl.viewportHeight, 0.1, 100.0);
}

Volume.prototype.drawAxis = function(alpha) {
  this.webgl.use(this.lineprogram);
  this.gl.uniform1f(this.lineprogram.uniforms["uAlpha"], alpha);
  this.gl.uniform4fv(this.lineprogram.uniforms["uColour"], new Float32Array([1.0, 1.0, 1.0, 0.0]));

  this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.linePositionBuffer);
  this.gl.enableVertexAttribArray(this.lineprogram.attributes["aVertexPosition"]);
  this.gl.vertexAttribPointer(this.lineprogram.attributes["aVertexPosition"], this.linePositionBuffer.itemSize, this.gl.FLOAT, false, 0, 0);

  this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.lineColourBuffer);
  this.gl.enableVertexAttribArray(this.lineprogram.attributes["aVertexColour"]);
  this.gl.vertexAttribPointer(this.lineprogram.attributes["aVertexColour"], this.lineColourBuffer.itemSize, this.gl.FLOAT, false, 0, 0);

  //Axis position, default centre, use slicer positions if available
  var pos = [0.5/this.scaling[0], 0.5/this.scaling[1], 0.5/this.scaling[2]];
  if (this.slicer) {
    pos = [this.slicer.slices[0]/this.scaling[0], 
           this.slicer.slices[1]/this.scaling[1],
           this.slicer.slices[2]/this.scaling[2]];
  }
  this.webgl.modelView.translate(pos);
  this.webgl.setMatrices();
  this.gl.drawArrays(this.gl.LINES, 0, this.linePositionBuffer.numItems);
  this.webgl.modelView.translate([-pos[0], -pos[1], -pos[2]]);
}

Volume.prototype.drawBox = function(alpha) {
  this.webgl.use(this.lineprogram);
  this.gl.uniform1f(this.lineprogram.uniforms["uAlpha"], alpha);
  this.gl.uniform4fv(this.lineprogram.uniforms["uColour"], this.borderColour.rgbaGL());

  this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.boxPositionBuffer);
  this.gl.bindBuffer(this.gl.ELEMENT_ARRAY_BUFFER, this.boxIndexBuffer);
  this.gl.enableVertexAttribArray(this.lineprogram.attributes["aVertexPosition"]);
  this.gl.vertexAttribPointer(this.lineprogram.attributes["aVertexPosition"], this.boxPositionBuffer.itemSize, this.gl.FLOAT, false, 0, 0);
    this.gl.vertexAttribPointer(this.lineprogram.attributes["aVertexColour"], 4, this.gl.UNSIGNED_BYTE, true, 0, 0);

    //this.webgl.modelView.scale(this.scaling);  //Apply scaling
    this.webgl.modelView.scale([1.0/this.scaling[0], 1.0/this.scaling[1], 1.0/this.scaling[2]]);  //Invert scaling

  this.webgl.setMatrices();
  this.gl.drawElements(this.gl.LINES, this.boxIndexBuffer.numItems, this.gl.UNSIGNED_SHORT, 0);
}

Volume.prototype.timeAction = function(action, start) {
  if (!window.requestAnimationFrame) return;
  var timer = start || new Date().getTime();
  function logTime() {
    var elapsed = new Date().getTime() - timer;
    if (elapsed < 50) 
      window.requestAnimationFrame(logTime); //Not enough time, assume triggered too early, try again
    else {
      console.log(action + " took: " + (elapsed / 1000) + " seconds");
      /*if (elapsed > 200 && this.quality > 32) {
        this.quality = Math.floor(this.quality * 0.5);
        OK.debug("Reducing quality to " + this.quality + " samples");
        this.draw();
      } else if (elapsed < 100 && this.quality < 512 && this.quality >= 128) {
        this.quality = this.quality * 2;
        OK.debug("Increasing quality to " + this.quality + " samples");
        this.draw();
      }*/
    }
  }
  window.requestAnimationFrame(logTime);
}

Volume.prototype.rotateX = function(deg) {
  this.rotation(deg, [1,0,0]);
}

Volume.prototype.rotateY = function(deg) {
  this.rotation(deg, [0,1,0]);
}

Volume.prototype.rotateZ = function(deg) {
  this.rotation(deg, [0,0,1]);
}

Volume.prototype.rotation = function(deg, axis) {
  //Quaterion rotate
  var arad = deg * Math.PI / 180.0;
  var rotation = quat4.fromAngleAxis(arad, axis);
  rotation = quat4.normalize(rotation);
  this.rotate = quat4.multiply(rotation, this.rotate);
}

Volume.prototype.zoom = function(factor) {
  this.translate[2] += factor * this.modelsize;
}

Volume.prototype.zoomClip = function(factor) {
  //var clip = parseFloat($("nearclip").value) - factor;
  //$("nearclip").value = clip;
  this.draw(true);
  //OK.debug(clip + " " + $("nearclip").value);
}

Volume.prototype.click = function(event, mouse) {
  this.rotating = false;
  this.draw(true);
  return false;
}

Volume.prototype.move = function(event, mouse) {
  this.rotating = false;
  if (!mouse.isdown) return true;

  //Switch buttons for translate/rotate
  var button = mouse.button;
  if (this.flipbuttons && button != 1)
    button = (button == 0 ? 2 : 0);

  switch (button)
  {
    case 0:
      this.rotateY(mouse.deltaX/5.0);
      this.rotateX(mouse.deltaY/5.0);
      this.rotating = true;
      break;
    case 1:
      this.rotateZ(Math.sqrt(mouse.deltaX*mouse.deltaX + mouse.deltaY*mouse.deltaY)/5.0);
      this.rotating = true;
      break;
    case 2:
      var adjust = this.modelsize / 1000;   //1/1000th of size
      this.translate[0] += mouse.deltaX * adjust;
      this.translate[1] -= mouse.deltaY * adjust;
      break;
  }

  this.draw(true, true);
  return false;
}

Volume.prototype.wheel = function(event, mouse) {
  if (event.shiftKey) {
    var factor = event.spin * 0.01;
    this.zoomClip(factor);
  } else {
    var factor = event.spin * 0.05;
    this.zoom(factor);
  }
  this.delayedRender(250); //Delayed high quality render

  return false; //Prevent default
}

Volume.prototype.pinch = function(event, mouse) {

  var zoom = (event.distance * 0.0001);
  console.log(' --> ' + zoom);
  this.zoom(zoom);
  this.delayedRender(250); //Delayed high quality render
}

//Delayed high quality render
Volume.prototype.delayedRender = function(time, skipImm) {
  if (!skipImm) this.draw(true, true); //Draw immediately in low quality
  //Set timer to draw the final render
  if (this.delaytimer) clearTimeout(this.delaytimer);
  var that = this;
  this.delaytimer = setTimeout(function() {that.draw(true);}, time);
}

Volume.prototype.applyBackground = function(bg) {
  if (!bg) return;
  this.background = new Colour(bg);
  var hsv = this.background.HSV();
  this.borderColour = hsv.V > 50 ? new Colour(0xff444444) : new Colour(0xffbbbbbb);

  //document.body.style.background = bg;

    //Set canvas background
    if (this.properties.usecolourmap)
      this.canvas.style.backgroundColor = bg;
    else
      this.canvas.style.backgroundColor = "black";


}
/**
 * @constructor
 */
function Toolbox(id, x, y) {
  //Mouse processing:
  this.el = $(id);
  this.mouse = new Mouse(this.el, this);
  this.mouse.moveUpdate = true;
  this.el.mouse = this.mouse;
  this.style = $S(id);
  if (x && y) {
    this.style.left = x + 'px';
    this.style.top = y + 'px';
  } else {
    this.style.left = ((window.innerWidth - this.el.offsetWidth) * 0.5) + 'px';
    this.style.top = ((window.innerHeight - this.el.offsetHeight) * 0.5) + 'px';
  }
  this.drag = false;
}

Toolbox.prototype.toggle = function() {
  if (this.style.visibility == 'visible')
    this.hide();
  else
    this.show();
}

Toolbox.prototype.show = function() {
  this.style.visibility = 'visible';
}

Toolbox.prototype.hide = function() {
  this.style.visibility = 'hidden';
}

//Mouse event handling
Toolbox.prototype.click = function(e, mouse) {
  this.drag = false;
  return true;
}

Toolbox.prototype.down = function(e, mouse) {
  //Process left drag only
  this.drag = false;
  if (mouse.button == 0 && e.target.className.indexOf('scroll') < 0 && ['INPUT', 'SELECT', 'OPTION', 'RADIO'].indexOf(e.target.tagName) < 0)
    this.drag = true;
  return true;
}

Toolbox.prototype.move = function(e, mouse) {
  if (!mouse.isdown) return true;
  if (!this.drag) return true;

  //Drag position
  this.el.style.left = parseInt(this.el.style.left) + mouse.deltaX + 'px';
  this.el.style.top = parseInt(this.el.style.top) + mouse.deltaY + 'px';
  return false;
}

Toolbox.prototype.wheel = function(e, mouse) {
}
