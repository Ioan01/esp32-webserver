var networks = [];

var ssid = "";

var password = "";

var ip = "";

function scanAps() {
    document.getElementById("passwordField").style.display = "none";
    document.getElementById("connectButton").style.display = "none";
    document.getElementById("loading").style.display = "none";
    document.getElementById("passwordField").value = "";


    document.getElementById("ipField").style.display = "none";
    document.getElementById("ipField").value = "";


    const xhr = new XMLHttpRequest();
    document.getElementById("loading").style.display = "block";

    xhr.open("GET", "scanAps");
    xhr.send();
    xhr.responseType = "json";
    xhr.onload = () => {
        document.getElementById("loading").style.display = "none";

        if (xhr.readyState == 4 && xhr.status == 200) {
            console.log(xhr.response);
            networks = xhr.response.networks;

            let nets = document.getElementById("networks");
            while (nets.firstChild) {
                nets.firstChild.remove();
            }

            if (networks != null) {
                if (networks.length > 0) {
                    if (networks[0].open === false) {
                        document.getElementById("passwordField").value = "";
                        document.getElementById("passwordField").style.display = "inline";

                        document.getElementById("ipField").value = "";
                        document.getElementById("ipField").style.display = "inline";
                    }

                    ssid = networks[0].ssid;
                    document.getElementById("connectButton").style.display = "inline";
                }

                networks.forEach((network) => {
                    elem = document.createElement("option");
                    elem.value = network.ssid;
                    elem.text = network.ssid;
                    nets.appendChild(elem);
                });
            }
        } else {
            console.log(`Error: ${xhr.status}`);
        }
    };
}

function connect() {
    const xhr = new XMLHttpRequest();
    document.getElementById("connectButton").style.display = "none";

    document.getElementById("loading").style.display = "block";

    xhr.open("POST", `connect`);

    if (document.getElementById("connection") != null)
        document.getElementById("connection").remove();

    xhr.send(`${ssid}\\${password}\\${ip}\\`);
    xhr.responseType = "json";
    xhr.onload = () => {
        document.getElementById("loading").style.display = "none";

        var elem = document.createElement("span");
        elem.id = "connection";

        if (xhr.readyState == 4 && xhr.status == 200) {
            elem.textContent = "Successfully connected";
        } else {
            elem.textContent =
                "Failed to connect. Please check WiFi availability and the password.";
            elem.style.color = "#ff0000";
        }

        document.getElementById("body").appendChild(elem);
        document.getElementById("connectButton").style.display = "inline";

    };
}

function selectionChange(select) {
    ssid = networks[select.selectedIndex].ssid;
    if (networks[select.selectedIndex].open === false) {
        document.getElementById("passwordField").value = "";
        document.getElementById("passwordField").style.display = "inline";

        document.getElementById("ipField").value = "";
        document.getElementById("ipField").style.display = "inline";
    }
    else password = '';
}

function passwordChange() {
    password = document.getElementById("passwordField").value;
}

function ipChange() {
    ip = document.getElementById("ipField").value;
}
