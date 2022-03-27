//
// Cote UI de l'application "lucioles"
//
// Auteur : G.MENEZ
// RMQ : Manipulation naive (debutant) de Javascript
//
let infoContainer = new InfoContainer();
let chart = new TempChart();

function init() {
    new NavListeners();
    new TempChart();
    let node_url = "http://localhost:3000";
    let log_container = document.getElementById("logs_container")
    let ident_list = [];
    let esp_list = [];
    let log_list = [];
    let new_esp;
    let api_sources_list = [];


    getApiSourcesFromNode();
    //
    setInterval(() => {
        refreshLogs();
        waitingGif();
        getApiDataFromNode();
        getApiSourcePred();
        checkList(); // on compare notre liste d'esps avec celle du serveur et on ajoute les idents des nouveaux
        getLastDataForEsps(); // On récupère les dernières données pour notre liste d'esps
        handleMarkers();
        waitingGif();
        },2000)

    // setInterval(()=>{
    //     checkList(); // on compare notre liste d'esps avec celle du serveur et on ajoute les idents des nouveaux
    //     getLastDataForEsps();
    //     getApiDataFromNode()// On récupère les dernières données pour notre liste d'esps
    //     handleMarkers();
    //     },60000)

    function handleMarkers(){
        let liste = esp_list.concat(api_sources_list);
        liste.forEach((source)=> {
            if(source.getMarker()){
                source.refreshDataMarker();
            } else if(source.haslocalisation()) {
                source.setUpMarker();
                if(!(source.getCity() && source.getCountry()))
                source.getLocationName();
            };
        })
    }

    function waitingGif() {
        let isLoading = false;
        let liste = esp_list.concat(api_sources_list);
        if (liste.length > 0) {
            liste.forEach((esp) => {
                if (!(!!esp.getMarker())) {
                    isLoading = true;
                }
            })
            if (!isLoading) {
                document.getElementById("loading-screen").classList.add("hidden");
            }
        }

    }
    function getLastDataForEsps(){
        esp_list.forEach((esp)=> {
            getDataForEsp(esp);
        })
    }

    function getDataForEsp(esp){
        $.ajax({
            url: node_url.concat("/esp/temp?who="+esp.getIdent()),
            type: 'GET',
            headers: { Accept: "application/json", },
            success: function (resultat, statut) {
                esp.setData(resultat);
                esp.extractLastData();
            },
            error: function (resultat, statut, erreur) {
            },
            complete: function (resultat, statut) {
            }
        });

    }

    function checkList(){
        $.ajax({
            url: node_url.concat("/esp/list"), // URL to "GET" : /esp/temp ou /esp/light
            type: 'GET',
            headers: { Accept: "application/json", },
            //data: {"who": wh}, // parameter of the GET request
            success: function (resultat, statut) { // Anonymous function on success
                if(resultat.length){
                    resultat.forEach((esp)=> {
                        if(!ident_list.includes(esp.identification)){
                            new_esp = new dataSource(esp.user,esp.identification,"esp");
                            ident_list.push(esp.identification);
                            esp_list.push(new_esp);
                        }
                    })
                }
                //     resultat.forEach(function (element) {
                // listeData.push([Date.parse(element.date),element.value]);
                //     });
                //     serie.setData(listeData); //serie.redraw();
            },
            error: function (resultat, statut, erreur) {
            },
            complete: function (resultat, statut) {
            }
        });

    }

    function getApiSourcePred() {

        api_sources_list.forEach((source) => {
            $.ajax({
                url: node_url.concat("/api/predict?city=" + source.getCity()),
                type: 'GET',
                headers: {Accept: "application/json",},
                success: function (resultat, statut) {
                    if (resultat[0].pred) {
                        let predict = [];
                        resultat[0].pred.forEach((pred) => {
                            predict.push({"dt": pred.dt * 1000, "temp": pred.temp})
                        });
                        source.setPred(predict);
                    }
                },
                error: function (resultat, statut, erreur) {
                },
                complete: function (resultat, statut) {
                }
            });
        })
    }

    function refreshLogs(){
        $.ajax({
            url: node_url.concat("/esp/logs"),
            type: 'GET',
            headers: { Accept: "application/json", },
            success: function (resultat, statut) {
                if(resultat.length){
                    resultat.forEach((log) => {
                        if(!log_list.includes(JSON.stringify(log))){
                            log_list.push(JSON.stringify(log));
                            log_container.innerHTML += log.date+": "+log.msg+"<br>";
                        }
                    })
                }
            },
            error: function (resultat, statut, erreur) {
            },
            complete: function (resultat, statut) {
            }
        });
    }

    function getApiSourcesFromNode(){
        $.ajax({
            url: node_url.concat("/api/locations"),
            type: 'GET',
            headers: { Accept: "application/json", },
            success: function (resultat, statut) {
                resultat.forEach((source)=> {
                    let newSource = new dataSource(source.city,null,"api");
                    newSource.setLat(source.lat);
                    newSource.setLong(source.long);
                    newSource.setCountry(source.country);
                    newSource.setCity(source.city);
                    api_sources_list.push(newSource);
                })
            },
            error: function (resultat, statut, erreur) {
            },
            complete: function (resultat, statut) {
            }
        });
    }

    function getApiDataFromNode(){
        api_sources_list.forEach((source) => {
            $.ajax({
                url: node_url.concat("/api/temp?city="+source.getCity()),
                type: 'GET',
                headers: { Accept: "application/json", },
                success: function (resultat, statut) {
                    source.setData(resultat);
                    source.extractLastData();
                },
                error: function (resultat, statut, erreur) {
                },
                complete: function (resultat, statut) {
                }
            });
        })
    }
}


//assigns the onload event to the function init.
//=> When the onload event fires, the init function will be run. 
window.onload = init;


