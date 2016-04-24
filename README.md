KRÁTKY POPIS
===============

Programy client a server slúžiace na vzájomné preposielanie súborrov za 
využitia socketov a protokolu popísaného v "protocol.pdf".

SERVER
===============

Server sa spúšťa s povinným prametrom -p a portom v rozsahu 1024-65535, 
ktoré niesu well known porty. Server beží neustále a naslúcha na zadanom 
porte na správy od klientov, ktorých počet môže byť vyšší ako 1 vďaka tomu,
že server oblsuhuje každého klienta na zvlášť vlákne.

Počas komunikácie by nemalo dojsť k neúspešnemu ukončeniu serveru

Pre ukladanie dát server využíva dočastný súbor. Po úspešnom prijatí
dát sa tento súbor premenuje na správny názov.

Príklad spustenia:
./server -p 9999
- server načúva na porte 9999

KLIENT
===============

Klient sa spúšťa s povinnými prametrami -p teda port v rozsahu 1024-65535 
a -h teda hostname servra kde aplikácia server beží na danom porte.
Ďalej client pozná parametre -u a -d ktoré nejde kombinovať, -u je upload
na server a -d download zo servra.

Počas komunikácie môže dojsť k neúspešnemu ukončeniu klienta s 
podrobnejšou hláškou na stderr.

Pre ukladanie dát client využíva taktiež dočastný súbor. Po úspešnom prijatí
dát sa tento súbor premenuje na správny názov.

Príklad spustenia:

./client -p 9999 -h host -d file
- klient žiada súbor "file" of hosta "host" na porte 9999

./client -p 9999 -h host -u file
- klient žiada o uchovanie súboru "file" na hosta "host" na porte 9999