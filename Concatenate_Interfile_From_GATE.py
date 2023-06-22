import numpy as np
import matplotlib.pyplot as plt
import os
import glob
import sys

####################################### Code To concatenate interfile data (.hdr .sin) generated from GATE #######################
## Usage: python3 Concatenate_Interfile_From_GATE.py path output_filename
## Example: python3 Concatenate_Interfile_From_GATE.py Brain_Perfusion/LEHR_NoAttn_0.05Bq/Proj Brain_Perfusion_LEHR_NoAttn_0.05Bq
## Developed by Benjamin Auer, PhD from the Brigham and Women's Hospital and Harvard Medical School, Boston, MA, USA / Email: bauer@bwh.harvard.edu
########################################################################################################################
def read_interfile_header(folder_path):
    header_files = glob.glob(os.path.join(folder_path, '*-0.hdr'))
    if len(header_files) == 0:
        raise FileNotFoundError("No header file found.")
    header_file = header_files[0]
    dimensions = {}
    with open(header_file, 'r') as f:
        for line in f:
            line = line.strip()
            if line.startswith('!matrix size [1]'):
                dimensions['SizeX'] = int(line.split(':=')[1].strip())
            elif line.startswith('!matrix size [2]'):
                dimensions['SizeY'] = int(line.split(':=')[1].strip())
            elif line.startswith('!number of projections'):
                dimensions['SizeZ'] = int(line.split(':=')[1].strip())*2
    return dimensions
    
def load_interfile_images(folder_path, dimensions, num_files=None):
    image_files = glob.glob(os.path.join(folder_path, '*.sin'))
    summed_image = None
    summed_image_files = []

    if num_files is not None:
        image_files = image_files[:num_files]

    for file in image_files:
        data = np.fromfile(file, dtype=np.uint16)
        image = np.reshape(data, (dimensions['SizeZ'], dimensions['SizeY'], dimensions['SizeX']), order='C')
        
        total=np.sum(image)
        filename=os.path.basename(file)
        print(f"file: {filename}, Ncts: {total}")

        if summed_image is None:
            summed_image = image
        else:
            summed_image += image

        summed_image_files.append(os.path.basename(file))

    return summed_image, summed_image_files

def main():
    if len(sys.argv) < 4:
        print("Usage: python sum_interfile_images.py <folder_path> <output_filename> <num_files>")
        return

    folder_path = sys.argv[1]
    
    # Get the number of files to sum from the command-line argument
    num_files = int(sys.argv[3]) if len(sys.argv) > 3 else None

    # Read the dimensions from the header file
    dimensions = read_interfile_header(folder_path)

    # Load and sum the interfile images
    summed_image_Interfile, summed_image_files = load_interfile_images(folder_path, dimensions, num_files)

    # Print the image files being summed
    print("Images being summed:")
    for file in summed_image_files:
        print(file)
    
    # Print the number of images being summed
    num_images_summed = len(summed_image_files)
    print(f"Number of images summed: {num_images_summed}")

    # Calculate the sum of all pixel values in the summed image
    pixel_sum = np.sum(summed_image_Interfile)
    print(f"Total Number of counts: {pixel_sum}")

    # Save the summed image in the interfile format
    output_filename = sys.argv[2] + "_" + str(len(summed_image_files)) + '.sin'
    output_path = os.path.join(folder_path, output_filename)

    # Reshape the summed image to match the interfile dimensions
    summed_image_reshaped = np.reshape(summed_image_Interfile, (-1,), order='C')

    # Save the summed image as a binary file
    summed_image_reshaped.tofile(output_path)

    print(f"Summed image saved as {output_path}")

    #TO install a python package python3 -m pip install matplotlib
    
if __name__ == "__main__":
    main()
    
    
