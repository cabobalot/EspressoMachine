#include <Arduino.h>
#include "dataWebPage.h"
#include <WiFi.h>

// WiFi credentials
const char* ssid = "Bean-Team-ESP";
const char* password = "";

// Web server running on port 80
WiFiServer server(80);

String temperatureValue = "0.0";
String pressureValue = "0.0";
String temperatureSetpoint = "81.7";
String pressureSetpoint = "71.2";

void dataWebPage::init() {
  Serial.begin(115200);

  // Set up the Access Point with provided SSID and password
  WiFi.softAP(ssid, password);

  // Start the server
  server.begin();
}

void dataWebPage::update(float temp, float pressure, float tempSetPoint, float pressureSetPoint){
  
  temperatureValue = String(temp, 1);
  pressureValue = String(pressure, 1);
  temperatureSetpoint = String(tempSetPoint, 1);
  pressureSetpoint = String(pressureSetPoint, 1);

  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                           
    // Read the HTTP request
    String request = client.readStringUntil('\r');
    client.flush();

    // Check what type of request this is
    if (request.indexOf("GET /data") >= 0) {
      // Send all data as JSON
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:application/json");
      client.println("Connection: close");
      client.println();
      client.print("{\"temp\":");
      client.print(temperatureValue);
      client.print(",\"pressure\":");
      client.print(pressureValue);
      client.print(",\"tempSet\":");
      client.print(temperatureSetpoint);
      client.print(",\"pressureSet\":");
      client.print(pressureSetpoint);
      client.println("}");
    }
    else {
      // Send the main HTML page
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/html");
      client.println("Connection: close");
      client.println();

      // Send the HTML page with the initial content
      client.println("<!DOCTYPE html><html><head>");
      client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
      client.println("<style>");
      client.println("body { font-family: Monaco, monospace; background-color: #fdf5e6; color: #111; text-align: center; padding: 10px; }");
      client.println("table { margin: 0 auto; border-collapse: collapse; width: 90%; background-color: #fff; border-radius: 10px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); }");
      client.println("th, td { padding: 15px 20px; border: 1px solid #ddd; text-align: center; font-size: 1.2em; }");
      client.println("th { background-color: #987654; color: white; }");
      client.println("th:first-child { border-top-left-radius: 10px; }");
      client.println("th:last-child { border-top-right-radius: 10px; }");
      client.println("td { background-color: #f9f9f9; font-weight: bold; color: #333; }");
      client.println("h1 { color: #654321; font-size: 2em; margin-bottom: 20px; }");
      client.println(".container { max-width: 1000px; margin: 0 auto; padding: 20px; background-color: #fff; border-radius: 10px; }");
      client.println(".setpoint { color: #2196F3; }");
      client.println("</style>");
      client.println("</head><body>");
      client.println("<div class=\"container\">");
      client.println("<h1>Sensor Data</h1>");
      client.println("<table>");
      client.println("<tr><th>Sensor</th><th>Current Value</th><th>Setpoint</th></tr>");
      client.println("<tr><td>Temperature</td><td><span id=\"tempValue\">0.0</span> &#8451;</td><td class=\"setpoint\"><span id=\"tempSetpoint\">0.0</span> &#8451;</td></tr>");
      client.println("<tr><td>Pressure</td><td><span id=\"pressureValue\">0.0</span> psi</td><td class=\"setpoint\"><span id=\"pressureSetpoint\">0.0</span> psi</td></tr>");
      client.println("</table>");

      // Temperature graph 
      client.println("<h2>Temperature Graph</h2>");
      client.println("<div style='text-align:center; margin-bottom:10px;'>");
      client.println("<span style='margin-right:20px;'><svg width='30' height='3' style='vertical-align:middle;'><line x1='0' y1='1.5' x2='30' y2='1.5' stroke='#ff5733' stroke-width='3'/></svg> <span style='vertical-align:middle;'>Current</span></span>");
      client.println("<span><svg width='30' height='3' style='vertical-align:middle;'><line x1='0' y1='1.5' x2='30' y2='1.5' stroke='#ff5733' stroke-width='2' stroke-dasharray='5,5'/></svg> <span style='vertical-align:middle;'>Setpoint</span></span>");
      client.println("</div>");
      client.println("<svg id='tempGraph' width='600' height='350' style='background:#f0f0f0; border:4px solid #987654;'></svg>");

      // Pressure graph 
      client.println("<h2>Pressure Graph</h2>");
      client.println("<div style='text-align:center; margin-bottom:10px;'>");
      client.println("<span style='margin-right:20px;'><svg width='30' height='3' style='vertical-align:middle;'><line x1='0' y1='1.5' x2='30' y2='1.5' stroke='#3377ff' stroke-width='3'/></svg> <span style='vertical-align:middle;'>Current</span></span>");
      client.println("<span><svg width='30' height='3' style='vertical-align:middle;'><line x1='0' y1='1.5' x2='30' y2='1.5' stroke='#3377ff' stroke-width='2' stroke-dasharray='5,5'/></svg> <span style='vertical-align:middle;'>Setpoint</span></span>");
      client.println("</div>");
      client.println("<svg id='pressureGraph' width='600' height='350' style='background:#f0f0f0; border:4px solid #987654;'></svg>");

      // JS
      client.println("<script>");
      client.println("const maxPoints = 20;");
      client.println("const tempData = [];");
      client.println("const pressureData = [];");
      client.println("const tempSetpointData = [];");
      client.println("const pressureSetpointData = [];");

      client.println("function drawGraph(svgId, data, setpointData, yMin, yMax, strokeColor, setpointColor) {");
      client.println("  const svg = document.getElementById(svgId);");
      client.println("  const width = svg.clientWidth;");
      client.println("  const height = svg.clientHeight;");
      client.println("  svg.innerHTML = '';");
      client.println("  if(data.length < 2) return;");

      client.println("  const points = data.map((val, idx) => {");
      client.println("    const x = (idx / (maxPoints - 1)) * width;");
      client.println("    const y = height - ((val - yMin) / (yMax - yMin)) * height;");
      client.println("    return x + ',' + y;");
      client.println("  }).join(' ');");

      client.println("  const polyline = document.createElementNS('http://www.w3.org/2000/svg', 'polyline');");
      client.println("  polyline.setAttribute('fill', 'none');");
      client.println("  polyline.setAttribute('stroke', strokeColor);");
      client.println("  polyline.setAttribute('stroke-width', '3');");
      client.println("  polyline.setAttribute('points', points);");
      client.println("  svg.appendChild(polyline);");

      client.println("  if (setpointData.length >= 2) {");
      client.println("    const setpointPoints = setpointData.map((val, idx) => {");
      client.println("      const x = (idx / (maxPoints - 1)) * width;");
      client.println("      const y = height - ((val - yMin) / (yMax - yMin)) * height;");
      client.println("      return x + ',' + y;");
      client.println("    }).join(' ');");
      client.println("    const setpointLine = document.createElementNS('http://www.w3.org/2000/svg', 'polyline');");
      client.println("    setpointLine.setAttribute('fill', 'none');");
      client.println("    setpointLine.setAttribute('stroke', setpointColor);");
      client.println("    setpointLine.setAttribute('stroke-width', '2');");
      client.println("    setpointLine.setAttribute('stroke-dasharray', '5,5');");
      client.println("    setpointLine.setAttribute('points', setpointPoints);");
      client.println("    svg.appendChild(setpointLine);");
      client.println("  }");

      client.println("  const axis = document.createElementNS('http://www.w3.org/2000/svg', 'line');");
      client.println("  axis.setAttribute('x1', 0);");
      client.println("  axis.setAttribute('y1', height);");
      client.println("  axis.setAttribute('x2', width);");
      client.println("  axis.setAttribute('y2', height);");
      client.println("  axis.setAttribute('stroke', '#333');");
      client.println("  axis.setAttribute('stroke-width', '1');");
      client.println("  svg.appendChild(axis);");

      client.println("  const yMinText = document.createElementNS('http://www.w3.org/2000/svg', 'text');");
      client.println("  yMinText.setAttribute('x', 5);");
      client.println("  yMinText.setAttribute('y', height - 5);");
      client.println("  yMinText.setAttribute('font-size', '12');");
      client.println("  yMinText.setAttribute('fill', '#333');");
      client.println("  yMinText.textContent = yMin.toFixed(1);");
      client.println("  svg.appendChild(yMinText);");
    
      client.println("  const yMaxText = document.createElementNS('http://www.w3.org/2000/svg', 'text');");
      client.println("  yMaxText.setAttribute('x', 5);");
      client.println("  yMaxText.setAttribute('y', 15);");
      client.println("  yMaxText.setAttribute('font-size', '12');");
      client.println("  yMaxText.setAttribute('fill', '#333');");
      client.println("  yMaxText.textContent = yMax.toFixed(1);");
      client.println("  svg.appendChild(yMaxText);");
      client.println("}");

      client.println("function updateData() {");
      client.println("  fetch('/data').then(r => r.json()).then(data => {");
      client.println("    if (!isNaN(data.temp)) {");
      client.println("      if(tempData.length >= maxPoints) tempData.shift();");
      client.println("      tempData.push(data.temp);");
      client.println("      document.getElementById('tempValue').textContent = data.temp.toFixed(1);");
      client.println("    }");
      client.println("    if (!isNaN(data.pressure)) {");
      client.println("      if(pressureData.length >= maxPoints) pressureData.shift();");
      client.println("      pressureData.push(data.pressure);");
      client.println("      document.getElementById('pressureValue').textContent = data.pressure.toFixed(1);");
      client.println("    }");
      client.println("    if (!isNaN(data.tempSet)) {");
      client.println("      if(tempSetpointData.length >= maxPoints) tempSetpointData.shift();");
      client.println("      tempSetpointData.push(data.tempSet);");
      client.println("      document.getElementById('tempSetpoint').textContent = data.tempSet.toFixed(1);");
      client.println("    }");
      client.println("    if (!isNaN(data.pressureSet)) {");
      client.println("      if(pressureSetpointData.length >= maxPoints) pressureSetpointData.shift();");
      client.println("      pressureSetpointData.push(data.pressureSet);");
      client.println("      document.getElementById('pressureSetpoint').textContent = data.pressureSet.toFixed(1);");
      client.println("    }");
      client.println("    drawGraph('tempGraph', tempData, tempSetpointData, 0, 160, '#ff5733', '#ff5733');");
      client.println("    drawGraph('pressureGraph', pressureData, pressureSetpointData, 0, 140, '#3377ff', '#3377ff');");
      client.println("  }).catch(err => console.error('Fetch error:', err));");
      client.println("}");

      client.println("setInterval(updateData, 500);");
      client.println("updateData();");

      client.println("</script>");
      client.println("</div>");
      client.println("</body></html>");

      client.println();
    }
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}