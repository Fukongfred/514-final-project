from machine import Pin, ADC, PWM
import time

# Initialize the ADC pin for FSR
fsr = ADC(Pin(D0))
fsr.atten(ADC.ATTN_11DB)  # Configure the attenuation for maximum scale range

# Initialize the PWM pin for Buzzer
buzzer = PWM(Pin(D1), freq=2000, duty=512)  # Start with a 2kHz frequency and 50% duty cycle

threshold = 100  # Define the weight change threshold for stopping the buzzer
initial_weight = fsr.read()  # Read the initial weight

def check_weight_change():
    current_weight = fsr.read()
    if abs(current_weight - initial_weight) > threshold:
        return True
    return False

# Main loop
try:
    while True:
        if check_weight_change():
            buzzer.deinit()  # Stop the buzzer
            print("Weight change detected, stopping buzzer.")
            break  # Exit the loop
        time.sleep(0.1)  # Delay for a bit to avoid spamming

except KeyboardInterrupt:
    buzzer.deinit()  # Ensure the buzzer is stopped if the script is interrupted
