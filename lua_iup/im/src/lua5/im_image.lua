-- If all parameteres, besides the image, are nil, this is equivalent to image:Clone.
-- If any parameter is not nil, then the value is used instead of the one from the source image.
-- If a parameter is a function, then the function is called, passing the source
-- image as parameter, to obtain the substituion value.

function im.ImageCreateBased(image, width, height, color_space, data_type)        
  -- default values are those of the source image                                 
  width       = width       or image:Width()                                      
  height      = height      or image:Height()                                     
  color_space = color_space or image:ColorSpace()                                 
  data_type   = data_type   or image:DataType()                                   
                                                                                  
  -- callback to calculate parameters based on source image                       
  if type(width)       == "function" then       width = width(image) end        
  if type(height)      == "function" then      height = height(image) end       
  if type(color_space) == "function" then color_space = color_space(image) end  
  if type(data_type)   == "function" then   data_type = data_type(image) end    
                                                                                  
  -- create a new image                                                           
  local new_image = im.ImageCreate(width, height, color_space, data_type)               
  image:CopyAttributes(new_image)                                                 
  if (image:HasAlpha()) then new_image:AddAlpha() end
  return new_image                                                                
end                                                                               
