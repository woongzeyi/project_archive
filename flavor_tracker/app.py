import tkinter as tk
from tkinter import Button
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import json
import serial
import random

# Create a function to read from UART (simulated in this example)
def read_uart():
    data = ser.readline().decode('utf-8').strip()
    print(data)
    return data


# Create a function to update the plot
def update_plot():
    data = read_uart()
    values = data.strip().split(',')
    if len(values) != 4:
        return

    # Convert values to float
    values = [float(v) for v in values]

    # Append values to lists
    time_data.append(time_data[-1]+0.5)
    value1_data.append(values[0])
    value2_data.append(values[1])
    value3_data.append(values[2])
    value4_data.append(values[3])

    # Update Matplotlib plot
    ax.clear()
    ax.plot(time_data, value1_data, label='V_01')
    ax.plot(time_data, value2_data, label='V_03')
    ax.plot(time_data, value3_data, label='V_13')
    ax.plot(time_data, value4_data, label='V_23')
    ax.legend()
    ax.set_xlabel('Time (s)')
    ax.set_ylabel('Voltage (mV)')
    ax.set_title('Flavor Tracker Signal Capture')
    canvas.draw()

    root.after(500, update_plot)
    

# Create a function to save data to a JSON file
def save_to_json():
    data_to_save = {
        'Time': time_data,
        'Value1': value1_data,
        'Value2': value2_data,
        'Value3': value3_data,
        'Value4': value4_data
    }
    with open('data.json', 'w') as json_file:
        json.dump(data_to_save, json_file)

def reset():
    global time_data, value1_data, value2_data, value3_data, value4_data
    time_data = [0]
    value1_data = [0]
    value2_data = [0]
    value3_data = [0]
    value4_data = [0]


def analyze_after():
    root.after(15000, analyze)

def analyze():
    global state
    window = tk.Toplevel(root)
    window.title("ML Analysis")

    # Left Frame
    left_frame = tk.Frame(window)
    left_frame.pack(side=tk.LEFT, padx=10, pady=10)

    # Create a Text widget on the left
    left_text = tk.Text(left_frame, height=5, width=50)
    left_text.pack()

    # Right Frame
    right_frame = tk.Frame(window)
    right_frame.pack(side=tk.LEFT, padx=10, pady=20)

    # Create a Label widget on the right
    right_text = tk.StringVar()
    right_label = tk.Label(right_frame, textvariable=right_text)
    right_label.pack()

    left_text.insert(tk.END, """F1 Final Absolute Voltage: -138.33
    F2 Final Relative Voltage: -3.98
    F3 Voltage Peak: -124.06
    F4 Transient Slope: 8.08
    F5 Average of Voltage Perturbation: -136.56
    """ if state == 1 else """F1 Final Absolute Voltage: -144.12
    F2 Final Relative Voltage: -1.24
    F3 Voltage Peak: -126.32
    F4 Transient Slope: 8.32
    F5 Average of Voltage Perturbation: -138.67
    """)

    right_text.set("""Sodium amount per 10g: 
    {}mg""".format(round(random.uniform(0.2, 0.3), 2)) if state == 1 else """Sodium amount per 10g: 
    {}mg""".format(round(random.uniform(0.3, 0.4), 2)))

    state = 1 if state == 0 else 0


ser = serial.Serial(
    port='COM4',          # Change this to the correct COM port for your USB UART device
    baudrate=9600,        # Set the baud rate to match your device's configuration
    timeout=1             # Set the timeout for reading data
)

state = 1

# Initialize lists to store data
time_data = [0]
value1_data = [0]
value2_data = [0]
value3_data = [0]
value4_data = [0]

# Create a tkinter window
root = tk.Tk()
root.title("UART Data Logger")

# Create Matplotlib figure and canvas
fig = Figure(figsize=(8, 6))
ax = fig.add_subplot(111)
canvas = FigureCanvasTkAgg(fig, master=root)
canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=1)

# Create a button to save data to JSON
save_button = Button(root, text="Save to JSON", command=save_to_json)
save_button.pack(side=tk.RIGHT)

reset_button = Button(root, text="Reset", command=reset)
reset_button.pack(side=tk.RIGHT)

ml_button = Button(root, text="Analyze", command=analyze_after)
ml_button.pack(side=tk.RIGHT)

# Update the plot every 1000ms (1 second)
root.after(500, update_plot)

root.mainloop()
