var mlrLangInUse; //country code

function mlr_onLoad(lang) {
	if(lang != "en" && lang != "de"){
		lang = "en";
	}
	mlr({
		dropID: "settingsMLDrop",
		stringAttribute: "data-mlr-id",
		chosenLang: lang,
		mLstrings: MLstrings,
		countryCodes: true,
	});
}

var mlr = function({
    dropID = "settingsMLDrop",
    stringAttribute = "data-mlr-id",
    chosenLang = "en", //country code of chosen language
    mLstrings = MLstrings,
    countryCodes = false,
} = {}) {
    const root = document.documentElement;

//    var listOfLanguages = Object.keys(mLstrings[0]);
    mlrLangInUse = chosenLang;

    (function createMLDrop() {
        var mbPOCControlsLangDrop = document.getElementById(dropID);
        // Reset the menu
        mbPOCControlsLangDrop.innerHTML = "";
        // Now build the options
        mlCodes.forEach((lang, langidx) => {
            let HTMLoption = document.createElement("option");
            HTMLoption.value = lang.id;
            HTMLoption.textContent = lang.name;
            mbPOCControlsLangDrop.appendChild(HTMLoption);
            if (lang.id === chosenLang) {
                mbPOCControlsLangDrop.value = lang.id;
            }
        });
        mbPOCControlsLangDrop.addEventListener("change", mlOnLangChanged);
    })();

	function mlOnLangChanged(e){
        var mbPOCControlsLangDrop = document.getElementById(dropID);
		mlrLangInUse = mbPOCControlsLangDrop[mbPOCControlsLangDrop.selectedIndex].value;
		resolveAllMLStrings();
		// Here we update the 2-digit lang attribute if required
		if (countryCodes === true) {
			root.setAttribute("lang", mlCodes.find((language)=>{return language.id===mlrLangInUse}).code);
		}
		onLanguageChanged(); //call onLanguageChanged function from main.js
	}
	mlOnLangChanged(null);

    function resolveAllMLStrings() {
        let stringsToBeResolved = document.querySelectorAll(`[${stringAttribute}]`);
        stringsToBeResolved.forEach(stringToBeResolved => {
            //let originaltextContent = stringToBeResolved.textContent;
            let mlrid = stringToBeResolved.getAttribute(stringAttribute);
            let resolvedText = resolveMLString(mlrid, mLstrings);
            stringToBeResolved.innerHTML = resolvedText;
        });
    }
};

function mlr_resolve(id){
	return resolveMLString(id, MLstrings);
}

function resolveMLString(id, mLstrings) {
	var stringObject = mLstrings[id];
	if(stringObject){
		var translation = stringObject[mlrLangInUse];
		if(translation)
			return translation;
	}
	return "<span style='color: red;'><b>Translation not found</b></span>";
}
