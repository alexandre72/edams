# Example on how to use Arduino with EDAMS #

In my house, I'm using Arduino with EDAMS. But you're free to use whatever you want, the only rule is to use xPL. Alls devices and software communications are made by using this protocol.


# Details #

Here's a simplified schematic to see my installation:

![http://edams.googlecode.com/files/myhouse_scheme.png](http://edams.googlecode.com/files/myhouse_scheme.png)


Arduino ethernetshield allow me to put Arduino at distance of my Linux PC. On Arduino I'm using xPL.Arduino library(See more on:http://code.google.com/p/xpl-arduino/), to declare device(DHT11, PIR sensor's...), with a syntax like this:

```
xPL_Message msg;
 
msg.hop = 1;
msg.type = XPL_TRIG;
msg.SetTarget_P(PSTR("*"));
msg.SetSchema_P(PSTR("sensor"), PSTR("basic"));
msg.AddCommand_P(PSTR("device"),PSTR("DHT11"));
msg.AddCommand_P(PSTR("type"),PSTR( "humidty"));
msg.AddCommand("current", dht11.value);
 xpl.SendMessage(&msg);
...
```

Too see more about sensor.basic usage with EDAMS, see[how to declare device](HowToDeclareDevice.md).

After launching xPL\_HUB(in daemon mode, but to see if all is ok you can try in nodaemon mode):

```
alex ~/$xPL_Hub -nodaemon -xpldebug
12/12/28 14:10:34 INFO: Running on console
12/12/28 14:10:34 xPL_DEBUG: Checking if interface lo is valid w/flags 73
12/12/28 14:10:34 xPL_DEBUG: Checking if interface wlan0 is valid w/flags 4163
12/12/28 14:10:34 xPL_DEBUG: Choose interface wlan0 as default interface
12/12/28 14:10:34 xPL_DEBUG: Auto-assigning IP address of 192.168.0.1
12/12/28 14:10:34 xPL_DEBUG: Assigned xPL Broadcast address of 192.168.0.255, port 3865
12/12/28 14:10:34 xPL_DEBUG: Attemping standalone xPL
```

After launching EDAMS, I add a new location and fill informations about it(name, description, a recent photo, geolocalization...).

Now I select it by just clicking on it, click on "Add" button on the right panel. Devices picker opens,  I should see DHT11 device and others with theirs label. After clicking on "Select" I should see it in list in location panel. With 'Edit' button  I can change widget(like counter, thermometer...). With 'Actions' button, I can open actions editor that can be used to performs some predefined actions at some moments(like when data reach some values).

To have a nice background on my global devices map, I've designed my house with sweethome3d software(great opensource 3D house designing software!). You can get it on:

http://www.sweethome3d.com/fr/features.jsp

Now in preferences window I'll be able to set map background image to sweethome3d photo generated file(something like:/home/alex/house.png). Next step is to go to "Global view", in a resizable(with left/down/up/right arrows) and drag&drop(with mouse's button 1) rectangle I'll be able to move and resize it. I can drag&drop widgets anywhere I want too, but they can be moved only into location's group.

When new data comes from xPL sensor.basic device's label "UPDATED" appears to inform me about data updates, widgets show me data.