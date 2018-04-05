var DEBUG = 0;
var Clay = require('pebble-clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig);

// Listen for when the watchface is opened
Pebble.addEventListener('ready',
  function(e) {
    if (DEBUG) console.log("PebbleKit JS ready!");
  }
);
