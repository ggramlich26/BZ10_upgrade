function saveTime() {
    let wakeupTime = document.getElementById("wakeupTime").value;
    let standbyTime = document.getElementById("standbyTime").value;
	console.log("wakeupTime: " + wakeupTime + ", standbyTime: " + standbyTime);
	var data = {'wakeupTime':wakeupTime, 'standbyTime':standbyTime};
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.onreadystatechange = function() { 
		if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
			console.log(xmlHttp.responseText);
	}
	xmlHttp.open("POST", "setSettingsData", true); // true for asynchronous 
	xmlHttp.setRequestHeader('Content-Type', 'application/json');
	xmlHttp.send(JSON.stringify(data));
}

function saveMachineSettings() {
	let boilerP = document.getElementById("boilerP").value;
	let boilerI = document.getElementById("boilerI").value;
	let boilerD = document.getElementById("boilerD").value;
	let BUP = document.getElementById("BUP").value;
	let BUI = document.getElementById("BUI").value;
	let BUD = document.getElementById("BUD").value;
	let boilerFlowmeter = document.getElementById("boilerFlowmConv").value;
	let bypassFlowmeter = document.getElementById("bypassFlowmConv").value;
	var data = {'boilerP':boilerP, 'boilerI':boilerI, 'boilerD':boilerD, 
			'BUP':BUP, 'BUI':BUI, 'BUD':BUD, 
			'boilerFlowmConv':boilerFlowmeter, 'bypassFlowmConv':bypassFlowmeter};
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.onreadystatechange = function() { 
		if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
			console.log(xmlHttp.responseText);
	}
	xmlHttp.open("POST", "setSettingsData", true); // true for asynchronous 
	xmlHttp.setRequestHeader('Content-Type', 'application/json');
	xmlHttp.send(JSON.stringify(data));
}

function settings_onLoad(){
 	//request mode from ESP
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.onreadystatechange = function() { 
		if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
			settings_onDataReceived(xmlHttp.responseText);
	}
	xmlHttp.open("GET", "getSettings?rando="+Math.random(), true); // true for asynchronous 
	xmlHttp.send(null);
	document.getElementById("settingsWifiPW").value = 'xxxxxx';
}

function settings_onDataReceived(dataJSON){
	console.log(dataJSON);
	let data = JSON.parse(dataJSON);
	if ('wifiSSID' in data){
		document.getElementById("settingsWifiSSID").value = data.wifiSSID;
    }
    if ('wifiPW' in data){
		document.getElementById("settingsWifiPW").value = data.wifiPW;
    }
    if ('bonjourName' in data){
    	document.getElementById("settingsWifiBonjour").value = data.bonjourName;
    }
    if ('language' in data){
		mlr_onLoad(data.language);
    }
	if('boilerP' in data){
    	document.getElementById("boilerP").value = data.boilerP;
	}
	if('boilerI' in data){
    	document.getElementById("boilerI").value = data.boilerI;
	}
	if('boilerD' in data){
    	document.getElementById("boilerD").value = data.boilerD;
	}
	if('BUP' in data){
    	document.getElementById("BUP").value = data.BUP;
	}
	if('BUI' in data){
    	document.getElementById("BUI").value = data.BUI;
	}
	if('BUD' in data){
    	document.getElementById("BUD").value = data.BUD;
	}
	if('boilerFlowmConv' in data){
    	document.getElementById("boilerFlowmConv").value = data.boilerFlowmConv;
	}
	if('bypassFlowmConv' in data){
    	document.getElementById("bypassFlowmConv").value = data.bypassFlowmConv;
	}
	if('wakeupTime' in data){
    	document.getElementById("wakeupTime").value = data.wakeupTime;
	}
	if('standbyTime' in data){
    	document.getElementById("standbyTime").value = data.standbyTime;
	}
	if('boilerTemp' in data){
    	document.getElementById("boilerTemp").value = data.boilerTemp;
	}
	if('BUTemp' in data){
    	document.getElementById("headTemp").value = data.BUTemp;
	}
	if('volOffset' in data){
    	document.getElementById("volOffset").value = data.volOffset;
	}
	if('volDist' in data){
    	document.getElementById("volDist").value = data.volDist;
	}
	if('preinfBuildup' in data){
    	document.getElementById("preinfBuildup").value = data.preinfBuildup;
	}
	if('preinfWait' in data){
    	document.getElementById("preinfWait").value = data.preinfWait;
	}
}

function saveBrewingParams(){
    let tBoiler = document.getElementById("boilerTemp").value;
    let tBU = document.getElementById("headTemp").value;
    let volOffset = document.getElementById("volOffset").value;
    let volDist = document.getElementById("volDist").value;
    let preinfBuildup = document.getElementById("preinfBuildup").value;
    let preinfWait = document.getElementById("preinfWait").value;
    console.log("Boiler temp: " + tBoiler + ", BU temp: " + tBU + ", volume Offset: " + volOffset +
 	", distribution Volume: " + volDist + ", preinfusion Buildup time: " + preinfBuildup +
	", preinfusion wait time: " + preinfWait);
    var data = {'boilerTemp':tBoiler, 'BUTemp':tBU, 'volOffset':volOffset, 'volDist':volDist,
				'preinfBuildup':preinfBuildup, 'preinfWait':preinfWait};
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.onreadystatechange = function() { 
		if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
			console.log(xmlHttp.responseText);
	}
	xmlHttp.open("POST", "setSettingsData", true); // true for asynchronous 
	xmlHttp.setRequestHeader('Content-Type', 'application/json');
	xmlHttp.send(JSON.stringify(data));
}

function saveWifi() {
     let ssid = document.getElementById("settingsWifiSSID").value;
     let pw = document.getElementById("settingsWifiPW").value;
     let bonjour = document.getElementById("settingsWifiBonjour").value;
	console.log("ssid: " + ssid + ", pw: " + pw + " bonjour: " + bonjour);
	var data;
	if(pw != 'xxxxxx'){
    	data = {'wifiSSID':ssid, 'wifiPW':pw, 'bonjourName':bonjour};
     }
     else {
    	data = {'wifiSSID':ssid, 'bonjourName':bonjour};
     }
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.onreadystatechange = function() { 
		if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
			console.log(xmlHttp.responseText);
	}
	xmlHttp.open("POST", "setSettingsData", true); // true for asynchronous 
	xmlHttp.setRequestHeader('Content-Type', 'application/json');
	xmlHttp.send(JSON.stringify(data));
}

function saveGeneralSettings() {
	let lang = mlrLangInUse;
	console.log(", language: " + lang);
    var data = {'language':lang};
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.onreadystatechange = function() { 
		if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
			console.log(xmlHttp.responseText);
	}
	xmlHttp.open("POST", "setSettingsData", true); // true for asynchronous 
	xmlHttp.setRequestHeader('Content-Type', 'application/json');
	xmlHttp.send(JSON.stringify(data));
}
