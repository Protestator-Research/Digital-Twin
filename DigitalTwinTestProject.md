 # Digital Twin Test Project
```SysMD::Global
Document uses ScalarValues, SI, ISO26262. 
```
Within this Project the Digital Twin Models are defined.

[toc]
# Definition of the Base Components
```SysMD::Global
SystemV:Voltage [V].
ZeroV:Voltage [V].
SystemClock:Voltage(0..SystemV) = rect(freq=500kHz)
```
```SysMD::Global
Condensator isA Component
Condensator imports SI.
Condensator hasA
    capacity: Capacity(10^(-12)..10000) [F]
    inputVoltage: Voltage() [V] measurable
    charge: Capacity*Volage [C] measurable
    outputVoltage : inputVoltage at t_0 + 1/C * \int^{t_0}_{t} I(a) da [v] mesurable
    outputCurrent : C * (V(t) * d/dt) [i]
   
```
```SysMD::Global
Condensator isA Component
Condensator imports SI.
Condensator hasA
    capacity: Capacity(10^(-12)..10000) [F]
    inputVoltage: Voltage() [V] measurable
    charge: Capacity*Volage [C] measurable
    outputVoltage : inputVoltage at t_0 + 1/C * \int^{t_0}_{t} I(a) da [v] mesurable
    outputCurrent : C * (V(t) * d/dt) [i]
```
```SysMD::Global
Resistor isA Component
Resistor imports SI.
Resistor hasA
    inputVoltage: Voltage() [V] measurable
    inputCurrent: Current() [I] measurable
    resistance:Resistace() [Omega]
    outputVoltage:Voltage() [V] measurable
```
```SysMD::Global
Paralell isA Component
Paralell imports SI.
Paralell hasA
    inputVoltageA:Voltage() [V] measurable
    inputVoltageB:Voltage() [V] measurable
    outputVoltage:Voltage() [V] measurable = ite(inputVoltageA>=inputVoltageB,inputVoltageA,inputVoltageB)
    inputCurrentA:Current() [A] measurable
    inputCurrentB:Current() [A] measurable
    outputCurrent=inputCurrentA+inputCurrentB
```
# Definition of the Base Digital Components
```SysMD::Global
NAND isA Component.
NAND imports SI.
NAND hasA
    inputVoltageA:Voltage() [V] measurable
    inputVoltageB:Voltage() [V] measurable        
    threshholdVoltage:Voltage() [V]
    outputVoltage:Voltage() [V] measurable = (!(inputVoltageA>=threshholdVoltage)&&(inputVoltageB>=threshholdVoltage))*SystemV
```
```SysMD::Global
COMP isA Component.
COMP imports SI.
COMP hasA
    lowInputVoltage : Voltage [V] measurable
    highInputVoltage : Voltage [V] measurable
    outputVoltage : Voltage [V] measurable = (highInputVoltage >= lowInputVoltage)*SystemV
```
# Advanced Components
```SysMD::Global
// VoltageDivider isA Component.
// VoltageDivider imports SI.
// VoltageDivider hasA
//    highInputVoltage:Voltage [V] measurable
//    lowInputVoltage:Voltage [V] measurable
//    outputVoltage:Voltage [V] measurable
//    UpperResistor:Resistor(inputVoltage = highInputVoltage,outputVoltage = outputVoltage)
//    LowerResistor:Resistor(inputVoltage = lowInputVoltage,outputVoltage = outputVoltage)
```
```SysMD::Global
Integrator isA Component.
Integrator imports SI.
Integrator hasA
    inputVoltage : Voltage [V] measurable
    resitstance:Resistance [Omega]
    outputVoltage : Voltage [V] measurable
    voltage2 : Voltage [V]
    resistor1: Resistor(inputVoltage=inputVoltage, outputVoltage=voltage2, resistance=resistance)
    opAmp : OperationalAmplifier(highinputVoltage = ZeroV,lowinputVoltage=voltage2, outputVoltage=outputVOltage)
    c:Condensator(inputVoltage=voltage2,outputVoltage=outputVoltage)
```
```SysMD::Global
DFlipFlop isA Component.
DFlipFlop imports SI.
DFlipFlop hasA
    Clock : Voltage [V] measurable
    inputD : Voltage [V] measurable
    NotS : Voltage [V] = NAND(inputVoltageA = inputD, inputVoltageB = Clock)
    NotR : Voltage [V] = NAND(inputVoltageA = Clock, inputVoltageB = NotS)
    inputQ_t0 : Voltage [V] measurable
    inputNQ_t0 : Voltage [V] measurable
    inputQ_t0 = NAND(inputVoltageA = NotS, inputVoltageB = inputNQ_t0)
    inputNQ_t0 = NAND(inputVoltageA = NotR, inputVoltageB = inputQ_t0)
```
```SysMD::Global
VoltageDevider isA Component.
VoltageDevider imports SI.
VoltageDevider hasA
    HighInputVoltage:Voltage [V] measurable
    LowInputVoltage:Voltage [V] measurable
    resistance1:Resistace() [Omega]
    resistance2:Resistace() [Omega]
    OutputVoltage:Voltage [V] measurable = (HighInputVoltage-LowInputVoltage)/(resistance1+resistance2)*resistance2
```
# Complete System
```SysMD::Global
DeltaSigma imports SI.
DeltaSigma hasA
    inputVoltage : Voltage() [V] measurable
    bitstreamVoltage : Voltage() [V] measurable
    outputVoltage:Voltage() [V] measurable
    
    C2Voltage:Voltage() [V] = Condensator(inputVoltage = inputVoltage,capacity = 10mu)     
    voltage2 = Paralell(inputvoltageA = C2Voltage, inputVoltageB=inputVoltage)
    R10 = Resitor(inputVoltage=SystemV,resistance = 22k, outputVoltage = voltage2)
    R9 = Resistor(inputVoltage=voltage2,resitstance=22k, outputVoltage = ZeroV)    
    I1InputVoltage:Voltage [V]
    integrator1 = Integrator(inputVoltage=I1InputVoltage, resistance=0)
    R3 = Resistor(inputVoltage=voltage2,resistance=4.7k)    
    R8 = Resistor(inputVoltage=integrator1.outputVoltage,resistance = 10)
    R4 = Resistor(inputVoltage=bitStreamVoltage, resistance 4,7k)
    I1InputVoltage=ite((R3.outputVoltage >=R4.outputVoltage)&&(R3.outputVoltage>=R8.outputVoltage),R3.outputVoltage,ite(R4.outputVoltage>=R8.outputVoltage,R4.outputVoltage,R8.outputVoltage))
    R15 = Resistor(inputVoltage=integrator1.outputVoltage,resistance=10k)
    R14 = Resistor(inputVoltage = outputVoltage, resistance = 22k)    
    I2InputVoltage = ite(R15.outputVoltage    
    integrator2 = Integrator(inputVoltage=I2InputVoltage,resistance = 0)
    R16 = Resistor(inputVoltage=integrator2.outputVoltate, resistance 100k)
    vd1 = VoltageDevider(HighInputVoltage=SystemV,LowInputVoltage=ZeroV,resistance1=2,2k,resistance2=2,2k)
    comp1 = COMP(lowInputVoltage=vd1.OutputVoltage, highInputVoltage=R16.outputVoltage)
    flipFlop = DFlipFlop(Clock=SystemClock, inputD = comp1.outputVoltage, inputQ_t0=outputVoltage, inputNQ_t0=bitstreamVoltage)
```
