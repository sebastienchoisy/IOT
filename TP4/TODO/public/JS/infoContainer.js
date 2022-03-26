class InfoContainer {
    constructor() {
        this.dataSource = undefined;
        this.chart = new TempChart();
    }
    setDataSource(dataSource){
        this.dataSource = dataSource;
        if(this.chart.getChart()){
            this.chart.refreshData(this.dataSource.getName(),this.dataSource.getData());
        } else {
            this.chart.initNewTempChart(this.dataSource.getData(),this.dataSource.getName())
        }
        this.refreshData();

    }
    setInfo(){
        document.getElementById("name").innerHTML = this.dataSource.getName()+" ( "+this.dataSource.getType()+" )";
        document.getElementById("temp").innerHTML = this.dataSource.getLastTemp()+ "°C";
        document.getElementById("ident").innerHTML = this.dataSource.getType()=="api"? "aucune":this.dataSource.getIdent();
        document.getElementById("loc").innerHTML = this.dataSource.getCity()? this.dataSource.getCity()+", "+this.dataSource.getCountry()+
            " <img width='15px' height='15px' src='../assets/flags/"+this.dataSource.getCountry().toLowerCase()+".svg'>" : "aucune";
        document.getElementById("h-temp").innerHTML = this.getRecordTemp()[1]+ "°C";
        document.getElementById("l-temp").innerHTML = this.getRecordTemp()[0]+ "°C";
    }
    setTemp(temp){
        this.temp = temp;
        document.getElementById("temp").innerHTML = "Temp: "+this.temp;
    }

    getChart(){
        return this.chart;
    }

    getRecordTemp(){
        let records = [];
        let tempArray = this.dataSource.getData().map((data) => parseFloat(data.value));
        records[0] = Math.min(...tempArray);
        records[1] = Math.max(...tempArray);
        return records;
    }

    refreshData(){
        if(dataSource) {
            setInterval(() => {
                this.setInfo();
                this.chart.refreshData(this.dataSource.getName(),this.dataSource.getData());
            },2000)
        }

    }
}