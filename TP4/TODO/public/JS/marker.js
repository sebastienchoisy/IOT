class Marker {
    constructor(name,long,lat,temp){
        this.long = long;
        this.lat = lat;
        this.name = name;
        this.temp = temp;
    }

    addToMap(){
        L.marker([this.lat,this.long])
            .bindPopup(
                this.name+"<br>"+this.temp
            ).addTo(map)
            .on("click",() => {
                infoContainer.setName(this.name);
                infoContainer.setTemp(this.temp)
                infoContainer.refreshData();
            })
    }
}