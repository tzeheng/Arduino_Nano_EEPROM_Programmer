import serial
import argparse
import time

def read_in_chunks(file_object, chunk_size=64):
    while True:
        data = file_object.read(chunk_size)
        if not data:
            break
        yield data


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='''TH EEPROM Programmer''',
                                     prog="EEPROM",
                                     usage='python %(prog)s [options]',
                                     epilog='''Please like and subscribe''')
    parser.add_argument("-p",
                        help="COM Port of TEEPROM Programmer",
                        type=str,
                        required=True)
    parser.add_argument("-m",
                        help="Software mode: w(rite), e(rase) or r(ead)",
                        type=str,
                        required=True)
    parser.add_argument("-s",
                        help="EEPROM size to write/erase/read in kB",
                        type=int,
                        required=True)
    parser.add_argument("-f",
                        help="Path to firmware bin file",
                        type=str)
    args = parser.parse_args()

    ser = serial.Serial(args.p, 57600, serial.EIGHTBITS, serial.PARITY_NONE, serial.STOPBITS_ONE)
    dat = ""
    print("Waiting for TEEPROM serial...")
    while (dat != b'<TEEPROM flasher ready>\r\n'):
        dat = ser.readline()
    print(dat.decode('utf-8'))

    # READ EEPROM
    if(args.m == 'r'):
        ser.write(str.encode(args.m+str(args.s)+"\n"))
        response = ser.readline().rstrip()
        print(response.decode('utf-8'))
        response = ser.readline().rstrip()
        while(response != b'DONE!'):
            print(response.decode('utf-8'))
            response = ser.readline().rstrip()
        print(response.decode('utf-8'))

    # ERASE EEPROM
    if (args.m == 'e'):
        ser.write(str.encode(args.m+str(args.s)+"\n"))
        response = ser.readline().rstrip()
        print(response.decode('utf-8'))
        response = ser.readline().rstrip()
        while (response != b'DONE!'):
            print(response.decode('utf-8'))
            response = ser.readline().rstrip()
        print(response.decode('utf-8'))

    # WRITE EEPROM
    if (args.m == 'w'):
        loop = 0
        ser.write(str.encode(args.m + str(args.s) + "\n"))
        response = ser.readline().rstrip()
        print(response.decode('utf-8'))

        f = open(args.f, 'rb')
        for piece in read_in_chunks(f):
            loop = loop + 1
            print(piece.hex())
            piece += b'\0'
            ser.write(piece)
            time.sleep(1)
            if (loop >= args.s * 2):
                break
        f.close()


        # ser.write( #64 bytes per write
        #     b'\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\n')
        # time.sleep(1)
        # ser.write(
        #     b'\x00\x00\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\x01\x02\x03\x04\x05\x06\x07\x08\n')
        # time.sleep(1)
        response = ser.readline().rstrip()
        while (response != b'DONE!'):
            print(response.decode('utf-8'))
            response = ser.readline().rstrip()
        print(response.decode('utf-8'))
    # values = bytearray([4, 9, 62, 144, 56, 30, 147, 3, 210, 89, 111, 78, 184, 151, 17, 129])
    # ser.write(values)