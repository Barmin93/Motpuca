// MOTPUCA DEFINITIONS  

#declare zoom = 1.5;


#declare draw_tumor    = yes;  
#declare draw_normal   = yes;       
#declare draw_tubes    = yes;
#declare draw_barriers = no;    
#declare draw_nucleus  = no;
#declare states = no;
#declare densO2 = no;
#declare densTAF = no;
                       
#declare clip  = yes;
                      
#declare glass = no;  
#declare blur  = no;

#declare cell_exp = 1; 
#declare vessel_exp = 1;
           
//global_settings { max_trace_level 1 }

sky_sphere 
{
  //pigment { BackgroundColor }
  pigment { rgb<1,1,1> }
}
                                      
camera
 {
  location EyePos
  look_at <0, 0, 0> 
  scale 1/zoom
  up y                
  right -1.5*x
  
  #if (blur)                  
    aperture 10
    focal_point <0, 0, 0>
    blur_samples 64
  #end
 }          
 
#macro transformation()
  matrix <ViewMatrix[0],  ViewMatrix[1],  ViewMatrix[2],
          ViewMatrix[4],  ViewMatrix[5],  ViewMatrix[6],
          ViewMatrix[8],  ViewMatrix[9],  ViewMatrix[10],
          ViewMatrix[12], ViewMatrix[13], ViewMatrix[14]>
#end                              
                                      
light_source
{
  LightSourcePos
  color rgb <1, 1, 1>
}                       
  

#declare line_width=0.5;                               

#macro ba(b_from, b_to, kind)
  box
  {
    b_from, b_to
    #if (kind = 0)
      pigment { rgbft <InBarrierColor.x, InBarrierColor.y, InBarrierColor.z, 0, 0.95> }
    #else
      pigment { rgbft <OutBarrierColor.x, OutBarrierColor.y, OutBarrierColor.z, 0, 0.95> }
    #end
  }        
  merge
  {
    cylinder { <b_from.x, b_from.y, b_from.z>, <b_to.x, b_from.y, b_from.z>, line_width }
    cylinder { <b_to.x, b_from.y, b_from.z>, <b_to.x, b_to.y, b_from.z>, line_width }
    cylinder { <b_to.x, b_to.y, b_from.z>, <b_from.x, b_to.y, b_from.z>, line_width }
    cylinder { <b_from.x, b_to.y, b_from.z>, <b_from.x, b_from.y, b_from.z>, line_width }
    cylinder { <b_from.x, b_from.y, b_to.z>, <b_to.x, b_from.y, b_to.z>, line_width }
    cylinder { <b_to.x, b_from.y, b_to.z>, <b_to.x, b_to.y, b_to.z>, line_width }
    cylinder { <b_to.x, b_to.y, b_to.z>, <b_from.x, b_to.y, b_to.z>, line_width }
    cylinder { <b_from.x, b_to.y, b_to.z>, <b_from.x, b_from.y, b_to.z>, line_width }
    cylinder { <b_from.x, b_from.y, b_from.z>, <b_from.x, b_from.y, b_to.z>, line_width }
    cylinder { <b_to.x, b_from.y, b_from.z>, <b_to.x, b_from.y, b_to.z>, line_width }
    cylinder { <b_from.x, b_to.y, b_from.z>, <b_from.x, b_to.y, b_to.z>, line_width }
    cylinder { <b_to.x, b_to.y, b_from.z>, <b_to.x, b_to.y, b_to.z>, line_width }   
    sphere { <b_from.x, b_from.y, b_from.z>, line_width }  
    sphere { <b_from.x, b_from.y, b_to.z>, line_width }  
    sphere { <b_from.x, b_to.y, b_from.z>, line_width }  
    sphere { <b_from.x, b_to.y, b_to.z>, line_width }  
    sphere { <b_to.x, b_from.y, b_from.z>, line_width }  
    sphere { <b_to.x, b_from.y, b_to.z>, line_width }  
    sphere { <b_to.x, b_to.y, b_from.z>, line_width }  
    sphere { <b_to.x, b_to.y, b_to.z>, line_width }  
    
    #if (kind = 0)
      pigment { InBarrierColor }
    #else
      pigment { OutBarrierColor }
    #end  
    no_shadow
  }   
#end                                 

#declare StateColor = array[6]
{
  rgb <1, 1, 0>,
  rgb <1, 1, 0>,
  rgb <0, 0.5, 1>, // alive
  rgb <0, 0.5, 0>, // hypoxia
  rgb <0.5, 0.5, 0.5>, // apoptosis
  rgb <0, 0, 0> // necrosis
  
}

#macro c_old(tissue, pos, r, state, dsO2, dsTAF, clipped) 
 sphere 
  { 
   pos, r*cell_exp*2, 1    
   pigment { TissueColor[tissue] }
  }
#end


#macro c(tissue, pos, r, state, dsO2, dsTAF, clipped) 
  #if (clipped = 1 | clip = no)
    sphere
    {
      pos, r*cell_exp
      #if (glass)          
        #if (states)
          pigment { rgbft <StateColor[state].x + TissueColor[tissue].x, StateColor[state].y + TissueColor[tissue].y, StateColor[state].z + TissueColor[tissue].z, 0, 0.75> }   
        #else                           
          #if (densO2)
            pigment { rgbft <dsO2, dsO2, dsO2, 0, 0.75> }   
          #else
            #if (densTAF)
              pigment { rgbft <0, dsTAF, 0, 0, 0.75> }   
            #else
              pigment { rgbft <TissueColor[tissue].x, TissueColor[tissue].y, TissueColor[tissue].z, 0, 0.75> }   
            #end  
          #end
        #end
        interior { ior 1.4 }     
      #else
        #if (states)
          pigment { StateColor[state]+TissueColor[tissue] }   
        #else
          #if (densO2)
            pigment { rgb <dsO2, dsO2, dsO2> }   
          #else
            #if (densTAF)
              pigment { rgb <0, dsTAF*0.5, dsTAF> }   
            #else
              pigment { TissueColor[tissue] }        
            #end
          #end
        #end
      #end
      finish { phong 0.5  reflection {0.5} }
    }                           
    #if (draw_nucleus)
    sphere
    {
      pos, r*0.2
      pigment { TissueColor[tissue] }        
      finish { phong 0.5  reflection {0.5} }
    }                           
    #end
  #end
#end


#macro b(pos1, pos2, r, tip)  
    cylinder
    {
      pos1, pos2, r*vessel_exp
      open
    } 
    sphere
    {
      pos1, r*vessel_exp
    }                      
    #if (tip)
      sphere
      {
        pos2, r*vessel_exp
      } 
    #end  
#end


#macro bc()
  #if (glass)
    pigment { rgbft <VesselColor.x, VesselColor.y, VesselColor.z, 0, 0.5> }   
    interior { ior 1.4 }     
  #else
    pigment { TubeColor }        
  #end
  finish { phong 0.5  reflection {0.5} }
#end  


#macro d(spl, spl_r, v_cnt)
  #local ctr = 0;              
  blob 
   {                           
    threshold 0.5
  #while (ctr <= v_cnt - 1)
    sphere
     {
      spl(ctr), 2*spl_r(ctr).x, 1
      #if (glass)
        pigment { rgbft <VesselColor.x, VesselColor.y, VesselColor.z, 0, 0.5> }   
      #else
        pigment { VesselColor }        
      #end
     }
    #declare ctr = ctr + 0.1;
  #end   
   finish { phong 0.5  reflection {0.5} }
   interior { ior 1.4 }     
   }
#end