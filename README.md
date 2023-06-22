# Tools-To-Analyze-Simulation-Output

GATE can produce projection data in interfile format that can be imported in Amide or ImageJ. 

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

Can be opened in ImageJ depending on the number of projections.

- Python code to merge interfile together
python3 Concatenate_Interfile_From_GATE.py Brain_Perfusion/LEHR_NoAttn_0.05Bq/Proj Brain_Perfusion_LEHR_NoAttn_0.05Bq


- How to read image in imageJ or Amide or Matlab

  
- Root Code to output the simulation file and form the projection



- Provide projection set plus create movie



