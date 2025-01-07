---
usage: SI, ScalarValues
name: DTTester4Project
description: DTTester4Project
title: DTTester4Project
maintainer: Moritz Herzog
---

[toc]

# Basic Definitions
In the basic definitions the commands variable, measurable and controllable are defined. These Commands allow us to specify specific digital twin (dt) properties.

## Variable
**Will be replaced by attribute alone!** A variable displays a variable to the dt, whereas a attribute defined a static value.

```SysML
#command variable attribute;
```

## Measurable
Displays the dt a value that is measured. A measured value is a value that measured by the physical twin (pt) and streamed to the dt. Allowing the dt to implement functionality and much more.

```SysML
#command measurable variable;
```

## Controllable
Displays the dt a variable that is calculated by the dt and streamed to the pt. The pt has to act accordingly to the value of the variable. This allows for an controll elements of the pt.

```SysML
#command controllable variable; 
```

# Introduction
This represents the example of an bidirectional charger is displayed. This is a really basic implementation of the charger, also parts are missing.

# Components
In this section shows all components of the example for the digital twin.
## Cableport
The cable port definition is the definition of a port allowing for the connections between the components. The name stems from the cable, beeing the thing that connects the componets in reality, and the port, beeing the class of SysMLv2 creating the connection option. This allows the dt to implement the communication between the elements.

```SysML
port def CablePort {
    variable Voltage : Voltage;
    variable Current : Current;
}
```

## Grid
The grid part displays the real grid that is part of the systems definition. In the grid we could connect to an alternating current (two phases) or an tree phase current.

```SysML
part Grid {
    measurable ACVoltage : Voltage;
    port WallOutlet : CablePort;
}
```

## Rectifyer

This changes the alternating current to the direct current. This part can eigthe be in the car or in the charging station, dependend on the used standard [[Source](https://en.wikipedia.org/wiki/Charging_station)]. Normally also a n Inverter is needed inside of the model, to allow for the Implementation of the charging of the grid.

```SysML
part ACDC {
    port WallPlug : CablePort;
    measurable ACVoltage : Voltage;
    measurable DCVoltage : Voltage;
    port DCConnection : CablePort;
}
```

## DC-to-DC converter

The dc-to-dc converter allows for the convertion between the voltages.  With having an high voltage side and a low voltage side. The converter needs to convert in both directions, allowing for the bidirectional charging.

```SysML
part DCDC {
    port HVVoltagePort : CablePort;
    measurable HVVoltage : Voltage;
    measurable LVVoltage : Voltage;
    port LVVoltagePort : CablePort;
}
```

## Battery

The battery that is charged and discharged. Normally the battery as a specific side on the dc-dc converter.

```SysML
part Battery {
    port plusVoltage : CablePort;
    port referenceVoltage : CablePort;
    measurable batteryVoltage : Voltage;
    attribute internalResistance : Resistance;
    controllable offSwitch : Boolean;
}
```

## Load

The load that is used to display the bidirectinal system side.

```SysML
part Load {
    measurable dCVoltage : Volage;
    controllable offSwitch : Boolean;
}
```

# Complete System

```SysML
part EVCharger {
    part grid : Grid;
    part acdc : ACDC;
    part dcdc : DCDC;
    part battery : Battery;
    part load : Load;
}

EVCharger::grid::ACVoltage connectTo EVCharger::acdc::ACVoltage;
EVCharger::acdc::DCVoltage connectTo EVCharger::dcdc::DCVoltageInput;
EVCharger::dcdc::DCVoltageOutput connectTo EVCharger::battery::DCVoltageOutput;
EVCharger::dcdc::DCVoltageOutput connectTo EVCharger::load::DCVoltage;
```
