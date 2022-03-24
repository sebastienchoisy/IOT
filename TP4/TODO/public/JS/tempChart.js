class TempChart {
    constructor(data) {
        this.data = data;
        this.chart = undefined;
        this.initTempChart();
    }

    initTempChart(){
        let datalol = [];
        let dataTest = [{"timestamp":"2022-3-21 18:07:53","temperature":"16"},
            {"timestamp":"2022-3-21 18:08:10","temperature":"20"},
            {"timestamp":"2022-3-21 18:08:20","temperature":"24"},
            {"timestamp":"2022-3-21 18:08:40","temperature":"26"}];
        dataTest.forEach((data) => {
            let time = new Date(data.timestamp).getTime();
            datalol.push([time,parseFloat(data.temperature)]);
        })
        console.log(datalol);
        Highcharts.setOptions({
            global: { // https://stackoverflow.com/questions/13077518/highstock-chart-offsets-dates-for-no-reason
                    useUTC: false,
                    type: 'spline'
            },
            time: {timezone: 'Europe/Paris'}
            });
        this.chart = new Highcharts.chart({
            title: {text: 'Temperatures'},
        subtitle: { text: 'Irregular time data in Highcharts JS'},
            legend: {enabled: true},
            credits: false,
            chart: {renderTo: 'chart'},
            xAxis: {title: {text: 'Heure'}, type: 'datetime'},
            yAxis: {title: {text: 'Temperature (Deg C)'}},
            series: [{name: 'ESP1', data: datalol},
        	 {name: 'ESP2', data: []},
        	 {name: 'ESP3', data: []}],
        //colors: ['#6CF', '#39F', '#06C', '#036', '#000'],
        colors: ['red', 'green', 'blue'],
            plotOptions: {line: {dataLabels: {enabled: true},
        		     //color: "red",
        		     enableMouseTracking: true
        		    }
        	     }
        });
    }
}