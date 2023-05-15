function scanAps()
{
    const xhr = new XMLHttpRequest();
    xhr.open("GET", "scanAps");
    xhr.send();
    xhr.responseType = "json";
    xhr.onload = () => {
        if (xhr.readyState == 4 && xhr.status == 200) {
          console.log(xhr.response);
        } else {
          console.log(`Error: ${xhr.status}`);
        }
    }
}