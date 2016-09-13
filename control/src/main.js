/* @preserve
 *
 * main.js
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
