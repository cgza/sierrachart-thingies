
$client = new-object net.sockets.udpclient(0)  

$peerIP = "127.0.0.1" 

$peerPort = "22904"  

$send = [text.encoding]::ascii.getbytes("ALLOW_LOAD_DLL--C:\SierraChart\Data\carlozIndis.dll") 

[void] $client.send($send, $send.length, $peerIP, $peerPort)  

$client.close()  
