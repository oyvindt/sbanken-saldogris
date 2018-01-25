# sbanken-saldogris

Konverter din tomme sparegris til å vise hvor mye penger du har hos Sbanken!

![Saldogris](images/saldogris-video.gif?raw=true "Saldogris")

Bruker [Sbankens API-er](https://sbanken.no/bruke/utviklerportalen/) for å hente data.
Fyll inn SSN og API-credentials.

## Hardware

* Arduino / Arduino-kompatibel mikrokontroller med WiFi-støtte f.eks. [NodeMCU](https://www.kjell.com/se/sortiment/el-verktyg/arduino/utvecklingskort/nodemcu-utvecklingskort-p87949)
* Skjerm - f.eks. denne: [128 x 64 OLED](https://www.kjell.com/no/produkter/elektro-og-verktoy/elektronikk/optokomponenter/led-lcd-skjermer/luxorparts-grafisk-oled-skjerm-128-x-64-piksler-0-96--p87945)
* Gris eller annen egnet innkapsling (den på bildet er fra BR Leker: [Sparegris](https://www.br.no/vaare-kategorier/til-barnerommet/bankboks/sparegris-lilla?id=000000000102417002) )

## Todo
* Fiks henting av kontoer - når hentes de to første kontoene (og du må ha minst to kontoer, det programmet antar at nr 2 er et kredittkort..)
* Lag skikkelig datastruktur for lagring av alle kontoer