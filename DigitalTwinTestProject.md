 # Digital Twin Test Project
```SysMD::Global
Document uses ScalarValues, SI, ISO26262. 
```
Within this Project the Digital Twin Models are defined.
# Definition of the Base Components
```SysMD::Global
Condensator isA Component
Condensator imports SI.
Condensator hasA
    capacity: Capacity(10^(-12)..10000) [F]
    inputVoltage: Voltage() [V] measurable
    charge: Capacity*Volage [C] measureable
    outputVoltage : inputVoltage at t_0 + 1/C * \int^{t_0}_{t} I(a) da [v] mesurable
    outputCurrent : C * (V(t) * d/dt) [i]
   
```
```SysMD::Global
OperationalAmplifier isA Component
OperationalAmplifier imports SI.
OperationalAmplifier hasA
    lowInputVoltage:Voltage()[V] measurable
    highInputvoltage:Voltage()[V] measurable
    Amplification:Real()
    OutputVoltage:Amplification*(highInputVoltage-lowInputVoltage) [V] measurable
```
```SysMD::Global
Resistor isA Component
Resistor imports SI.
Resistor hasA
    inputVoltage: Voltage() [V] measurable
    inputCurrent: Current() [I] measuralbe
    resistance:Resistace() [Omega]
    outputVoltage:Voltage() [V] measurable
```
# Advanced Components
```SysMD::Global
VoltageDivider isA Component.
VoltageDivider imports SI.
VoltageDivider hasA
    highInputVoltage:Voltage [V] measurable
    lowInputVoltage:Voltage [V] mesurable
    outputVoltage:Voltage [V] mesurable
    UpperResistor:Resistor(inputVoltage = highInputVoltage,outputVoltage = outputVoltage)
    LowerResistor:Resistor(inputVoltage = lowInputVoltage,outputVoltage = outputVoltage)
```