# Introduction #

EDAMs can handle two device's type that follow xPL protocol specifications:

  1. sensor.basic:theses devices that get some data about physical phenomems like gas, input, light intensity, voltage...
  1. control.basic:theses devices that can be set to act in an specific way like relays, light bulb...


# sensor.basic #

xPL manufactured device's are rare, in most case you'll need to create your own xPL device's. For Arduino case you'll use, xpl.arduino(http://code.google.com/p/xpl-arduino

To get xPL working follow xpl-arduino examples code and to declare a new sensor you'll use something like:

```
msg.SetSchema_P(PSTR("sensor"), PSTR("basic")); 
msg.AddCommand_P(PSTR("device"),PSTR("temperature"));
msg.AddCommand_P(PSTR("type"),PSTR("temp"));
msg.AddCommand_P(PSTR("current"),PSTR("22"));
```

sensor.basic follow xPL standard, when EDAMS discover a new xPL containing sensor.basic message it can recognize it as a sensor device class.

'device' follow xPL rule from sensor.basic message, it's an unique name for your device into your xPL network.

'type' follow xPL rule from sensor.basic message see http://xplproject.org.uk/wiki/index.php?title=Schema_-_SENSOR.BASIC to set other type(battery, weight...). EDAMS set units associated to these type(volt, celsius...). Cosm.com follow its standard too.

current=22 is data get from your sensor. EDAMS use xPL specs to interpret data value.