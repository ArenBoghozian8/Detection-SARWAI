/**
 * USAGE:
 * node parse-world-file.js <filename of world xml or sdf file> <filename of output people data> <filename of output wall data>
 * people filename defaults to people.json
 * walls filename defaults to walls.json
 */

var fs = require('fs');
var xml = require('xml2js');

if (process.argv.length < 3) {
  console.error("Need to have the input SDF or XML file as an argument");
  process.exit();
}

var worldFilename = process.argv[2];
var peopleFilename = process.argv[3] || 'people.json';
var wallFilename = process.argv[4] || 'walls.json';

GenerateFiles(worldFilename, peopleFilename, wallFilename);

function GenerateFiles(worldFilename, destPeopleFilename, destWallFilename) {
  var sdfContents = fs.readFileSync(worldFilename, 'utf8');
  
  var parseString = require('xml2js').parseString;
  parseString(sdfContents, function(err, result) {
    // the JSON data we're going to operatoe on
    var data = JSON.parse(JSON.stringify(result));
    
    var people = JSON.stringify(getPeople(data));
    var walls = JSON.stringify(getWalls(data));
    fs.writeFileSync(destPeopleFilename, people);
    fs.writeFileSync(destWallFilename, walls);
    
    // writeStream.writeFile
    // fs.writeFile(destPeopleFilename, JSON.stringify(people), function(err) {
    //   if (err) {
    //     console.error(err);
    //     process.exit();
    //   }
    // });
    // fs.writeFileSync(destPeopleFilename, JSON.stringify(people));
    // fs.writeSync(destWallFilename, JSON.stringify(walls));
  });
}


function getWalls(data) {
  /**
   * Walls that denote doorways will have a smaller Z size and a higher Z position.
   * @param {*} px centroid position X relative to the parent Link's position and rotation
   * @param {*} py centroid position Y relative to the parent Link's position and rotation
   * @param {*} pz centroid position Z relative to the parent Link's position and rotation
   * @param {*} gx model size
   * @param {*} gy model size
   * @param {*} gz model size
   */
  var SubWall = function(px, py, pz, gx, gy, gz) {
    this.px = px;
    this.py = py;
    this.pz = pz;
    this.gx = gx;
    this.gy = gy;
    this.gz = gz;
  }

  // A link contains one or more visual parts of a wall.
  // The subwall positions and yaw (rotation along a vertical axis)
  /**
   * 
   * @param {*} px centroid position X
   * @param {*} py centroid position Y
   * @param {*} pz centroid position Z
   * @param {*} yaw rotation on a vertical axis of the entire object and its sub-objects
   */
  var Link = function(px, py, pz, yaw) {
    this.px = px;
    this.py = py;
    this.pz = pz;
    this.yaw = yaw;

    // Array of SubWalls
    this.subwalls = new Array();
  }
  var returnLinks = new Array();

  for (var i = 0; i < data.sdf.world[0].model.length; i++) {

    var model = data.sdf.world[0].model[i];

    var name = model.$.name.toLowerCase();
    var prefix = name.substring(0,9) 

    if (prefix == 'jacaranda') {
      var links = model.link;

      for (var j = 0; j < links.length; j++) {
        var dataLink = links[j];
        var pose = dataLink.pose[0]._;
        var p = pose.split(" ");
        var link = new Link(p[0], p[1], p[2], p[5]);
        var visuals = dataLink.visual;
        for (var k = 0; k < visuals.length; k++) {
          var dataVisual = visuals[k];
          var pArr = dataVisual.pose[0]._.split(" ");
          var gArr = dataVisual.geometry[0].box[0].size[0].split(" ");
          var subwall = new SubWall(pArr[0], pArr[1], pArr[2], gArr[0], gArr[1], gArr[2]);
          link.subwalls.push(subwall);
        }
        
        returnLinks.push(link);
      }

      return returnLinks;
    }
  }
}

function getPeople(data) {

  /**
   * 
   * @param {*} x 
   * @param {*} y 
   * @param {*} z 
   */
  var Person = function(x, y, z) {
    this.x = x;
    this.y = y;
    this.z = z;
  }

  var people = new Array();
  for (var i = 0; i < data.sdf.world[0].model.length; i++) {
    var model = data.sdf.world[0].model[i];
    var name = model.$.name;
    var nameArr = name.split("_");
    if (nameArr[0] != "person") {
      continue;
    }

    // here there be people
    var pose = model.pose[0]._;
    var coords = pose.split(" ");
    var x = coords[0];
    var y = coords[1];
    var z = coords[2];
    var p = new Person(x, y, z);
    people.push(p);

  }

  return people;
}
