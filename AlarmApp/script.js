function connectToESP32() {
    // Attempt to fetch from the ESP32 web server at 192.168.4.1.
    fetch('http://192.168.4.1', { mode: 'no-cors' })
      .then(() => {
        document.getElementById('status').textContent = 'Connected';
        // Redirect to the ESP32 web app.
        window.location.href = 'http://192.168.4.1';
      })
      .catch(() => {
        document.getElementById('status').textContent = 'Not Connected';
      });
  }
  