from paho.mqtt import client as mqtt_client
import paho.mqtt as mqtt
import json
import datetime
import numpy as np
import time
import matplotlib.pyplot as plt

def upperBoundChecker(values):
    output = np.empty(len(values))

    for i in range(len(values)):
        output[i] = (values[i]<12)
    
    return output

def lowerBoundChecker(values):
    output = np.empty(len(values))

    for i in range(len(values)):
        output[i] = (values[i]>-12)
    
    return output

def faultChecker(valuesUpper, valuesLower):
    output = np.empty(len(valuesUpper))

    for i in range(len(valuesUpper)):
        output[i] =  (not valuesLower[i]) or (not valuesUpper[i])
    
    return output


def main():
    x = np.linspace(0, 3*np.pi, num=1000)
    noisySin = 12*(np.sin(x))+np.random.normal(scale=0.5, size=len(x))

    # Define thresholds
    threshold1 = 12
    threshold2 = -12

    # Create discrete signals based on thresholds
    signal_above_threshold1 = noisySin > threshold1
    signal_below_threshold2 = noisySin < threshold2

    # Create a third signal for the conjunction of the two signals
    conjunction_signal = signal_above_threshold1 | signal_below_threshold2

    # Create subplots
    fig, axs = plt.subplots(4, 1, figsize=(12, 12), sharex=True)

    # Plot the sine wave with decision boundaries
    axs[0].plot(x, noisySin, label='Sine Wave', color='blue')
    axs[0].axhline(threshold1, color='green', linestyle=':', label='Threshold 1 (12V)')
    axs[0].axhline(threshold2, color='red', linestyle=':', label='Threshold 2 (-12V)')
    axs[0].set_title('Simulated input of AC Signal')
    axs[0].legend()
    axs[0].grid(True)

    # Plot the first discrete signal
    axs[1].plot(x, signal_above_threshold1, label='over 12V signal', linestyle='--', color='green')
    axs[1].set_title('Over 12V signal')
    axs[1].grid(True)

    # Plot the second discrete signal
    axs[2].plot(x, signal_below_threshold2, label='below -12V signal', linestyle='--', color='red')
    axs[2].set_title('Below -12V signal')
    axs[2].grid(True)

    # Plot the conjunction signal
    axs[3].plot(x, conjunction_signal, label='possible high component ware', linestyle='--', color='purple')
    axs[3].set_title('High component ware')
    axs[3].grid(True)

    # Labeling the x-axis
    plt.xlabel('x')

    # Adjust layout for better readability
    plt.tight_layout()

    # Show the plot
    plt.show()


    plt.Figure()
    fig, ax = plt.subplots()
    ax.plot(x, noisySin)
    ax.set_title('Simulation Signal')
    plt.plot(x, noisySin, label='Simulation Signal')
    plt.plot(x, upperBound, label='Upper Boundary Checker')
    plt.plot(x, lowerBound, label='Lower Boundary Checker')
    plt.plot(x, faultChecker(upperBound, lowerBound), label='Fault Occurance')
    plt.legend()
    plt.show()
    # client = connect_mqtt()
    # for elem in noisySin:
    #     publish(client=client,value=elem,topic=chargerTopic)
    #     time.sleep(4)
    #     publish(client=client,value=elem,topic=gridTopic)
    


if __name__ == "__main__":
    main()
