var app;
var canvas;
var context;

$(document).ready( function() {
	canvas = document.getElementById('canvas');
	context = canvas.getContext('2d');

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