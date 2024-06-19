import numpy as np
import cv2
import os
import sys

def read_jpg(image_name):
    return cv2.imread(image_name + '.jpg')

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
    if mse != 0:
        max_pixel_value = 255.0
        psnr = 10 * np.log10((max_pixel_value ** 2) / mse)
    return psnr

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python psnrone.py image_name")
        sys.exit(1)

    img = sys.argv[1]
    psnr = calculate_psnr(101, img)

    output_path = './PSNR-result/' + img + "full" + '.txt'
    os.makedirs(os.path.dirname(output_path), exist_ok=True)

    with open(output_path, 'w') as myfile:
        myfile.write(f"{psnr}\n")