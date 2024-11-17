import csv
import matplotlib.pyplot as plt

intensity = []
red = []
green = []
blue = []

with open("histogram.csv", "r") as csvfile:
    reader = csv.reader(csvfile)
    next(reader)  # Skip header

    for row in reader:
        intensity.append(int(row[0]))
        red.append(int(row[1]))
        green.append(int(row[2]))
        blue.append(int(row[3]))
        
plt.figure(figsize=(10, 6))
plt.plot(intensity, red, color='red', linestyle='-', label='Red Channel')
plt.plot(intensity, green, color='green', linestyle='-', label='Green Channel')
plt.plot(intensity, blue, color='blue', linestyle='-', label='Blue Channel')

plt.title("Picture Histogram")
plt.xlabel("Intensity")
plt.ylabel("Number of pixels")
plt.legend()

plt.savefig("histogram.png", format="png")
plt.close()

print("Saved image as 'histogram.png'")
