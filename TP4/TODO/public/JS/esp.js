
class Esp {

    constructor(user,ident) {
        this.user = user;
        this.ident = ident;
        this.longitude = '';
        this.latitude = '';
        this.temp = '';
        this.marker = undefined;
        this.data = [];
    }
    getUser(){
        return this.user;
    }
    setUser(user){
        this.user = user;
    }
    getIdent(){
        return this.ident;
    }
    getMarker(){
        return this.marker;
    }

    haslocalisation(){
        return !!(this.longitude && this.latitude);
    }

    setUpMarker(){
        this.marker = L.marker([this.latitude,this.longitude]).addTo(map);
        this.marker.bindPopup("<span style='color:red'>"+this.user+"</span><br>"+this.temp);
        this.marker.on("mouseover",() => this.marker.openPopup());
        this.marker.on("mouseout",() => this.marker.closePopup());
        this.marker.on("click",() => {
            infoContainer.setName(this.user);
            infoContainer.setTemp(this.temp)
            infoContainer.refreshData();
        })
    }

    extractLastData(){
        let lastData = this.data[this.data.length-1];
        this.temp = lastData.value;
        this.latitude = lastData.localisation.latitude;
        this.longitude = lastData.localisation.longitude;
    }

    refreshDataMarker(){
        this.marker.setLatLng(L.latLng(this.latitude,this.longitude));
    }
    setData(data){
        this.data = data;
    }
}