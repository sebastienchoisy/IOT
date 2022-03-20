class NavListeners {
    constructor() {
        this.initVariables();
        this.initMapButtonListener();
        this.initDocButtonListener();
        this.initLogsButtonListener();
    }

    initVariables(){
        this.mapButton = document.getElementById("map_button");
        this.mapButton.setAttribute('disabled','disabled');
        this.docButton = document.getElementById("doc_button");
        this.logsButton = document.getElementById("logs_button");
        this.mapContainer = document.getElementById("map_container");
        this.docContainer = document.getElementById("documentation_container");
        this.logsContainer = document.getElementById("logs_container");
        this.mapButton.setAttribute('disabled','disabled');
        this.docContainer.classList.add("hidden");
        this.logsContainer.classList.add("hidden");
    }

    // Init for map button
    initMapButtonListener(){
        this.mapButton.addEventListener("click", ()=> {
            this.docButton.removeAttribute('disabled');
            this.logsButton.removeAttribute('disabled');
            this.mapContainer.classList.remove("hidden");
            this.docContainer.classList.add("hidden");
            this.logsContainer.classList.add("hidden");
        })
    }

    // Init for doc button
    initDocButtonListener(){
        this.docButton.addEventListener("click", ()=> {
            this.docButton.setAttribute('disabled','disabled');
            this.mapButton.removeAttribute('disabled');
            this.logsButton.removeAttribute('disabled');
            this.docContainer.classList.remove("hidden");
            this.mapContainer.classList.add("hidden");
            this.logsContainer.classList.add("hidden");
        })
    }

    // Init for logs button
    initLogsButtonListener(){
        this.logsButton.addEventListener("click", ()=> {
            this.logsButton.setAttribute('disabled','disabled');
            this.docButton.removeAttribute('disabled');
            this.mapButton.removeAttribute('disabled');
            this.logsContainer.classList.remove("hidden");
            this.docContainer.classList.add("hidden");
            this.mapContainer.classList.add("hidden");
        })
    }
}