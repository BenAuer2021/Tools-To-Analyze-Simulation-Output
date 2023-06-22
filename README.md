# Tools-To-Analyze-Simulation-Output

## GATE Projection - Interfile Format

### Details from the GATE simulation
GATE can produce projection data in interfile format (*.hdr *.sin) that can be imported in [Amide]{https://amide.sourceforge.net} or [ImageJ]{https://imagej.net/software/fiji/}. 

In the bone imaging example described in details at the bottom in the **section 5** of this [page]{https://github.com/BenAuer2021/Simulation-Of-Nuclear-Medicine-Imaging-Systems-Scintigraphy-SPECT}, 64 projections of 230 by 170 (0.234 mm<sup>2</sup>) were created. Note, the projection plane is defined from the crystal compartment sizes and referencial. Thus, if the crystal or the SPECTHead volumes are shifted the projection plane shifts along the same parameter.

```ruby
/gate/output/projection/enable
/gate/output/projection/setInputDataName Photopeak
/gate/output/projection/setFileName ./PathTo/outputFileName
/gate/output/projection/projectionPlane YZ
/gate/output/projection/pixelSizeX 0.234 cm
/gate/output/projection/pixelSizeY 0.234 cm
/gate/output/projection/pixelNumberX 230
/gate/output/projection/pixelNumberY 170
```
### How to import projection file into an image viewer software?
To import the raw projection file in Amide or ImageJ, the below import parameters needs to be applied,
<img width="682" alt="image" src="https://github.com/BenAuer2021/Tools-To-Analyze-Simulation-Output/assets/84809217/fb19e9a8-b591-4001-940a-2bf7e4d4c622">

<img width="978" alt="Screen Shot 2023-06-22 at 11 04 04 AM" src="https://github.com/BenAuer2021/Tools-To-Analyze-Simulation-Output/assets/84809217/03fe5e6a-5f36-4ad0-abd2-bb573d1d3b87">

### How to merged projection files together?
If the simulation was parallelized into multiple individual lower count simulations, a number of interfile projection files will be created. We provide a python script that merged a given number of interfile projection image and print out the total number of counts in each set.


/usr/local/Cellar/root/6.26.06_2/share/root/macros



- Python code to merge interfile together
python3 Concatenate_Interfile_From_GATE.py Brain_Perfusion/LEHR_NoAttn_0.05Bq/Proj Brain_Perfusion_LEHR_NoAttn_0.05Bq


- How to read image in imageJ or Amide or Matlab

  
- Root Code to output the simulation file and form the projection



- Provide projection set plus create movie



