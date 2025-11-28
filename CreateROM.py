myArr = bytearray([i%256 for i in range(1024)])

with open('firmware.bin', 'wb') as f:
    f.write(myArr)