import numpy as np
import cv2
import csv
import os

def read_jpg(image_name):
	return cv2.imread(image_name+'.jpg')

def calculate_psnr(n, image_name):
	s = read_jpg('./source/' + image_name)
	r = read_jpg('./encoded/' + image_name + str(n))

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

img = "kok"
psnr_values = []  # Create an empty list to store the PSNR values
for i in range(100):
    psnr = calculate_psnr(i, img)
    psnr_values.append(psnr)  # Append the PSNR value directly to the list

with open('./PSNR-result/' + img + '.csv', 'w', newline='\n') as myfile:
    wr = csv.writer(myfile)
    for value in psnr_values:
        wr.writerow([value])  # Write each PSNR value as a single item in a row