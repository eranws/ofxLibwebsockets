var app;
var canvas;
var context;

var canvasData;
var data;
var type = 1;

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
	var pos = evt.position.cam;

	context.clearRect(0, 0, canvas.width, canvas.height);

	context.fillStyle = evt.grab ? 'green' : 'red';
	context.fillRect(pos[0] * canvas.width, pos[1] * canvas.height, 10, 10);
};


function appDrawImage( messageEvent ){
	var image = new Image();
	data = canvasData.data;

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

	context.putImageData(canvasData,0,0);
};