
$client = new-object net.sockets.udpclient(0) 

$peerIP = "127.0.0.1" 

$peerPort = "22904"  

#RELEASE THE DLL
$send = [text.encoding]::ascii.getbytes("RELEASE_DLL--C:\SierraChart\Data\carlozIndis.dll") 
[void] $client.send($send, $send.length, $peerIP, $peerPort) 
# $client.close() 

#COMPILE THE DLL FOR SIERRACHART
cl.exe /IC:\SierraChart\ACS_Source /GL /W1 /EHsc /nologo /LD /std:c++17 /FeC:\Users\czoa\OneDrive\Bolsa\SierraChart\carlozIndis.dll carlozIndis.cpp
# cl.exe /IC:\SierraChart\ACS_Source /permissive- /GS /GL /W3 /Gy /EHsc /nologo /LD /std:c++20 /FeC:\Users\czoa\OneDrive\Bolsa\SierraChart\carlozIndis.dll carlozIndis.cpp
# cl.exe /permissive- /GS /GL /W3 /Gy /EHsc /nologo /LD /std:c++20 /FeC:\SierraChart\Data\carlozIndis.dll C:\SierraChart\ACS_Source\carlozIndis.cpp

# RELOAD DLL
$send = [text.encoding]::ascii.getbytes("ALLOW_LOAD_DLL--C:\SierraChart\Data\carlozIndis.dll") 
[void] $client.send($send, $send.length, $peerIP, $peerPort) 
$client.close() 

