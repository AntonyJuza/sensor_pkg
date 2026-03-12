#!/usr/bin/env python3
"""
Simple helper script to read a DHT22 using Adafruit_DHT.
Prints: <temperature_celsius> <humidity_percent> on stdout

Usage: read_dht22.py <gpio_pin>
"""
import sys

try:
    import Adafruit_DHT
except Exception:
    print('nan nan')
    sys.exit(1)

def main():
    if len(sys.argv) != 2:
        print('nan nan')
        return
    try:
        pin = int(sys.argv[1])
    except ValueError:
        print('nan nan')
        return

    sensor = Adafruit_DHT.DHT22
    humidity, temperature = Adafruit_DHT.read_retry(sensor, pin)
    if humidity is None or temperature is None:
        print('nan nan')
        return

    print('{:.2f} {:.2f}'.format(temperature, humidity))

if __name__ == '__main__':
    main()
