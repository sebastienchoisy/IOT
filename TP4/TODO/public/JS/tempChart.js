class TempChart {
    constructor() {
        this.chart = undefined;
    }
    refreshData(user,data){
        this.chart.series[0].update({name: user, data: this.formatData(data)});
        this.chart.title.update({test:"test"});
        this.chart.setTitle({ text: 'Températures: '+user});
    }
    formatData(data) {
        let dataArray = [];
        data.forEach((el) => {
            dataArray.push([new Date(el.date).getTime(),parseFloat(el.value)]);
        })
        return dataArray;
    }
    getChart(){
        return this.chart;
    }

    initNewTempChart(data,name){
        let formattedData = this.formatData(data);
        Highcharts.setOptions({
            global: {
                    useUTC: false,
                    type: 'spline'
            },
            time: {timezone: 'Europe/Paris'}
            });
        this.chart = new Highcharts.chart('chart',{
            legend: {enabled: true},
            credits: false,
            xAxis: {title: {text: 'Heure'}, type: 'datetime'},
            yAxis: {title: {text: 'Temperature (Deg C)'}},
            series: [{name: name, data: formattedData}],
        colors: ['blue'],
            plotOptions: {line: {dataLabels: {enabled: true},
        		     color: "red",
        		     enableMouseTracking: true
        		    }
        	     }
        });
        this.chart.setTitle({text: 'Températures: '+name});
    }
}