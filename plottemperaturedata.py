# Ahmet Cetinkaya - 2023

import serial
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as pl
import matplotlib.animation as animation
import numpy as np
import threading
import time

sensor_data = {}
lock = threading.Lock()

what = {}
what2 = []
avgs = []
avgs_times = []
meds = []
meds_times = []

moving_avgs = []
moving_avgs_times = []

moving_meds = []
moving_meds_times = []

def plot():
    pl.rc('font', size=14)
    fig = pl.figure()

    def animate(i):
        lock.acquire()
        count = 0
        to_be_removed = []
        avg = 0
        medlist = []

        ncol = np.max(np.array([len(sensor_data.keys()), 2]))
        for addr in sensor_data.keys():
            count += 1
            xs = np.array(sensor_data[addr][0]) - time.time()
            if np.max(xs) < -10:
                to_be_removed.append(addr)
            ys = np.array(sensor_data[addr][1])

            avg += ys[-1] / len(sensor_data.keys())
            medlist.append(ys[-1])

            ax = pl.subplot(3, ncol, count)
            if xs.shape[0] > 0 and ys.shape[0] > 0:
                ax.clear()
                ax.plot(xs, ys, linestyle="dotted", linewidth=2, marker=".", markersize=10)
                ax.set_xlabel("Time")
                ax.set_ylabel("Temperature")
                ax.set_title(f"Device #{addr}")

        MOVING_COUNT = 15
        avgs.append(avg)
        avgs_times.append(time.time())

        meds.append(np.median(np.array(medlist)))
        meds_times.append(time.time())

        moving_avgs.append(np.array(avgs[-MOVING_COUNT:]).mean())
        moving_avgs_times.append(time.time())

        moving_meds.append(np.median(np.array(meds[-MOVING_COUNT:])))
        moving_meds_times.append(time.time())


        def splot(xs, ys, subplot_index, ylabel, title, color):
            ax = pl.subplot(3, ncol, subplot_index)
            if xs.shape[0] > 0 and ys.shape[0] > 0:
                ax.clear()
                ax.plot(xs, ys, color=color, linestyle="dotted", linewidth=2, marker=".", markersize=10)
                ax.set_xlabel("Time")
                ax.set_ylabel(ylabel)
                ax.set_title(title)

        splot(np.array(avgs_times[-20:]) - time.time(),
              np.array(avgs)[-20:],
              ncol + 1,
              "Average temperature",
              "Average",
              "C1")
        splot(np.array(meds_times[-20:]) - time.time(),
              np.array(meds)[-20:],
              ncol + 2,
              "Median temperature",
              "Median",
              "C1")
        splot(np.array(moving_avgs_times[-20:]) - time.time(),
              np.array(moving_avgs)[-20:],
              ncol + ncol + 1,
              "Moving average temperature",
              f"Moving Average (Average of last {MOVING_COUNT} averages)",
              "C2")
        splot(np.array(moving_meds_times[-20:]) - time.time(),
              np.array(moving_meds)[-20:],
              ncol + ncol + 2,
              "Moving median temperature",
              f"Moving Median (Median of last {MOVING_COUNT} medians)",
              "C2")



        for addr in to_be_removed: # These addresses did not provide any data for 10 seconds
            sensor_data.pop(addr)
        lock.release()

    ani = animation.FuncAnimation(fig, animate, interval=100)
    # print("SHOWING")
    pl.tight_layout()
    pl.subplots_adjust(wspace=0.35, hspace=0.35)
    pl.show()


threading.Thread(target=plot).start()


ser = serial.Serial('/dev/ttyUSB0', 115200)
while True:
    data = ser.readline()
    if data:
        # print(data.strip().decode('UTF-8'))
        lst = [int(v) for v in data.strip().decode('UTF-8').split(" ")]
        # print(lst)
        addr = lst[0]
        value = float(lst[1]) / 10
        timevalue = time.time()

        lock.acquire()
        if addr in sensor_data:
            sensor_data[addr][0].append(timevalue)
            sensor_data[addr][1].append(value)
            sensor_data[addr][0] = sensor_data[addr][0][-20:]
            sensor_data[addr][1] = sensor_data[addr][1][-20:]
        else:
            sensor_data[addr] = [[timevalue], [value]]

        # print(sensor_data)
        lock.release()
