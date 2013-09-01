var app;

$(document).ready( function() {
	var example = document.getElementById('example');
	var context = example.getContext('2d');
	context.fillStyle = 'red';
	context.fillRect(30, 30, 50, 50);
});

function appUpdate(evt){
	console.log(evt);
};