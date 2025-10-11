#include <Arduino.h>
#include "dataWebPage.h"
#include <WiFi.h>

// WiFi credentials
const char* ssid = "Bean-Team-ESP";
const char* password = "";

// Web server running on port 80
WiFiServer server(80);

String temperatureValue = "0.0 &#8451;";
String pressureValue = "0.0 psi";

String temperatureSetpoint = "81.7 &#8451;";
String pressureSetpoint = "71.2 psi";

void dataWebPage::init() {
  Serial.begin(115200);

  // Set up the Access Point with provided SSID and password
  WiFi.softAP(ssid, password);

  // Start the server
  server.begin();
}

void dataWebPage::update(float temp, float pressure, float tempSetPoint, float pressureSetPoint){
  
  temperatureValue = String(temp);
  pressureValue = String(pressure);
  temperatureSetpoint = String(tempSetPoint);
  pressureSetpoint = String(pressureSetPoint);

  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                           
    // Read the HTTP request
    String request = client.readStringUntil('\r');
    client.flush();

    // Check what type of request this is
    if (request.indexOf("GET /sensor/temp") >= 0) {
      // Send temperature data only
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/plain");
      client.println("Connection: close");
      client.println();
      client.println(temperatureValue);
    }
    else if (request.indexOf("GET /sensor/pressure") >= 0) {
      // Send pressure data only
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/plain");
      client.println("Connection: close");
      client.println();
      client.println(pressureValue);
    }
    else {
      // Send the main HTML page
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/html");
      client.println("Connection: close");
      client.println();

      // Send the HTML page with the initial content
      client.println("<html><head>");
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
      client.println("<tr><td>Temperature</td><td><span id=\"tempValue\">" + temperatureValue + "</span></td><td class=\"setpoint\">" + temperatureSetpoint + "</td></tr>");
      client.println("<tr><td>Pressure</td><td><span id=\"pressureValue\">" + pressureValue + "</span></td><td class=\"setpoint\">" + pressureSetpoint + "</td></tr>");
      client.println("</table>");

      // Temperature graph 
      client.println("<h2>Temperature Graph</h2>");
      client.println("<svg id='tempGraph' width='600' height='350' style='background:#f0f0f0; border:4px solid #987654;'></svg>");

      // Pressure graph 
      client.println("<h2>Pressure Graph</h2>");
      client.println("<svg id='pressureGraph' width='600' height='350' style='background:#f0f0f0; border:4px solid #987654;'></svg>");

      // JS
      client.println("<script>");
      client.println("const maxPoints = 20;");  // number of points shown in graph
      client.println("const tempData = [];");
      client.println("const pressureData = [];");

      client.println("function parseValue(str) {");
      client.println("  return parseFloat(str);");
      client.println("}");

      client.println("function drawGraph(svgId, data, yMin, yMax, strokeColor) {");
      client.println("  const svg = document.getElementById(svgId);");
      client.println("  const width = svg.clientWidth;");
      client.println("  const height = svg.clientHeight;");
      client.println("  svg.innerHTML = '';"); // clear previous graph");
      client.println("  if(data.length < 2) return;");

      client.println("  const points = data.map((val, idx) => {");
      client.println("    const x = (idx / (maxPoints - 1)) * width;");
      client.println("    const y = height - ((val - yMin) / (yMax - yMin)) * height;");
      client.println("    return x + ',' + y;");
      client.println("  }).join(' ');");

      client.println("  // Draw polyline");
      client.println("  const polyline = document.createElementNS('http://www.w3.org/2000/svg', 'polyline');");
      client.println("  polyline.setAttribute('fill', 'none');");
      client.println("  polyline.setAttribute('stroke', strokeColor);");
      client.println("  polyline.setAttribute('stroke-width', '3');");
      client.println("  polyline.setAttribute('points', points);");
      client.println("  svg.appendChild(polyline);");

      client.println("  // Draw axis line");
      client.println("  const axis = document.createElementNS('http://www.w3.org/2000/svg', 'line');");
      client.println("  axis.setAttribute('x1', 0);");
      client.println("  axis.setAttribute('y1', height);");
      client.println("  axis.setAttribute('x2', width);");
      client.println("  axis.setAttribute('y2', height);");
      client.println("  axis.setAttribute('stroke', '#333');");
      client.println("  axis.setAttribute('stroke-width', '1');");
      client.println("  svg.appendChild(axis);");

      client.println("  // Add yMin and yMax labels");
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

      // update temp
      client.println("function updateData() {");
      client.println("  fetch('/sensor/temp').then(r => r.text()).then(tempStr => {");
      client.println("    const tempVal = parseValue(tempStr);");
      client.println("    if (!isNaN(tempVal)) {");
      client.println("      if(tempData.length >= maxPoints) tempData.shift();");
      client.println("      tempData.push(tempVal);");
      client.println("      document.getElementById('tempValue').innerHTML = tempStr;");
      client.println("      drawGraph('tempGraph', tempData, 15, 35, '#ff5733');"); // Adjust Y axis as needed
      client.println("    }");
      client.println("  });");
      
      // update pressure
      client.println("  fetch('/sensor/pressure').then(r => r.text()).then(pressureStr => {");
      client.println("    const pressureVal = parseValue(pressureStr);");
      client.println("    if (!isNaN(pressureVal)) {");
      client.println("      if(pressureData.length >= maxPoints) pressureData.shift();");
      client.println("      pressureData.push(pressureVal);");
      client.println("      document.getElementById('pressureValue').innerHTML = pressureStr;");
      client.println("      drawGraph('pressureGraph', pressureData, 0, 100, '#3377ff');"); // Adjust Y axis as needed
      client.println("    }");
      client.println("  });");
      client.println("}");

      client.println("setInterval(updateData, 500);");
      client.println("updateData();");  // initial call

      client.println("</script>");

      client.println("</div>");
      client.println("</body></html>");

      // The HTTP response ends with another blank line
      client.println();
    }
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}



