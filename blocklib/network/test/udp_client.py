import socket
import time
import numpy as np

msgFromClient = "Hello UDP Server"

samples = list(range(100))
data = np.array(samples, dtype=np.uint8)*2
serverAddressPort = ("127.0.0.1", 8080)
bufferSize = 1000

# Create a UDP socket at client side
UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
# Send to server using created UDP socket

for ii in range(10):
    UDPClientSocket.sendto(data, serverAddressPort)
    time.sleep(0.1)

