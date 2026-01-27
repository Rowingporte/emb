from pymodbus.client import ModbusTcpClient
import time
import pygame

axis1, axis2, axis3, axis4, open = 0


def number_to_raw_data(val):
    if val < 0:
        val = (1 << 15) - val
    return val


def control_robot(request, direction):
    global axis1, axis2, axis3, axis4
    
    if direction == 'upl':
        axis1 += 100
    elif direction == 'downl':
        axis1 -= 100
    elif direction == 'leftl':
        axis2 += 100
    elif direction == 'rightl':
        axis2 -= 100
        
        
    
    elif direction == 'upr':
        axis3 += 100
    elif direction == 'downr':
        axis3 -= 100
    elif direction == 'leftr':
        axis4 += 100
    elif direction == 'rightr':
        axis4 -= 100

if name == 'main':
    print("--- START")
    client = ModbusTcpClient('192.168.228.215', port=5020)

    client.connect()
    print("Connected to Modbus server")

    print("Calibrate Robot if needed")
    client.write_register(311, 1)
    time.sleep(1)

    while client.read_input_registers(402, 1).registers[0] == 1:
        time.sleep(0.05)


    try:
        while True:
            
            joints_to_send = [axis1, axis2, axis3, axis4]
            joints_to_send_int = [int(val) for val in joints_to_send]
            print(joints_to_send_int)
            client.write_registers(0, joints_to_send_int)
            client.write_register(100, 1)
            while client.read_holding_registers(150, count=1).registers[0] == 1:
                time.sleep(0.01)



    except KeyboardInterrupt:
        pass

    client.close()
    print("Close connection to Modbus server")
    print("--- END")