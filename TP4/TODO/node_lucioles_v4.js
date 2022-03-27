
const path = require('path');

// var, const, let :
// https://medium.com/@vincent.bocquet/var-let-const-en-js-quelles-diff%C3%A9rences-b0f14caa2049
//====================================
// Utilisation du framework express
// Notamment g�r�r les routes
const express = require('express');
const fetch = (...args) => import('node-fetch').then(({default: fetch}) => fetch(...args));
// et pour permettre de parcourir les body des requetes
const bodyParser = require('body-parser');

const app = express();

app.use(bodyParser.urlencoded({ extended: true }))
app.use(bodyParser.json())
app.use(express.static(__dirname+'/public'));
app.use(function(request, response, next) { //Pour eviter les problemes de CORS/REST
	response.header("Access-Control-Allow-Origin", "*");
	response.header("Access-Control-Allow-Headers", "*");
	response.header("Access-Control-Allow-Methods", "POST, GET, OPTIONS, PUT, DELETE");
	next();
});
app.listen(3000, () => {
	console.log('Server listening on port 3000');
});

//--- MQTT module
const mqtt = require('mqtt')
// Topics MQTT
//const TOPIC_LIGHT = 'IOTSHAN/light'
const TOPIC_TEMP  = 'IOTSHAN/temp'
//const TOPIC_AUTH = 'IOTSHAN/auth'


//---  The MongoDB module exports MongoClient, and that's what
// we'll use to connect to a MongoDB database.
// We can use an instance of MongoClient to connect to a cluster,
// access the database in that cluster,
// and close the connection to that cluster.
const {MongoClient} = require('mongodb');

//----------------------------------------------------------------
// This function will retrieve a list of databases in our cluster and
// print the results in the console.
async function listDatabases(client){
	let databasesList = await client.db().admin().listDatabases();
    
    console.log("Databases in Mongo Cluster : \n");
    databasesList.databases.forEach(db => console.log(` - ${db.name}`));
}



//----------------------------------------------------------------
// asynchronous function named main() where we will connect to our
// MongoDB cluster, call functions that query our database, and
// disconnect from our cluster.
async function v0(){

    const mongoName = "lucioles"                   //Nom de la base
    //const mongoUri = 'mongodb://localhost:27017/'; //URL de connection
    //const mongoUri = 'mongodb://10.9.128.189:27017/'; //URL de connection		
    const mongoUri = 'mongodb+srv://Sebastien:IOTTP42022@cluster0.6ocof.mongodb.net/lucioles?retryWrites=true&w=majority';

    //Now that we have our URI, we can create an instance of MongoClient.
    const mg_client = new MongoClient(mongoUri,
				      {useNewUrlParser:true, useUnifiedTopology:true});

    // Connect to the MongoDB cluster
    mg_client.connect(function(err,  mg_client){
	if(err) throw err; // If connection to DB failed ... 
   
	//===============================================    
	// Print databases in our cluster
	listDatabases(mg_client);

	//===============================================    
	// Get a connection to the DB "lucioles" or create
		let dbo = mg_client.db(mongoName);

	// // Remove "old collections : temp and light
	// dbo.listCollections({name: "temp"})
	//     .next(function(err, collinfo) {
	// 	if (collinfo) { // The collection exists
	// 	    //console.log('Collection temp already exists');
	// 	    dbo.collection("temp").drop()
	// 	}
	//     });
	//
	// 	dbo.listCollections({name: "logs"})
	// 		.next(function (err, collinfo) {
	// 			if (collinfo) { // The collection exists
	// 				//console.log('Collection temp already exists');s
	// 				dbo.collection("logs").drop()
	// 			}
	// 		});
	//
		dbo.listCollections({name: "api"})
			.next(function (err, collinfo) {
				if (collinfo) { // The collection exists
					//console.log('Collection temp already exists');
					dbo.collection("api").drop()
				}
			});
	//
	// 	dbo.listCollections({name: "esp"})
	// 		.next(function(err, collinfo) {
	// 			if (collinfo) { // The collection exists
	// 				//console.log('Collection temp already exists');
	// 				dbo.collection("esp").drop()
	// 			}
	// 		});
	// On rempli notre collection API avec les données de l'api OpenWeather
		let data = require("./public/assets/api-locations.json");
		let apikey = "31d8779b48d2fbbe7e1e2183ea13041a";
		async function getCurrentDataFromAPI(lat,long){
			let url = "https://api.openweathermap.org/data/2.5/onecall?lat="+lat+"&lon="+long+"&exclude=hourly,daily,minutely,alerts&units=metric&appid="+apikey;
			return await fetch(url).then((response) => response.json());
		}
		async function getPredictDataFromAPI(lat,long){
			let url = "https://api.openweathermap.org/data/2.5/onecall?lat="+lat+"&lon="+long+"&exclude=hourly,current,minutely,alerts&units=metric&appid="+apikey;
			return await fetch(url).then((response) => response.json());
		}

		setInterval(() => {
			data.forEach((source) => {
				let predData;
				getPredictDataFromAPI(source.lat,source.long).then((res) => {
					predData = res;
					let pred_entry = {
						city: source.city,
						pred: predData.daily
					}
					dbo.collection("api-pred").insertOne(pred_entry, function (err, res) {
						if (err) throw err;
					});
				})
				let apiData;
				getCurrentDataFromAPI(source.lat,source.long).then((res) => {
					apiData = res;
					let api_entry = {
						date: apiData.current.dt*1000 + apiData.timezone_offset*1000 -7200000,
						city: source.city,
						value: apiData.current.temp
					}
					dbo.collection("api").insertOne(api_entry, function (err, res) {
						if (err) throw err;
					});
				});
			})
		},3000000)

	//===============================================
	// Connexion au broker MQTT distant
	//
	const mqtt_url = 'http://test.mosquitto.org:1883';
	const client_mqtt = mqtt.connect(mqtt_url);

		//===============================================
	// Des la connexion, le serveur NodeJS s'abonne aux topics MQTT 
	//
	client_mqtt.on('connect', function () {

	    client_mqtt.subscribe(TOPIC_TEMP, function (err) {
		if (!err) {
		    console.log('Node Server has subscribed to ', TOPIC_TEMP);
		}
	    })
	})

	//================================================================
	// Callback de la reception des messages MQTT pour les topics sur
	// lesquels on s'est inscrit.
	// => C'est cette fonction qui alimente la BD !
	//
	client_mqtt.on('message', async function (topic, message) {
		console.log("\nMQTT msg on topic : ", topic.toString());
		console.log("Msg payload : ", message.toString());
		let frTime = new Date();
		// Parsing du message suppos� recu au format JSON
		message = JSON.parse(message.toString());

		// Vérification du Json, si erreur on l'envoie dans la collection logs
		if (!("info" in message && "ident" in message.info && "status" in message && "temperature" in message.status)) {
			let new_error = {};
			if ("info" in message && "ident" in message.info) {
				new_error =
					{
						date: frTime,
						msg: "Mauvais JSON: " + message.info.ident + " => Erreur dans le JSON",
					};
			} else {
				new_error =
					{
						date: frTime,
						msg: "Mauvais JSON: Erreur dans le JSON ( aucune identification reçue)",
					};
			}
			dbo.collection("logs").insertOne(new_error, function (err, res) {
				if (err) throw err;
			});
		} else {
			let temp = message.status.temperature;
			let localisation = message.info.loc;
			let wh = message.info.ident;
			let user = message.info.user;
			let ip = message.info.ip;

			// Debug : Gerer une liste de who pour savoir qui utilise le node server
			// let wholist = []
			// var index = wholist.findIndex(x => x.who==wh)
			// if (index === -1){
			// wholist.push({who:wh});
			// }
			// console.log("wholist using the node server :", wholist);

			// Mise en forme de la donnee � stocker => dictionnaire
			// Le format de la date est iomportant => compatible avec le
			// parsing qui sera realise par hightcharts dans l'UI
			// cf https://www.w3schools.com/jsref/tryit.asp?filename=tryjsref_tolocalestring_date_all
			// vs https://jsfiddle.net/BlackLabel/tgahn7yv
			if (!await dbo.collection("esp").countDocuments({identification: wh})) {
				let esp_entry = {
					identification: wh,      // identify ESP who provide
					ip: ip,
					user: user
				};
				let new_login =
					{
						date: frTime.toLocaleString(),
						msg: "Nouvelle connexion : "+ wh ,
					};
				dbo.collection("esp").insertOne(esp_entry, function (err, res) {
					if (err) throw err;
					console.log("\nItem : ", esp_entry,
						"\ninserted in db in collection : esp");
				});

				dbo.collection("logs").insertOne(new_login,function (err, res) {
					if (err) throw err;
					console.log("\nItem : ", new_login,
						"\ninserted in db in collection : logs");
				});

			}
			//var frTime = new Date().toLocaleString("sv-SE", {timeZone: "Europe/Paris"});
			let new_entry =
				{
					date: frTime,
					identification: wh,      // identify ESP who provide
					value: temp,
					localisation: localisation
				};

			dbo.collection("temp").insertOne(new_entry, function (err, res) {
				if (err) throw err;
				console.log("\nItem : ", new_entry,
					"\ninserted in db in collection : temp");
			});
		}
	})
		// On récupére les données de l'API pour remplir notre base


		//================================================================
// Answering GET request on this node ... probably from navigator.
// => REQUETES HTTP reconnues par le Node
//================================================================

// Route / => Le node renvoie la page HTML affichant les charts
		app.get('/', function (req, res) {
			res.sendFile(__dirname+'/public/ui_lucioles.html');
		});


// The request contains the name of the targeted ESP !
//     /esp/temp?who=80%3A7D%3A3A%3AFD%3AC9%3A44
// /esp/:what
// Exemple d'utilisation de routes dynamiques
//    => meme fonction pour /esp/temp et /esp/light
		app.get('/esp/temp', function (req, res) {
			// cf https://stackabuse.com/get-query-strings-and-parameters-in-express-js/
			console.log(req.originalUrl);

			let wh = req.query.who // get the "who" param from GET request

			console.log("\n--------------------------------");
			console.log("A client/navigator ", req.ip);
			console.log("sending URL ",  req.originalUrl);
			console.log("values from object ", wh);

			// R�cup�ration des nb derniers samples stock�s dans
			// la collection associ�e a ce topic (wa) et a cet ESP (wh)
			const nb = 200;
			//dbo.collection(key).find({who:wh}).toArray(function(err,result) {
			dbo.collection("temp").find({identification:wh}).sort({_id:-1}).limit(nb).toArray(function(err, result) {
				if (err) throw err;
				res.json(result.reverse()); // This is the response.
			});
		});


		//path qui retourne la liste des esp avec leur identification et leur user
		app.get('/esp/list', function (req, res) {

			dbo.collection("esp").find().toArray(function(err, result) {
				if (err) throw err;
				res.json(result);
			});

		});
		//path qui retourne la liste des logs
		app.get('/esp/logs', function (req, res) {

			//dbo.collection(key).find({who:wh}).toArray(function(err,result) {
			dbo.collection("logs").find().toArray(function(err, result) {
				if (err) throw err;
				res.json(result); // This is the response.
			});
		});
		app.get('/api/locations',function(req,res){
			let data = require("./public/assets/api-locations.json");
			res.json(data);
		});
		app.get('/api/temp',async function(req,res){
			let city = req.query.city;
			dbo.collection("api").find({city:city}).limit(200).toArray(function(err, result) {
				if (err) throw err;
				res.json(result.reverse()); // This is the response.
			});
		});
		app.get('/api/predict',async function(req,res){
			let city = req.query.city;
			dbo.collection("api-pred").find({city:city}).limit(1).toArray(function(err, result) {
				if (err) throw err;
				res.json(result); // This is the response.
			});
		});

	//================================================================
	// Fermeture de la connexion avec la DB lorsque le NodeJS se termine.
	//
	process.on('exit', (code) => {
	    if (mg_client && mg_client.isConnected()) {
		console.log('mongodb connection is going to be closed ! ');
		mg_client.close();
	    }
	})
	
    });// end of MongoClient.connect
}// end def main

//================================================================
//==== Demarrage BD et MQTT =======================
//================================================================
v0().catch(console.error);
