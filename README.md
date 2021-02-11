***SDEA SECURED UDP PUNCHTRU***


Start relying server on a server out there on some IP

```
marius@(none):~/APPS/perlink$ ./perlink s
SERVER  
```

On same server or on another PC anywhere start a provider in group abc

```
marius@(none):~/APPS/perlink$ ./perlink p p:abc 6603 SRV.IP.ADD.RESS

```
On another PC start one or more consumers

```
./perlink c c:abc 3003 IP.RELAY.ING.SRV

```


The 2 pers are linked P2P. The server does not rely the data.

**Concept**



![alt text](https://raw.githubusercontent.com/circinusX1/perlink/master/doc/concept.png "concept")


**Flow**

![alt text](https://raw.githubusercontent.com/circinusX1/perlink/master/doc/flow.png "flow")



