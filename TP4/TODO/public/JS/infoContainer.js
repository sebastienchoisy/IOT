class InfoContainer {
    constructor() {
        this.name = '';
        this.temp = '';
        this.light = '';
    }

    setName(name){
        this.name = name;
    }

    setTemp(temp){
        this.temp = temp;
    }

    refreshData(){
        document.getElementById("name").innerHTML = "Name: "+this.name;
        document.getElementById("temp").innerHTML = "Temp: "+this.temp;
    }
}