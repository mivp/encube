/* @preserve
 *
 * d3plots.js
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
 * encube: d3plots
 * Plots for Miniature CAVE2
 *
 * Copyright (c) 2015, Dany Vohl. All rights reserved.
 * Author: Dany Vohl - dvohl [at] astro [dot] swin [dot] edu
 *
 * Licensed under the GNU Lesser General Public License
 * https://www.gnu.org/licenses/lgpl.html
 *
 */

var createHistogram = function(data, nbins, xmin, xmax, binInc, title,
                               node, panel,
                               mean, stand_dev, min_filter, max_filter)
{
    // Clear div.
    document.getElementById("divSVG").innerHTML = ""

    //console.log("nbins: " + nbins);
    //console.log("xmin: " + xmin);
    //console.log("xmax: " + xmax);
    //console.log("binInc: " + binInc);

    // A formatter for counts.
    var formatCount = d3.format(",.4f");
    var totalWidth = 400;
    var totalHeight = 240;
    var margin = {top: 40, right: 60, bottom: 50, left: 70},
      width = totalWidth - margin.left - margin.right,
      height = totalHeight - margin.top - margin.bottom;

    var binTicks = [];
    /*
    for (var i = xmin; i < xmax; i += binInc) {
        binTicks.push(i);
    }
    */
    for (var i = xmin; i < xmax; i += ((xmax-xmin)/10)) {
        binTicks.push(i);
    }

    var xmin = xmin;
    var xmax = xmax;

    var x = d3.scale.linear()
      .domain([xmin, xmax])
      .range([xmin, width]);
    var binWidth = parseFloat(width / (binTicks.length)) - 1;

    //var y = d3.scale.log()
    if (d3.min(data, function(d) { return d.y; }) != 0)
    {
        var y = d3.scale.log()
          .domain([d3.min(data, function(d) { return d.y; }), d3.max(data, function(d) { return d.y; })+1e-1])
          //.domain([cmin, cmax+1])
          .range([height, 0]);
    }
    else
    {
        var y = d3.scale.log()
          //.domain([d3.min(data, function(d) { return d.y; }), d3.max(data, function(d) { return d.y; })])
          .domain([d3.min(data, function(d) { return d.y; })+1e-6, d3.max(data, function(d) { return d.y; })+1e-1])
          //.domain([cmin, cmax+1])
          .range([height, 0]);
    }

    var xAxis = d3.svg.axis()
      .scale(x)
      .orient("bottom")
      .tickValues(binTicks);

    var yAxis = d3.svg.axis()
      .scale(y)
      .orient("left");

    var svg = d3.select("#divSVG").append("svg")
      .attr("width", width + margin.left + margin.right)
      .attr("height", height + margin.top + margin.bottom)
      .attr("id", "requestedHistogram")
      .append("g")
      .attr("transform", "translate(" + margin.left + "," + margin.top + ")");

    var bar = svg.selectAll(".bar")
      .data(data)
      .enter()
      .append("rect")
      .attr("class", "bar")
      .attr("x", function(d) { return x(d.x); })
      .attr("width", binWidth)
      .attr("y", function(d) { return y(d.y); })
      .attr("height", function(d) { return height - y(d.y); })
      .on("mouseover", function(d) {
        var barWidth = parseFloat(d3.select(this).attr("width"));
        var xPosition = parseFloat(d3.select(this).attr("x")) + (barWidth / 2);
        var yPosition = parseFloat(d3.select(this).attr("y")) - 10;

        svg.append("text")
          .attr("id", "tooltip")
          .attr("x", xPosition)
          .attr("y", yPosition)
          .attr("fill", "white")
          .attr("text-anchor", "middle")
          .text(d.y);
      })
      .on("mouseout", function(d) {
        d3.select('#tooltip').remove();
      });

    svg.append("g")
      .attr("class", "x axis")
      .attr("transform", "translate(0," + height + ")")
      .attr("fill", "white")
      .attr("font-size", 8)
      .call(xAxis)
        .selectAll("text")
        .style("text-anchor", "end")
        .attr("dx", "-.8em")
        .attr("dy", ".15em")
        .attr("transform", function(d) {
            return "rotate(-65)"
            });

    /*
    var brush = d3.svg.brush()
    .x(x2)
    .on("brush", brushed);
    */
    var brush = d3.svg.brush().x(x)
        .on("brushstart", brushstart)
        .on("brush", brushmove)
        .on("brushend", brushend);

    svg.append("g")
        .attr("class", "y axis")
        .attr("fill", "white")
        .attr("font-size", 10)
        //.attr("transform", "translate(0," + height + ")")
        .call(yAxis);

    // Add axis labels
    svg.append("text")
        .attr("class", "x label")
        .attr("transform", "translate(" + (width / 2) + " ," + (height + margin.bottom) + ")")
        //.attr("dy", "1em")
        .attr("text-anchor", "middle")
        .attr("fill", "white")
        .text("Normalised Flux");

    svg.append("text")
        .attr("class", "y label")
        .attr("transform", "rotate(-90)")
        .attr("y", 0 - margin.left)
        .attr("x", 0 - (height / 2))
        .attr("dy", "1em")
        .attr("text-anchor", "middle")
        .attr("fill", "white")
        .text("P(flux)");

    // Add title to chart
    svg.append("text")
        .attr("class", "title")
        .attr("transform", "translate(" + (width / 2) + " ," + (-20) + ")")
        //.attr("dy", "1em")
        .attr("text-anchor", "middle")
        .attr("fill", "white")
        //.text("Panel 1, screen 1: NGC1569");
        .text(title);

    svg.append("g")
        .attr("class", "brush")
        .call(brush)
        .selectAll("rect")
        .attr("height", height);

    // Include +/-3sigma button or not
    if (stand_dev != 0) {
        // make some buttons to drive our zoom
        d3.select("#divSVG").append("div")
        .attr("id","btnDiv")
        .style('font-size','75%')
        .style("width","250px")
        .style("position","relative")
        .style("left","-15px")
        .style("top","-10px")

        var btns = d3.select("#btnDiv").selectAll("button").data(["[-3s, +3s]"])
        btns = btns.enter().append("button")
                .attr("class","leftButtons")
        // fill the buttons with the year from the data assigned to them
        btns.each(function (d) {
            this.innerText = d;
        })

        btns.on("click", drawBrush);
        function drawBrush() {
            // define our brush extent to be begin and end of the year
            brush.extent([mean-(3*stand_dev), mean+(3*stand_dev)])

            brush(d3.select(".brush").transition());

            brush.event(d3.select(".brush").transition().delay(1000))
        }

        // define our brush extent to be [-3s, +3s]
        brush.extent([min_filter, max_filter])

        // now draw the brush to match our extent
        // use transition to slow it down so we can see what is happening
        // remove transition so just d3.select(".brush") to just draw
        brush(d3.select(".brush"));
    }


    function brushstart() {
      svg.classed("selecting", true);
    }

    var range_extent;
    var this_panel = panel;
    var this_node = node;
    function brushmove() {
        range_extent = d3.event.target.extent();

        /*
        var JSONstring = {'node':this_node,
                         'panel':this_panel,
                         'min_filter':range_extent[0],
                         'max_filter':range_extent[1],
                         'skip_history': 1};
        $j.ajax({
            type: "POST",
            url: SERVER + "/update",
            data: "range_filter="+ encodeURIComponent(JSON.stringify(JSONstring))
        });
        */

      //symbol.classed("selected", function(d) { return s[0] <= (d = x(d)) && d <= s[1]; });
    }

    function brushend() {
        var JSONstring = {'node':this_node,
                         'panel':this_panel,
                         'min_filter':range_extent[0],
                         'max_filter':range_extent[1],
                         'skip_history': 0};
        $j.ajax({
            type: "POST",
            url: SERVER + "/update",
            data: "range_filter="+ encodeURIComponent(JSON.stringify(JSONstring))
        });
        svg.classed("selecting", !d3.event.target.empty());
    }
};

var createStackedHistograms = function()
{
    var margin = {
            top: 10,
            right: 30,
            bottom: 30,
            left: 50
        },
        width = 960 - margin.left - margin.right,
        height = 500 - margin.top - margin.bottom;

    var parseDate = d3.time.format("%m/%d/%Y %I:%M:%S %p").parse;
    var formatDate = d3.time.format("%m/%y");
    var formatCount = d3.format(",.4f");

    var x = d3.time.scale().range([0, width]);
    var y = d3.scale.linear().range([height, 0]);
    var color = d3.scale.category10();

    var xAxis = d3.svg.axis().scale(x)
        .orient("bottom").tickFormat(formatCount);

    var yAxis = d3.svg.axis().scale(y)
        .orient("left").ticks(12);

    var stack = d3.layout.stack()
        .values(function(d) {
            return d.values;
        });

    // Create the SVG drawing area
    var svg = d3.select("body")
        .append("svg")
        .attr("width", width + margin.left + margin.right)
        .attr("height", height + margin.top + margin.bottom)
        .append("g")
        .attr("transform", "translate(" + margin.left + "," + margin.top + ")");

    // Get the data
    d3.csv("Rat_Sightings.csv", function(error, data) {

        // Parse the date strings into javascript dates
        data.forEach(function(d) {
            d.created_date = parseDate(d["Created Date"]);
        });

        // Determine the first and list dates in the data set
        var monthExtent = d3.extent(data, function(d) {
            return d.created_date;
        });

        // Create one bin per month, use an offset to include the first and last months
        var monthBins = d3.time.months(d3.time.month.offset(monthExtent[0], -1),
            d3.time.month.offset(monthExtent[1], 1));

        // Use the histogram layout to create a function that will bin the data
        var binByMonth = d3.layout.histogram()
            .value(function(d) {
                return d.created_date;
            })
            .bins(monthBins);

        // Use D3's nest function to group the data by borough
        var dataGroupedByBorough = d3.nest()
            .key(function(d) {
                return d["Borough"]
            })
            .map(data, d3.map);

        // Apply the histogram binning function to the data and convert from
        // a map to an array so that the stack layout can consume it
        var histDataByBorough = [];
        dataGroupedByBorough.forEach(function(key, value) {
            // Bin the data for each borough by month
            var histData = binByMonth(value);
            histDataByBorough.push({
                borough: key,
                values: histData
            });
        });

        var stackedHistData = stack(histDataByBorough);

        // Scale the range of the data by setting the domain
        x.domain(d3.extent(monthBins));
        y.domain([0, d3.max(stackedHistData[stackedHistData.length - 1].values, function(d) {
            return d.y + d.y0;
        })]);

        // Set up one group for each borough
        // Note that color doesn't have a domain set, so colors are assigned to boroughs
        // below on a first come, first serve basis
        var borough = svg.selectAll(".borough")
            .data(stackedHistData)
          .enter().append("g")
            .attr("class", "borough")
            .style("fill", function(d, i) {
                return color(d.borough);
            })
            .style("stroke", function(d, i) {
                return d3.rgb(color(d.borough)).darker();
            });

        // Months have slightly different lengths so calculate the width for each bin
        // Draw the rectangles, starting from the upper left corner and going down
        borough.selectAll(".bar")
            .data(function(d) {
                return d.values;
            })
          .enter().append("rect")
            .attr("class", "bar")
            .attr("x", function(d) {
                return x(d.x);
            })
            .attr("width", function(d) {
                return x(new Date(d.x.getTime() + d.dx)) - x(d.x) - 2;
            })
            .attr("y", function(d) {
                return y(d.y0 + d.y);
            })
            .attr("height", function(d) {
                return y(d.y0) - y(d.y0 + d.y);
            });

        // Add the X Axis
        svg.append("g")
            .attr("class", "x axis")
            .attr("transform", "translate(0," + height + ")")
            .call(xAxis);

        // Add the Y Axis and label
        svg.append("g")
            .attr("class", "y axis")
            .call(yAxis)
            .append("text")
            .attr("transform", "rotate(-90)")
            .attr("y", 6)
            .attr("dy", ".71em")
            .style("text-anchor", "end")
            .text("Number of Rat Sightings");

        // Add the legend and center it horizontally
        var maxLegendWidth = 110;
        var xStart = (width - maxLegendWidth * color.domain().length) / 2;
        var legend = svg.selectAll(".legend")
            .data(color.domain().slice())
          .enter().append("g")
            .attr("class", "legend")
            .attr("transform", function(d, i) {
                return "translate(" + i * maxLegendWidth + ",0)";
            });

        legend.append("rect")
            .attr("x", xStart)
            .attr("width", 18)
            .attr("height", 18)
            .style("fill", color);

        legend.append("text")
            .attr("x", xStart + 24)
            .attr("y", 9)
            .attr("dy", ".35em")
            .style("text-anchor", "start")
            .text(function(d) {
                return d;
            });

    });
}

// First test doesn't take any parameters and file name is hard coded.
var createScatterTracks = function(labels, colours)
{
    // Clear div.
    document.getElementById("divSVG").innerHTML = ""

    var totalWidth = 380;
    var totalHeight = 240;
    var margin = {top: 40, right: 60, bottom: 50, left: 70},
      width = totalWidth - margin.left - margin.right,
      height = totalHeight - margin.top - margin.bottom;

    /*var margin = {top: 20, right: 20, bottom: 30, left: 40},
        width = 960 - margin.left - margin.right,
        height = 500 - margin.top - margin.bottom;*/

    /*
     * value accessor - returns the value to encode for a given data object.
     * scale - maps value to a visual display encoding, such as a pixel position.
     * map function - maps from data value to display value
     * axis - sets up axis
     */

    // setup x
    var xValue = function(d) { return d["Age (years)"];}, // data -> value
        xScale = d3.scale.linear().range([0, width]), // value -> display
        xMap = function(d) { return xScale(xValue(d));}, // data -> display
        xAxis = d3.svg.axis().scale(xScale).orient("bottom");

    // setup y
    var yValue = function(d) { return d["Number of tracks"];}, // data -> value
        yScale = d3.scale.linear().range([height, 0]), // value -> display
        yMap = function(d) { return yScale(yValue(d));}, // data -> display
        yAxis = d3.svg.axis().scale(yScale).orient("left");

    // setup fill color
    var cValue = function(d) { return d.Type;},
        //color = function(d,i) { return s2plot_colours(i); }; //d3.scale.category10();
        color = d3.scale.ordinal()
            .domain(labels)
            .range(colours)
            //.domain(["Controls", "SympHD", "PreSympHD", "Other"])
            //.domain(["Controls", "SympHD"])
            //.range(["#4cff4c", "#33337f", "#4c4cff", "#33337f" ]);
            //.range(["#39bf39", "#dd842c"]);

    // add the graph canvas to the body of the webpage
    var svg = d3.select("#divSVG").append("svg")
        .attr("width", width + margin.left + margin.right)
        .attr("height", height + margin.top + margin.bottom)
      .append("g")
        .attr("transform", "translate(" + margin.left + "," + margin.top + ")");

    // add the tooltip area to the webpage
    var tooltip = d3.select("body").append("div")
        .attr("class", "tooltip")
        .style("opacity", 0);

    // load data
    d3.csv('ntracks.csv', function(error, data) {

      // change string (from CSV) into number format
      data.forEach(function(d) {
        d["Age (years)"] = +d["Age (years)"];
        d["Number of tracks"] = +d["Number of tracks"];
    //    console.log(d);
      });

      // don't want dots overlapping axis, so add in buffer to data domain
      xScale.domain([d3.min(data, xValue)-1, d3.max(data, xValue)+1]);
      yScale.domain([d3.min(data, yValue), d3.max(data, yValue)+1]);

      // x-axis
      svg.append("g")
          .attr("class", "x axis")
          .attr("fill", "white")
          .attr("transform", "translate(0," + height + ")")
          .call(xAxis)
        .append("text")
          .attr("class", "label")
          .attr("x", totalWidth/2)
          .attr("y", 40)
          .style("text-anchor", "end")
          .text("Age (year)");

      // y-axis
      svg.append("g")
          .attr("class", "y axis")
          .attr("fill", "white")
          .call(yAxis)
        .append("text")
          .attr("class", "label")
          .attr("transform", "rotate(-90)")
          .attr("y", 0 - margin.left)
          .attr("dy", ".80em")
          .style("text-anchor", "end")
          .text("Number of tracks");

      // draw dots
      svg.selectAll(".dot")
          .data(data)
        .enter().append("circle")
          .attr("class", "dot")
          .attr("r", 3.5)
          .attr("cx", xMap)
          .attr("cy", yMap)
          .style("fill", function(d) { return color(cValue(d));})
          .on("mouseover", function(d) {
            var xPosition = parseFloat(d3.select(this).attr("cx")) + (4);
            var yPosition = parseFloat(d3.select(this).attr("cy")) - 10;

            svg.append("text")
              .attr("id", "tooltip")
              .attr("x", xPosition)
              .attr("y", yPosition)
              .attr("fill", "white")
              .attr("text-anchor", "middle")
              .text("ID: " + d["Subject_ID"]);
          })
          .on("mouseout", function(d) {
            d3.select('#tooltip').remove();
          });

      // draw legend
      var legend = svg.selectAll(".legend")
          .data(color.domain())
        .enter().append("g")
          .attr("class", "legend")
          .attr("transform", function(d, i) { return "translate(0," + i * 20 + ")"; });

      // draw legend colored rectangles
      legend.append("rect")
          .attr("x", width - 18)
          .attr("width", 18)
          .attr("height", 18)
          .style("fill", color);

      // draw legend text
      legend.append("text")
          .attr("x", width - 24)
          .attr("y", 9)
          .attr("dy", ".35em")
          .style("text-anchor", "end")
          .text(function(d) { return d;})
    });

}


var linked_areas = function (title,
                               node, panel,
                               mean, stand_dev, min_filter, max_filter)
{
    console.log('linked_areas');
    // Clear div.
    document.getElementById("divSVG").innerHTML = ""

    var margin = { top: 10, right: 40, bottom: 120, left: 40 },
        margin2 = { top: 160, right: 40, bottom: 50, left: 40 },
        width = 400 - margin.left - margin.right,
        height = 240 - margin.top - margin.bottom,
        height2 = 240 - margin2.top - margin2.bottom;

    var x = d3.scale.linear().range([0, width]),
        x2 = d3.scale.linear().range([0, width]),
        y = d3.scale.linear().range([height, 0]),
        y2 = d3.scale.linear().range([height2, 0]);

    var xAxis = d3.svg.axis().scale(x).orient("bottom"),
        xAxis2 = d3.svg.axis().scale(x2).orient("bottom"),
        yAxis = d3.svg.axis().scale(y).orient("left");

    var brush = d3.svg.brush()
        .x(x2)
        .on("brush", brushed)
        .on("brushend", brushend);

    var area = d3.svg.area()
        .interpolate("monotone")
        .x(function (d) { return x(d.bin); })
        .y0(height)
        .y1(function (d) { return y(d.count); });

    var area2 = d3.svg.area()
        .interpolate("monotone")
        .x(function (d) { return x2(d.bin); })
        .y0(height2)
        .y1(function (d) { return y2(d.count); });


    var formatCount = d3.format(",.4f");

    // make some buttons to drive our zoom
    d3.select("#divSVG").append("div")
    .attr("id","btnDiv")
    .style('font-size','75%')
    .style("width","250px")
    .style("position","relative")
    .style("left","-15px")
    .style("top","-10px")

    var btns = d3.select("#btnDiv").selectAll("button").data(["[-3s, +3s]"])
    btns = btns.enter().append("button")
            .attr("class","leftButtons")
    // fill the buttons with the year from the data assigned to them
    btns.each(function (d) {
        this.innerText = d;
    })

    btns.on("click", drawBrush);
    function drawBrush() {
        // define our brush extent to be begin and end of the year
        brush.extent([mean-(3*stand_dev), mean+(3*stand_dev)])

        brush(d3.select(".brush").transition());

        brush.event(d3.select(".brush").transition().delay(1000))
    }

    var svg = d3.select("#divSVG").append("svg")
        .attr("width", width + margin.left + margin.right)
        .attr("height", height + margin.top + margin.bottom);

    svg.append("defs").append("clipPath")
        .attr("id", "clip")
        .append("rect")
        .attr("width", width)
        .attr("height", height);

    var focus = svg.append("g")
        .attr("class", "focus")
        .attr("transform", "translate(" + margin.left + "," + margin.top + ")");

    var context = svg.append("g")
        .attr("class", "context")
        .attr("transform", "translate(" + margin2.left + "," + margin2.top + ")");

    d3.csv("histogram.csv", type, function (error, data) {
        x.domain(d3.extent(data.map(function (d) { return d.bin; })));

        console.log("Min count is ");

        if (d3.min(data.map(function (d) { return d.count; })) != 0)
        {
            console.log("Min count is greater than 0");
            y.domain([d3.min(data.map(function (d) { return d.count; })),
                      d3.max(data.map(function (d) { return d.count; }))]);
        }
        else
        {
            console.log("Min count is 0");
            y.domain([d3.min(data.map(function (d) { return d.count; }))+1e-6,
                      d3.max(data.map(function (d) { return d.count; }))]);
        }
        x2.domain(x.domain());
        y2.domain(y.domain());



        focus.append("path")
            .datum(data)
            .attr("class", "area")
            .attr("d", area);

        focus.append("g")
            .attr("class", "x axis")
            .attr("transform", "translate(0," + height + ")")
            .attr("fill", "white")
            .call(xAxis);

        focus.append("g")
            .attr("class", "y axis")
            .attr("fill", "white")
            .call(yAxis);

        context.append("path")
            .datum(data)
            .attr("class", "area")
            .attr("d", area2);

        context.append("g")
            .attr("class", "x axis")
            .attr("fill", "white")
            .attr("transform", "translate(0," + height2 + ")")
            .call(xAxis2);

        context.append("g")
            .attr("class", "x brush")
            .call(brush)
            .selectAll("rect")
            .attr("y", -6)
            .attr("height", height2 + 7);


        // define our brush extent to be [-3s, +3s]
        brush.extent([min_filter, max_filter]);
        x.domain(brush.empty() ? x2.domain() : brush.extent());
        focus.select(".area").attr("d", area);
        focus.select(".x.axis").call(xAxis);

        // now draw the brush to match our extent
        // use transition to slow it down so we can see what is happening
        // remove transition so just d3.select(".brush") to just draw
        brush(d3.select(".brush"));
    });

    var range_extent;
    var this_panel = panel;
    var this_node = node;
    function brushed() {
        x.domain(brush.empty() ? x2.domain() : brush.extent());
        focus.select(".area").attr("d", area);
        focus.select(".x.axis").call(xAxis);
        range_extent = d3.event.target.extent();
    }

    function brushend() {
        var JSONstring = {'node':this_node,
                         'panel':this_panel,
                         'min_filter':range_extent[0],
                         'max_filter':range_extent[1],
                         'skip_history': 0};
        $j.ajax({
            type: "POST",
            url: SERVER + "/update",
            data: "range_filter="+ encodeURIComponent(JSON.stringify(JSONstring))
        });
    }

    function type(d) {
        d.bin = d.bin;
        d.count = +d.count;
        return d;
    }



}