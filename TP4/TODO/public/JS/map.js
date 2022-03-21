let map = L.map('map', {
    center: [20.0, 5.0],
    minZoom: 2,
    zoom: 2,
})
L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    attribution:
        '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a>',
    subdomains: ['a', 'b', 'c'],
}).addTo(map)

new Marker("esp",2.3522,48.8566,25).addToMap();
new Marker("MianMian se démerde",126,45,32).addToMap();
