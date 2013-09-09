var socket;
var statusDiv;

$(document).ready( function() {
	setupSocket();
	
	document.getElementById("brow").textContent = " " + BrowserDetect.browser + " " + BrowserDetect.version +" " + BrowserDetect.OS +" ";
	statusDiv = document.getElementById("status");

});


// setup web socket
function setupSocket(){

	// setup websocket
	// get_appropriate_ws_url is a nifty function by the libwebsockets people
	// it decides what the websocket url is based on the broswer url
	// e.g. https://mygreathost:9099 = wss://mygreathost:9099

	if (BrowserDetect.browser == "Firefox") {
		socket = new MozWebSocket(get_appropriate_ws_url());
	} else {
		socket = new WebSocket(get_appropriate_ws_url());
	}

	socket.binaryType = "arraybuffer";

	
	// open
	try {
		socket.onopen = function() {
			statusDiv.style.backgroundColor = "#40ff40";
			statusDiv.textContent = " websocket connection opened ";
		} 

		// received message
		socket.onmessage = function got_packet(msg) {

			if (msg.data instanceof ArrayBuffer) {
				appDrawImage(msg);
			}

			else
			{
				var jsonData;
				try {
					jsonData = JSON.parse(msg.data);
					appUpdate(jsonData);
				} catch( e ){
					console.log(e);
				}
			}
		}

		socket.onclose = function(){
			statusDiv.style.backgroundColor = "#ff4040";
			statusDiv.textContent = " websocket connection CLOSED ";
		}
	} catch(exception) {
		alert('<p>Error' + exception);  
	}
}