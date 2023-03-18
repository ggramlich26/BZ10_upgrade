function onLoad(){
	settings_onLoad();
	mlr_onLoad();
}

function onLanguageChanged(){
}

function toggleDisplay(id) {
	element = document.getElementById(id);
	if(element.style.display === "none" || element.style.display === "")
		element.style.display = "block";
	else
		element.style.display = "none";
}
