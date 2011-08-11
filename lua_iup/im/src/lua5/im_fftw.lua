
-------------------------------------------------------------------------------
-- Creates a new function, with the name suffixed by "New". This new function
-- creates a new image, based on a source image, and calls the previous function
-- with this new image.
-- We assume here that the functions returns only one parameter or none.

local function OneSourceOneDest (funcname, width, height, color_space, data_type)
  local func = im[funcname]
  assert(func) -- see if function is really defined

  -- define function with "New" suffix
  im[funcname.."New"] = function (src_image, ...)
    -- create destination image
    local dst_image = im.ImageCreateBased(src_image, width, height, color_space, data_type)

    -- call previous method, repassing all parameters
    local ret = func(src_image, dst_image, ...)
    if (ret) then
      return ret, dst_image
    else
      return dst_image
    end
  end
end

-------------------------------------------------------------------------------
-- This function is similar to OneSourceOneDest, but it receives two source
-- images.

local function TwoSourcesOneDest (funcname, width, height, color_space, data_type)
  local func = im[funcname]

  -- see if function is really defined
  assert(func, string.format("undefined function `%s'", funcname))

  -- define function with "New" suffix
  im[funcname.."New"] = function (src_image1, src_image2, ...)
    -- create destination image
    local dst_image = im.ImageCreateBased(src_image1, width, height, color_space, data_type)

    -- call previous method, repassing all parameters
    local ret = func(src_image1, src_image2, dst_image, ...)
    if (ret) then
      return ret, dst_image
    else
      return dst_image
    end
  end
end

-------------------------------------------------------------------------------

TwoSourcesOneDest("ProcessCrossCorrelation")
OneSourceOneDest("ProcessAutoCorrelation", nil, nil, nil, im.CFLOAT)
OneSourceOneDest("ProcessFFT")
OneSourceOneDest("ProcessIFFT")
