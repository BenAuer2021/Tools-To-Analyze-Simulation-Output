# Tools-To-Analyze-Simulation-Output

These tutorials are developed and maintained by Auer Benjamin from the Brigham and Women's Hospital and Harvard Medical School, Boston, MA, USA.

**Contact:** Auer, Benjamin Ph.D <bauer@bwh.harvard.edu>

Table of contents:
```diff
- 1. GATE Projection - Interfile Format
- 2. Sample Analysis ROOT Code for SPECT application
```

# 1. GATE Projection - Interfile Format

### Details from the GATE simulation
GATE can produce projection data in interfile format (*.hdr *.sin) that can be imported in [Amide](https://amide.sourceforge.net) or [ImageJ](https://imagej.net/software/fiji/) for example. 

In the bone imaging example described in details at the bottom in the **section 5** of this [page](https://github.com/BenAuer2021/Simulation-Of-Nuclear-Medicine-Imaging-Systems-Scintigraphy-SPECT), 64 projections of 230 by 170 (0.234 mm<sup>2</sup>) were created. Note, the projection plane is defined from the crystal compartment sizes and referencial. Thus, if the crystal or the SPECTHead volumes are shifted the projection plane shifts along the same parameter.

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
### How to import projection file into an image viewer software
To import the raw projection file in Amide or ImageJ, the below import parameters need to be applied:
<img width="514" alt="Screen Shot 2023-08-16 at 6 28 12 PM" src="https://github.com/BenAuer2021/Tools-To-Analyze-Simulation-Output/assets/84809217/bb89e021-1fab-41f2-a61b-5c9a40bc1d1c">

<img width="1300" alt="image" src="https://github.com/BenAuer2021/Tools-To-Analyze-Simulation-Output/assets/84809217/cf432bc5-21e3-44f2-8c4c-28e0ceb7bf41">

In ImageJ, use File -> Import Raw... on the .sin file and then use the same parameters shown above. 

### How to merged projection files together?
If the simulation was parallelized into multiple individual lower count simulations, a number of interfile projection files will be created. We provide a python script (`Concatenate_Interfile_From_GATE_NumberOfFiles.py`) that merges a given number of interfile projection image and prints out the total number of counts in each set. The file is available here: https://github.com/BenAuer2021/Tools-To-Analyze-Simulation-Output/blob/main/Concatenate_Interfile_From_GATE_NumberOfFiles.py

# 2. GATE Root Output
GATE can also create a root file (*.root) that includes all the details of the simulation. This output is generally prefered as it includes important details on the counts detected (e.g. scatter/primary, source/detected locations, energy, ...). The root file can thus be processed in a way that the simulation does not have to be re-run if the energy window is changed for example.

## Interactive Viewer
The content of any root file can be visualized in root via the following command,`root --web=off ROOT_FILE.root`. Then, type `TBrowser T` in the root prompt and a visualization window will pop up. You can right click on any TTree (Photopeak for example) and select StartViewer, a second window will pop up allowing you to visualize details of the simulation for example, detected locations in 3D by dragging the globalPosX, globalPosY, globalPosZ into the X,Y,Z flags of the second window (top left).

Example to visualize the energy spectrum in the photpeak window:
<img width="1500" alt="image" src="https://github.com/BenAuer2021/Tools-To-Analyze-Simulation-Output/assets/84809217/b80aeb8f-9d40-4a82-9fa9-135ea15ef6c8">

https://github.com/BenAuer2021/Tools-To-Analyze-Simulation-Output/assets/84809217/1009fcbf-a270-4bd0-81e6-39b23ad58c3a

## Sample Analysis ROOT Code for SPECT applications

### Details
We provide a sample root analysis code that produces the 2D projections for scatter, primary, and total events. It also applies the [Triple Energy Window Scatter correction technique](https://ieeexplore.ieee.org/abstract/document/97591) routinely used in clinical practise for scatter correction. The 2D projections are written as *.img files (unsigned integer 32 bit), the total counts contained in each of these projection set is also printed out. This code also produces the energy spectra for the photopeak and full spectrum for primary, scatter, and total counts. 

The following parameters need to be defined in the top section of the code:

 ```ruby 
UInt_t nProjection_per_head = 32; //60
Float_t angleStep = 5.6250; // 3 degree
Float_t anglePitch = 180; //H-Mode
Float_t crystalSize_transaxial = 538.2; //mm
Float_t crystalSize_axial = 397.8; //mm
UInt_t nPix_transaxial = 230; //pixel size 0.234 cm
UInt_t nPix_axial = 170;
UInt_t nHead = 2;
Float_t Shift_Oz = 0; //in mm Shift of the detector head
// Energy Window for Tc-99m 20%, 10%, 10%
Float_t Photopeak_E_min=0.126, Photopeak_E_max=0.154;
Float_t Scatter_Upper_E_min=0.112, Scatter_Upper_E_max=0.126;
Float_t Scatter_Lower_E_min=0.154, Scatter_Lower_E_max=0.168;
```

### How to run the code
Start root by typing `root` in a terminal window, and then compile the code and create libraries via `.L Projection_macro.cpp++`. The code can then be run via the command `Projection_macros("LEHR_BoneScan_pos2_0.5Bq_Correct_100_files")`. For the code the be found in all directories in a given computer, add the Projection_macro.cpp file into the `share/root/macros` folder.

![image](https://github.com/BenAuer2021/Tools-To-Analyze-Simulation-Output/assets/84809217/8a23f7c6-fdfb-4e43-8688-943bb1585343)


### What output does it create?

The `Projection_macro.cpp` code creates the 2D projection for the photopeak in a pdf file, as well as the scatter, primary, total energy spectra for the photopeak and full spectrum,
```ruby
FileNamePhotopeak_Projection2D.pdf
FileNamePhotopeakSpectra.pdf
FileNameFullSpectra.pdf
```
Scatter and primary events are sorted from the root file, and consist of a perfect differentiation.

<img width="714" alt="Screen Shot 2023-08-16 at 6 40 48 PM" src="https://github.com/BenAuer2021/Tools-To-Analyze-Simulation-Output/assets/84809217/92968e50-90e3-46aa-bdc8-094e63fb31c7">

The code also creates multiple projections in raw format,
 ```ruby
# Photopeak projection - energy range is defined in the code
FileName_Projection_Photopeak_170x230x64_PixSize_2.34mmx2.34mm.img
# Upper scatter window projection for TEW - energy range defined in the code
FileName_Projection_UpperScatterWindow_170x230x64_PixSize_2.34mmx2.34mm.img
# Lower scatter window projection for TEW - energy range defined in the code
FileName_Projection_LowerScatterWindow_170x230x64_PixSize_2.34mmx2.34mm.img
# TEW projection - energy range defined in the code
FileName_Projection_Photopeak_ScatterViaTEW_170x230x64_PixSize_2.34mmx2.34mm.img
# True Primary photopeak projection - differentiate via GATE ROOT output flag
FileName_Projection_Photopeak_Primary_170x230x64_PixSize_2.34mmx2.34mm.img
# True Scatter photopeak projection - differentiate via GATE ROOT output flag
FileName_Projection_Photopeak_ScatterFromGate_170x230x64_PixSize_2.34mmx2.34mm.img
```

<img width="812" alt="Screen Shot 2023-08-16 at 6 41 57 PM" src="https://github.com/BenAuer2021/Tools-To-Analyze-Simulation-Output/assets/84809217/ac78d50c-fca2-4470-a5d3-f262f6d92903">

We provide the root file generated by GATE and *img and *pdf files created by the code. The second bed position of the bone scan was acquired without attenuation to improve simulation efficiency for this illustrative example. See https://github.com/BenAuer2021/Simulation-Of-Nuclear-Medicine-Imaging-Systems-Scintigraphy-SPECT/edit/main/README.md section 5.

Any of the *img file can be imported in Amide or ImageJ via the following parameters,
<img width="681" alt="image" src="https://github.com/BenAuer2021/Tools-To-Analyze-Simulation-Output/assets/84809217/ef583a6f-ac4d-42ab-8fd7-30d8e7654ad1">









