---
usage: SI, ScalarValues
name: DTTester4Airbag
description: DTTester4Airbag
title: DTTester4Airbag
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
This represents the example of an airbag is displayed. [Airbag Information Hella](https://www.hella.com/techworld/de/technik/elektrik-und-elektronik/airbag-system/)

