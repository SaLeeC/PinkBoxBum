
String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";


String html_1 = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta name='viewport' content='width=device-width, initial-scale=1.0' />
    <meta charset='utf-8'>
    <style>
        body    {font-size: 120%; }
        .main   {display: table;
                width: 300px;
                margin: auto;
                padding: 10px 10px 10px 10px;
                border: 3px solid blue;
                border-radius: 10px;
                text-align: center; }
        p       {font-size: 75%; }
    </style>
    <title>Pink Box Bum</title>
</head>
<body>
    <div id='main1' class='main'>
        <h3>STATO</h3>
        <div id='content1'>
            <p id='temp_6'>------</p>
            <p id='temp_7'>------</p>
            <p id='temp_8'>------</p>
        </div>
    </div>
    <br />
    <div id='main2' class='main'>
        <h2>EVENTI</h2>
        <div id='content2'>
            <p>
                <span id='temp_5'>------</span> &nbsp;cm;
            </p>
            <p>
                <span id='temp_0'>------</span> &nbsp;di&nbsp;
                <span id='temp_1'>------</span>
            </p>
        </div>
    </div>
    <br />
    <div id='main3' class='main'>
        <h3>BLOCCHI</h3>
        <div id='content3'>
           <p>
                <span id='temp_2'>-----</span> &nbsp;Avviamenti
            </p>
            <p>
                <span id='temp_3'>-----</span> &nbsp;Blocco Motore
            </p>
            <p>
                <span id='temp_4'>-----</span> &nbsp;Blocco Overflow Imp.
            </p>
        </div>
        <br />
        <p>Recieved data = <span id='rd'>---</span> </p>
    </div>
</body>

<script>
  var Socket;
  function init()
  {
    Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
    Socket.onmessage = function(event) { processReceivedCommand(event); };
  }

function processReceivedCommand(evt)
{
    document.getElementById('rd').innerHTML = evt.data;
          var tmpArray = evt.data.split("|");
          document.getElementById('temp_0').innerHTML = tmpArray[0];
          document.getElementById('temp_1').innerHTML = tmpArray[1];
          document.getElementById('temp_2').innerHTML = tmpArray[2];
          document.getElementById('temp_3').innerHTML = tmpArray[3];
          document.getElementById('temp_4').innerHTML = tmpArray[4];
          document.getElementById('temp_5').innerHTML = tmpArray[5];
          document.getElementById('temp_6').innerHTML = tmpArray[6];
          document.getElementById('temp_7').innerHTML = tmpArray[7];
          document.getElementById('temp_8').innerHTML = tmpArray[8];

}

  window.onload = function(e)
  {
    init();
  }
</script>
</html>
)=====";
