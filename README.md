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
If the simulation was parallelized into multiple individual lower count simulations, a number of interfile projection files will be created. We provide a python script (`Concatenate_Interfile_From_GATE_NumberOfFiles.py`) that merged a given number of interfile projection image and print out the total number of counts in each set. https://github.com/BenAuer2021/Tools-To-Analyze-Simulation-Output/blob/main/Concatenate_Interfile_From_GATE_NumberOfFiles.py

## GATE Root Output
GATE can also create a root file (*.root) that includes all the details of the simulation. This output is generally prefered as it includes important details on the counts detected (e.g. scatter/primary, source/detected locations, energy, ...). Such file can thus be processed in a way that the simulation does not have to be re-run if the energy window is changed for example.

### Interactive Viewer
The content of any root file can be visualized in root via the following command,`root --web=off ROOT_FILE.root`. Then, type `TBrowser T` in the root prompt and a visualization window will pop up. You can right click on any TTree (Photopeak for example) and select StartViewer, a second window will pop up allowing you to visualize details of the simulation for example, detected locations in 3D by dragging the globalPosX, globalPosY, globalPosZ into the X,Y,Z flags of the second window (top left).

Example fo visualize the energy spectrum in the photpeak window,
<img width="1500" alt="Screen Shot 2023-06-22 at 11 25 09 AM" src="https://github.com/BenAuer2021/Tools-To-Analyze-Simulation-Output/assets/84809217/719e8c5d-3cfc-4092-94e7-6b8713469ff1">

### Sample Analysis ROOT Code for SPECT application

#### Details
We provide a sample root analysis code that produces the 2D projections for scatter, primary, and total events as well as with the [Triple Energy Window Scatter correction technique]{https://ieeexplore.ieee.org/abstract/document/97591} routinely used in clinic. The 2D projections are written as *img files (unsigned integer 32 bit), the total counts contained in each of these projection set is also print out. This code also produces the energy spectra for the photopeak and full spectrum for primary, scatter, and total counts. 

The following parameters need to be defined in the top section of the code,

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

#### How to Run it?
Start root by typing 'root' in a terminal window, and then compile the code and create libraries via `.L Projection_macro.cpp++`. The code can then be run via the command 'Projection_macros("Brightview_Main_LEHR_Tc99m_BoneScan_pos2_1000_files")'. For the code the be find in all directories in a given computer, add the Projection_macro.cpp file into the 'share/root/macros' folder.

![image](https://github.com/BenAuer2021/Tools-To-Analyze-Simulation-Output/assets/84809217/f44f3dfa-b081-4e4d-9753-22c32a72b860)

#### What output does it create?

The 'Projection_macro.cpp' code creates the 2D projection for the photopeak in a pdf file, as well as the scatter, primary, total energy spectra for the photopeak and full spectrum,
```ruby
FileNamePhotopeak_Projection2D.pdf
FileNamePhotopeakSpectra.pdf
FileNameFullSpectra.pdf
```
Scatter and primary events are sorted from the root file, and consist of a perfect differentiation.

<img width="712" alt="Screen Shot 2023-06-22 at 12 03 00 PM" src="https://github.com/BenAuer2021/Tools-To-Analyze-Simulation-Output/assets/84809217/0e612a58-e91c-4527-adf7-8e281702dc41">

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

<img width="811" alt="Screen Shot 2023-06-22 at 12 09 00 PM" src="https://github.com/BenAuer2021/Tools-To-Analyze-Simulation-Output/assets/84809217/3994cc93-5924-438a-bbaa-1029cc53b82e">

We provide the root file generated by GATE and *img and *pdf files created by the code. The bone scan was acquired without attenuation to improve simulation efficiency for this illustrative example.

Any of the *img file can be imported in Amide or ImageJ via the following parameters,
<img width="681" alt="image" src="https://github.com/BenAuer2021/Tools-To-Analyze-Simulation-Output/assets/84809217/001f403d-9d02-4aba-afc3-a974c2ed1e3b">








