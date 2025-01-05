// toggle the left navbar
function toggleNavbar() {
    const navbar = document.getElementById('sideNavbar');
    navbar.classList.toggle('active');
}

document.addEventListener("DOMContentLoaded", () => {
    if (document.getElementById("map")) {

    const map = L.map('map', {
        center: [51.505, -0.09],
        zoom: 13,
        zoomControl: false
    });

    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        maxZoom: 19,
        attribution: '© OpenStreetMap'
    }).addTo(map);
    
    L.control.zoom({
        position: 'bottomright'
    }).addTo(map);

    const marker = L.marker([51.505, -0.09]).addTo(map)
        .bindPopup("Latitude: 51.505, Longitude: -0.09")
        .openPopup();

    fetchLocation();

    function updateMap(lat, lon) {
        const newLatLng = new L.LatLng(lat, lon);
        marker.setLatLng(newLatLng);
        marker.getPopup().setContent(`Latitude: ${lat}, Longitude: ${lon}`);
        map.setView(newLatLng, 15);
    }

async function fetchLocation() {
    try {
        const response = await fetch("/gps-info");
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        const gpsInfo = await response.json();
        console.log("Received GPS data:", gpsInfo);

        if (gpsInfo.latitude && gpsInfo.longitude) {
            updateMap(gpsInfo.latitude, gpsInfo.longitude);
        } else {
            console.error("Invalid GPS data received:", gpsInfo);
        }
    } catch (error) {
        console.error("Failed to fetch GPS info from ESP32:", error);
    }
}

    setInterval(fetchLocation, 5000); // Fetch every 5 seconds
    }
});

async function populateBoardInfo() {
    try {
        const response = await fetch("/board-info");
        if (!response.ok) {
            throw new Error("Network response was not ok");
        }

        const boardInfo = await response.json();

        document.getElementById("manufacturer").textContent = boardInfo.manufacturer;
        document.getElementById("model").textContent = boardInfo.model;
        document.getElementById("serial").textContent = `Serial: ${boardInfo.serial}`;
        document.getElementById("hardware-version").textContent = `HW Ver: ${boardInfo.hardwareVersion}`;
        document.getElementById("firmware-version").textContent = `FW Ver: ${boardInfo.firmwareVersion}`;
        document.getElementById("software-version").textContent = `SW Ver: ${boardInfo.softwareVersion}`;

        populateConfigTable(boardInfo.config);
    } catch (error) {
        console.error("Failed to fetch board info:", error);
    }
}

function populateConfigTable(config) {
    const tableBody = document.getElementById("config-table-body");
    tableBody.innerHTML = "";

    // Add each config item as a table row
    for (const [key, value] of Object.entries(config)) {
        const row = document.createElement("tr");

        const keyCell = document.createElement("td");
        keyCell.textContent = key;

        const valueCell = document.createElement("td");
        valueCell.textContent = value;

        row.appendChild(keyCell);
        row.appendChild(valueCell);
        tableBody.appendChild(row);
    }
}
populateBoardInfo();