var app;
var canvas;
var context;

var canvasData;
var data;
var type = 1;
var	imageTimestamp = new Date().getTime();


var handPos = [];
var handGrab = false;
var positions = [];


// http://paulirish.com/2011/requestanimationframe-for-smart-animating/
// http://my.opera.com/emoller/blog/2011/12/20/requestanimationframe-for-smart-er-animating

// requestAnimationFrame polyfill by Erik Möller
// fixes from Paul Irish and Tino Zijdel

(function() {
    var lastTime = 0;
    var vendors = ['ms', 'moz', 'webkit', 'o'];
    for(var x = 0; x < vendors.length && !window.requestAnimationFrame; ++x) {
        window.requestAnimationFrame = window[vendors[x]+'RequestAnimationFrame'];
        window.cancelAnimationFrame = window[vendors[x]+'CancelAnimationFrame'] 
                                   || window[vendors[x]+'CancelRequestAnimationFrame'];
    }
 
    if (!window.requestAnimationFrame)
        window.requestAnimationFrame = function(callback, element) {
            var currTime = new Date().getTime();
            var timeToCall = Math.max(0, 16 - (currTime - lastTime));
            var id = window.setTimeout(function() { callback(currTime + timeToCall); }, 
              timeToCall);
            lastTime = currTime + timeToCall;
            return id;
        };
 
    if (!window.cancelAnimationFrame)
        window.cancelAnimationFrame = function(id) {
            clearTimeout(id);
        };
}());

// Properties _____________________________________________

var fps = 30;
var interval = 1000 / fps;


// Animation Logic ________________________________________

function draw() {
    setTimeout(function() {
        window.requestAnimationFrame(draw);
        
	context.clearRect(0, 0, canvas.width, canvas.height);
	
	if (new Date().getTime() - imageTimestamp < 500)
	{
		context.putImageData(canvasData,0,0);
	}	

	context.fillStyle = handGrab ? 'green' : 'red';
	context.fillRect(handPos[0] * canvas.width, handPos[1] * canvas.height, 10, 10);

    }, interval);
}

draw();




$(document).ready( function() {
	canvas = document.getElementById('canvas');
	context = canvas.getContext('2d');
	canvasData = context.getImageData(0,0,canvas.width, canvas.height);
	data = canvasData.data;

	context.fillStyle = 'gray';
	context.fillRect(0, 0, canvas.width, canvas.height);
});


function appUpdate(evt){
//	console.log(evt);
	if (!evt.positions) return;

	positions = [];
	for (var i in evt.positions)
	{
		positions[i] = {
		 "x" : evt.positions[i].real[0],
		 "y" : evt.positions[i].real[1],
		 "z" : evt.positions[i].real[2]
		}
	}
 	//handGrab = evt.grab;
// 	console.log(evt.position.real);

};


function appDrawImage( messageEvent ){
	var image = new Image();
	data = canvasData.data;
	imageTimestamp = new Date().getTime();


	var bytearray = new Uint8Array( messageEvent.data );
	var index = 0;

	// b & w
	if ( type == 0 ){
		for (var i = 0; i < data.length; i+=4) {
		   	data[i] = bytearray[index]; 
			data[i + 1] = bytearray[index];
			data[i + 2] = bytearray[index];
			data[i + 3] = 255;
			index++;
		}
	// rgb
	} else if ( type == 1 ){
		for (var i = 0; i < data.length; i+=4) {
			data[i] = bytearray[index]; index++;
			data[i + 1] = bytearray[index]; index++;
			data[i + 2] = bytearray[index]; index++;
			data[i + 3] = 255;
		}
	
	// rgba
	} else {
		console.log( bytearray.length );
		console.log( data.length );
		for (var i = 0; i < bytearray.length; i++) {
			data[i] = bytearray[i]
		}
	}

};