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
        getApiDataFromNode()
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
        if (esp_list.length > 0) {
            esp_list.forEach((esp) => {
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
                //console.log(resultat.map((test) => ({"date":new Date().getTime(),"value": test.value})));
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
                let listeData = resultat;
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
    //=== Initialisation des traces/charts de la page html ===
    // Apply time settings globally
    // Highcharts.setOptions({
	// global: { // https://stackoverflow.com/questions/13077518/highstock-chart-offsets-dates-for-no-reason
    //         useUTC: false,
    //         type: 'spline'
	// },
	// time: {timezone: 'Europe/Paris'}
    // });
    // // cf https://jsfiddle.net/gh/get/library/pure/highcharts/highcharts/tree/master/samples/highcharts/demo/spline-irregular-time/
    // chart1 = new Highcharts.TempChart({
    //     title: {text: 'Temperatures'},
	// subtitle: { text: 'Irregular time data in Highcharts JS'},
    //     legend: {enabled: true},
    //     credits: false,
    //     chart: {renderTo: 'container1'},
    //     xAxis: {title: {text: 'Heure'}, type: 'datetime'},
    //     yAxis: {title: {text: 'Temperature (Deg C)'}},
    //     series: [{name: 'ESP1', data: []},
	// 	 {name: 'ESP2', data: []},
	// 	 {name: 'ESP3', data: []}],
	// //colors: ['#6CF', '#39F', '#06C', '#036', '#000'],
	// colors: ['red', 'green', 'blue'],
    //     plotOptions: {line: {dataLabels: {enabled: true},
	// 		     //color: "red",
	// 		     enableMouseTracking: true
	// 		    }
	// 	     }
    // });
    // chart2 = new Highcharts.TempChart({
    //     title: { text: 'Lights'},
    //     legend: {title: {text: 'Lights'}, enabled: true},
    //     credits: false,
    //     chart: {renderTo: 'container2'},
    //     xAxis: {title: {text: 'Heure'},type: 'datetime'},
    //     yAxis: {title: {text: 'Lumen (Lum)'}},
	// series: [{name: 'ESP1', data: []},
	// 	 {name: 'ESP2', data: []},
	// 	 {name: 'ESP3', data: []}],
	// //colors: ['#6CF', '#39F', '#06C', '#036', '#000'],
	// colors: ['red', 'green', 'blue'],
    //     plotOptions: {line: {dataLabels: {enabled: true},
	// 		     enableMouseTracking: true
	// 		    }
	// 	     }
    // });

//     //=== Gestion de la flotte d'ESP =================================
//     var which_esps = [
// 	"80:7D:3A:FD:E8:48"
// //	,"1761716416"
// //	"80:7D:3A:FD:C9:44"
//     ]
//
//     for (var i = 0; i < which_esps.length; i++) {
// 	process_esp(which_esps, i)
//     }
// };
//
//
// //=== Installation de la periodicite des requetes GET============
// function process_esp(which_esps,i){
//     const refreshT = 10000 // Refresh period for chart
//     esp = which_esps[i];    // L'ESP "a dessiner"
//     //console.log(esp) // cf console du navigateur
//
//     // Gestion de la temperature
//     // premier appel pour eviter de devoir attendre RefreshT
//     get_samples('/esp/temp', chart1.series[i], esp);
//     //calls a function or evaluates an expression at specified
//     //intervals (in milliseconds).
//     window.setInterval(get_samples,
// 		       refreshT,
// 		       '/esp/temp',     // param 1 for get_samples()
// 		       chart1.series[i],// param 2 for get_samples()
// 		       esp);            // param 3 for get_samples()
//
//     // Gestion de la lumiere
//     get_samples('/esp/light', chart2.series[i], esp);
//     window.setInterval(get_samples,
// 		       refreshT,
// 		       '/esp/light',     // URL to GET
// 		       chart2.series[i], // Serie to fill
// 		       esp);             // ESP targeted
// }
//
//
// //=== Recuperation dans le Node JS server des samples de l'ESP et
// //=== Alimentation des charts ====================================
// function get_samples(path_on_node, serie, wh){
//     // path_on_node => help to compose url to get on Js node
//     // serie => for choosing chart/serie on the page
//     // wh => which esp do we want to query data
//
//     //node_url = 'http://localhost:3000'
//     node_url = 'http://134.59.131.45:3000'
//     //node_url = 'http://192.168.1.101:3000'
//
//     //https://openclassrooms.com/fr/courses/1567926-un-site-web-dynamique-avec-jquery/1569648-le-fonctionnement-de-ajax
//     $.ajax({
//         url: node_url.concat(path_on_node), // URL to "GET" : /esp/temp ou /esp/light
//         type: 'GET',
//         headers: { Accept: "application/json", },
// 	data: {"who": wh}, // parameter of the GET request
//         success: function (resultat, statut) { // Anonymous function on success
//             let listeData = [];
//             resultat.forEach(function (element) {
// 		listeData.push([Date.parse(element.date),element.value]);
//             });
//             serie.setData(listeData); //serie.redraw();
//         },
//         error: function (resultat, statut, erreur) {
//         },
//         complete: function (resultat, statut) {
//         }
//     });
}


//assigns the onload event to the function init.
//=> When the onload event fires, the init function will be run. 
window.onload = init;


