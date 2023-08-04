# Delta Sigma Official Project
## Table of Content

[toc]
# Introduction
# Global Definitions

There are global Definitions that are nessesary to achive a operating System. These Global Properties are defined by the components or the System Designer.
```SysMD::Global
Global defines rect isA Function.
Global hasA SystemV:Voltage [V].
Global hasA ZeroV:Voltage [V].
Global hasA SystemClock:Voltage(0..SystemV) = rect(500kHz).
Global defines connectedTo isA Relationship.
Global defines hasValue isA Relationship.
```
# Definitions of Components
## Analog Base Components

These are all Components, that do not contain any other components and reessemble the analog components that are visible in any system.
### Condensator
```SysMD::Global
Condensator isA Component.
Condensator imports SI.
Condensator hasA capacity:Capacity(0..10000).
Condensator hasA inputVoltage:Voltage measurable input.
Condensator hasA charge:Charge = Condensator::capacity*Condensator::inputVoltage.
Condensator hasA outputVoltage : Voltage measurable output. //inputVoltage at t_0 + 1/C * \int^{t_0}_{t} I(a) da [v];
Condensator hasA outputCurrent : Current measurable output. //C * (V(t) * d/dt) [i].
```
### Resistor
```SysMD::Global
Resistor isA Component.
Resistor imports SI.
Resistor hasA inputVoltage: Voltage [V] measurable input.
Resistor hasA inputCurrent: Current [I] measurable input.
Resistor hasA resistance:Resistance [Omega].
Resistor hasA outputVoltage:Voltage [V] measurable output.
```
### Paralellconnection of two elements
This might be removed in the future!
```SysMD::Global
Paralell isA Component.
Paralell imports SI.
Paralell hasA inputVoltageA:Voltage [V] measurable input.
Paralell hasA inputVoltageB:Voltage [V] measurable input.
Paralell hasA outputVoltage:Voltage = if(Paralell::inputVoltageA>=Paralell::inputVoltageB) Paralell::inputVoltageA else Paralell::inputVoltageB measurable output.
Paralell hasA inputCurrentA:Current [A] measurable input.
Paralell hasA inputCurrentB:Current [A] measurable input.
Paralell hasA outputCurrent:Current = Paralell::inputCurrentA + Paralell::inputCurrentB measurable output.
```
### Operational Aplifier
```SysMD::Global
OperationalAmplifier isA Component.
OperationalAmplifier imports SI, Math.
OperationalAmplifier hasA highinputVoltage:Voltage [V] measurable input.
OperationalAmplifier hasA lowinputVoltage:Voltage [V] measurable input.
OperationalAmplifier hasA outputVoltage:Voltage [V] = if(h-l>0) amp*(h-l) else amp*(l-h) measurable output.
OperationalAmplifier hasA amplificationFactor:Real(100).
```
## Definition of Digital Base Components
This contains all components that are do not contain any other components and can be found in any other Digital Circuit 
### NAND
```SysMD::Global
NAND isA Component.
NAND imports SI.
NAND hasA inputVA : Voltage [V] measurable input.
NAND hasA inputVB : Voltage [V] measurable input.
NAND hasA threshholdV : Voltage [V].
NAND hasA outputV : Voltage [V] = if((NAND::inputVA<=NAND::threshholdV)&(NAND::inputVB<=NAND::threshholdV)) Global::SystemV else Global::ZeroV measurable output.
```
### Comperator
```SysMD::Global
COMP isA Component.
COMP imports SI.
COMP hasA lowInputVoltage : Voltage [V] measurable input.
COMP hasA highInputVoltage : Voltage [V] measurable input.
COMP hasA outputVoltage : Voltage [V] = if(highInputVoltage >= lowInputVoltage) SystemV else ZeroV measurable output.
```
## Definition Advanced Components
Advanced components consists of base components or advanced components.
### Voltage Devider
```SysMD::Global
VoltageDevider isA Component.
VoltageDevider imports SI.
VoltageDevider hasA HighInputVoltage:Voltage [V] measurable input.
VoltageDevider hasA LowInputVoltage:Voltage [V] measurable input.
VoltageDevider hasA resistance1:Resistance [Omega].
VoltageDevider hasA resistance2:Resistance [Omega].
VoltageDevider hasA OutputVoltage:Voltage [V] = (OutputVoltage::HighInputVoltage-OutputVoltage::LowInputVoltage)/(OutputVoltage::resistance1+OutputVoltage::resistance2)*OutputVoltage::resistance2 measurable output.
```
### Integrator
```SysMD::Global
Integrator isA Component.
Integrator imports SI.
Integrator hasA inputVoltage:Voltage [V] measurable input.
Integrator hasA outputVoltage:Voltage [V] measurable input.
Integrator hasA voltage2:Voltage [V].
Integrator hasA opAmp:OperationalAmplifier.
Integrator hasA c:Condensator.
Integrator::voltage2 connectTo Integrator::resistor1::outputVoltage.
Integrator::vd::HighInputVoltage connectTo Integrator::SystemV.
Integrator::vd::LowInputVoltage connectTo Integrator::ZeroV.
Integrator::vd::OutputVoltage connectTo Integrator::opAmp::highinputVoltage.
Integrator::opAmp::lowinputVoltage connectTo Integrator::voltage2.
Integrator::outputVoltage connectTo Integrator::opAmp::outputVoltage.
Integrator::c::inputVoltage connectTo Integrator::voltage2.
Integrator::outputVoltage connectTo Integrator::c::outputVoltage.
```
### Addion Element
```SysMD::Global
Summing isA Component.
Summing imports SI.
Summing hasA inputVoltage: [2..3] Voltage [V] measurable input.
Summing hasA outputVoltage:Voltage [V] measurable output.
Summing hasA voltage2:Voltage [V].
Summing hasA inputResistor: [2..3] Resistor.
Summing hasA summingResistor:Resistor
Summing hasA opAmp:OperationalAmplifier.
Summing hasA vd:VoltageDevider.
Summing::inputVoltage connectTo Summnig::inputResistor::inputVoltage.
Summnig::inputResistor::outputVoltage connectTo Summing::voltage2.
Summing::voltage2 connectTo Summing::summingResistor::inputVoltage.
Summing::vd::HighInputVoltage connectTo Summing::SystemV.
Summing::vd::LowInputVoltage connectTo Summing::ZeroV.
Summing::vd::OutputVoltage connectTo Summing::opAmp::highinputVoltage.
Summing::voltage2 connectTo Summing::opAmp::lowinputVoltage.
Summing::opAmp::lowinputVoltage connectTo Summing::outputVoltage.
Summing::summingResistor::outputVoltage connectTo Summing::outputVoltage.
```
### D-Flip-Flop
```SysMD::Global
DFlipFlop isA Component.
DFlipFlop imports SI.
DFlipFlop hasA Clock : Voltage [V] measurable.
DFlipFlop hasA inputD : Voltage [V] measurable input.
DFlipFlop hasA inputQ_t0 : Voltage [V] measurable input output.
DFlipFlop hasA inputNQ_t0 : Voltage [V] measurable input output.
DFlipFlop hasA NandS:NAND.
DFlipFlop hasA NandR:NAND.
DFlipFlop hasA NandQ:NAND.
DFlipFlop hasA NandNQ:NAND.
DFlipFlop hasA NotS : Voltage [V] = NandS::outputV.
DFlipFlop hasA NotR : Voltage [V] = NandR::outputV.
DFlipFlop::NandS::inputVA connectTo DFlipFlop::inputD.
DFlipFlop::NandS::inputVB connectTo DFlipFlop::Clock.
DFlipFlop::NandR::inputVA connectTo DFlipFlop::Clock.
DFlopFlop::NandR::inputVA connectTo DFlipFlop::NotS.
DFlipFlop::NandQ::inputVA connectTo DFlipFlop::NotS.
DFlipFlop::NandQ::inputVB connectTo DFlipFlop::inputNQ_t0.
DFlipFlop::NandNQ::inputVA connectTo DFlipFlop::NotR.
DFlipFlop::NandNQ::inputVB connectTo DFlipFlop::inputQ_t0.  
DFlipFlop::inputQ_t0 connectTo DFlipFlop::NandQ::outputV.
DFlipFlop::inputNQ_t0 connectTo DFlipFlop::NandNQ::outputV.
```
### Quantizer
```SysMD::Global
Quantizer isA Component.
Quantizer hasA register:DFlipFlop.
Quantizer hasA comperator:COMP.
Quantizer hasA vd:VoltageDevider.
Quantizer hasA inputVoltage:Voltage [V] measurable input.
Quantizer hasA outputVoltage:Voltage [V] measurable input.
Quantizer::register::Clock connectTo Global::SystemClock
Quantizer::vd::HighInputVoltage connectTo Global::SystemV.
Quantizer::vd::LowInputVoltage connectTo Global::ZeroV.
Quantizer::comperator::highInputVoltage connectTo Quantizer::inputVoltage.
Quantizer::comperator::lowInputVoltage connectTo Quantizer::vd::OutputVoltage.
Quantizer::comperator::outputVoltage connectTo Quantizer::register::inputD.
Quantizer::outputVoltage connectTo Quantizer::register::inputQ_t0.
```
### Factor
```SysMD::Global
Factor isA Component.
Factor hasA inputVoltage:Voltage [V] measurable input.
Factor hasA relationFactor:Real.
Factor hasA resistor:Resistor.
Factor hasA outputVoltage:Voltage [V] = Factor::relationFactor * Factor::inputVoltage measurable output.
Factor::resistor::inputVoltage connectTo Factor::inputVoltage.
Factor::resistor::outputVoltage connectTo Factor::outputVoltage.
```
# Complete System
```SysMD::Global
DeltaSigma imports SI.
DeltaSigma hasA Part a: [1..2] Factor.
DeltaSigma hasA Part b: [1..2] Factor.
DeltaSigma hasA Part c: [1..2] Factor.
DeltaSigma hasA Part adder: [1..2] Summing.
DeltaSigma hasA Part integrator: [1..2] Integrator.
DeltaSigma hasA Part quant:Quantizer.
DeltaSigma hasA inputVoltage:Voltage measurable input.
DeltaSigma hasA outputVoltage:Voltage measurable output.
DeltaSigma::inputVoltage connectTo DeltaSigma::a::inputVoltage.
DeltaSigma::outputVoltage connectTo DeltaSigma::b::inputVoltage.
DeltaSigma::a1::outputVoltage connectTo DeltaSigma::adder1::inputVoltage1.
DeltaSigma::b1::outputVoltage connectTo DeltaSigma::adder1::inputVoltage2.
DeltaSigma::adder1::outputVoltage connectTo DeltaSigma::integrator1::inputVoltage.
DeltaSigma::integrator1::outputVoltage connectTo DeltaSigma::c1::inputVoltage.
DeltaSigma::c1::outputVoltage connectTo DeltaSigma::adder2::inputVoltage3.
DeltaSigma::a2::outputVoltage connectTo DeltaSigma::adder2::inputVoltage1.
DeltaSigma::b2::outputVoltage connectTo DeltaSigma::adder2::inputVoltage2.
DeltaSigma::adder2::outputVoltage connectTo DeltaSigma::integrator1::inputVoltage.
DeltaSigma::integrator1::inputVoltage connectTo DeltaSigma::c2::inputVoltage.
DeltaSigma::c2::outputVoltage connectTo DeltaSigma::quant::inputVoltage.
DeltaSigma::quant::outputVoltage connectTo DeltaSigma::outputVoltage
```
