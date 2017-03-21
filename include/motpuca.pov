// MOTPUCA DEFINITIONS  

#declare zoom = 1.1;            

#declare BackgroundColor = rgb <0.2, 0.25, 0.3>;

#declare draw_tumor    = yes;  
#declare draw_normal   = no;       
#declare draw_vessels  = yes;
#declare draw_barriers = no;    
#declare draw_nucleus  = no;
#declare states = no;
                       
#declare clip  = yes;    
                      
#declare glass = yes;  
#declare blur  = no;

#declare cell_exp = 1; 
#declare vessel_exp = 1;
           
//global_settings { max_trace_level 3 }

sky_sphere 
{
  pigment { BackgroundColor }
}
                                      
camera
 {
  location EyePos
  look_at <-40, 0, 0> 
  scale 1/zoom
  up y                
  right -image_width/image_height*x
//  right -1*x
  
  #if (blur)                  
    aperture 20
    focal_point <0, 0, 0>
    blur_samples 16
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
  rgb <1, 0, 0>, // alive
  rgb <1, 0, 0>, // hypoxia
  rgb <0.5, 0.5, 0.5>, // apoptosis
  rgb <0, 0, 0> // necrosis
  
}


#macro c(tissue, pos, r, state, clipped) 
  #if (clipped = 1 | clip = no | tissue = 1)
    sphere
    {
      pos, r*cell_exp
      #if (glass)          
        #if (states)
          pigment { rgbft <StateColor[state].x, StateColor[state].y, StateColor[state].z, 0, 0.75> }   
        #else
          pigment { rgbft <TissueColor[tissue].x, TissueColor[tissue].y, TissueColor[tissue].z, 0, 0.75> }   
        #end
        interior { ior 1.4 }     
      #else
        #if (states)
          pigment { StateColor[state] }   
        #else
          pigment { TissueColor[tissue] }        
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


#macro b(pos1, pos2, r)  
  merge
  {
    cylinder
    {
      pos1, pos2, r*vessel_exp
    } 
    sphere
    {
      pos1, r*vessel_exp
    } 
    sphere
    {
      pos2, r*vessel_exp
    } 
    #if (glass)
      pigment { rgbft <VesselColor.x, VesselColor.y, VesselColor.z, 0, 0.25> }   
      interior { ior 1.4 }     
    #else
      pigment { VesselColor }        
    #end
    finish { phong 0.5  reflection {0.5} }
}
#end
