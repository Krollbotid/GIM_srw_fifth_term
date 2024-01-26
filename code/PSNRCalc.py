import numpy as np
import cv2
import csv
import os

def read_jpg(image_name):
	return cv2.imread(image_name+'.jpg')

def calculate_psnr(n):
	s = read_jpg('./source/kok')
	r = read_jpg('./recovery/kok'+n)

	# Convert images to floating point to prevent overflow during calculations
	s = s.astype(np.float64)
	r = r.astype(np.float64)

	# Compute the difference between the two images
	diff = cv2.absdiff(s, r)
	diff = np.square(diff)  # Element-wise multiplication to get squared differences

	# Calculate the mean squared error (MSE)
	mse = np.mean(diff)

	# If MSE is zero, PSNR is infinity
	psnr = float('inf')
	if mse != 0 :
		max_pixel_value = 255.0
		psnr = 10 * np.log10((max_pixel_value ** 2) / mse)
	return psnr


def write_csv(n,data):
	with open('./PSNR-result/'+n+'.csv', 'w', newline='') as myfile:
		wr = csv.writer(myfile, quoting=csv.QUOTE_ALL)
		wr.writerow(data)

for i in range(7):
	print("Creating CSV of PSNR-result",i,"...",sep="")
	write_csv(str(i),[calculate_psnr(str(i))])

os.system("pause")
