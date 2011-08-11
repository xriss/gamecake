
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
-- This function is similar to OneSourceOneDest, but it receives three source
-- images.

local function ThreeSourcesOneDest (funcname, width, height, color_space, data_type)
  local func = im[funcname]
  assert(func) -- see if function is really defined

  -- define function with "New" suffix
  im[funcname.."New"] = function (src_image1, src_image2, src_image3, ...)
    -- create destination image
    local dst_image = im.ImageCreateBased(src_image1, width, height, color_space, data_type)

    -- call previous method, repassing all parameters
    local ret = func(src_image1, src_image2, src_image3, dst_image, ...)
    if (ret) then
      return ret, dst_image
    else
      return dst_image
    end
  end
end

-------------------------------------------------------------------------------
-- This function is similar to OneSourceOneDest, but it creates two destiny
-- images.

local function OneSourceTwoDests (funcname, width, height, color_space, data_type)
  local func = im[funcname]
  assert(func) -- see if function is really defined

  -- define function with "New" suffix
  im[funcname.."New"] = function (src_image, ...)
    -- create destination image
    local dst_image1 = im.ImageCreateBased(src_image, width, height, color_space, data_type)
    local dst_image2 = im.ImageCreateBased(src_image, width, height, color_space, data_type)

    -- call previous method, repassing all parameters
    local ret = func(src_image, dst_image1, dst_image2, ...)
    if (ret) then
      return ret, dst_image1, dst_image2
    else
      return dst_image1, dst_image2
    end
  end
end

-------------------------------------------------------------------------------
-- This function is similar to OneSourceOneDest, but it creates three destiny
-- images.

local function OneSourceThreeDests (funcname, width, height, color_space, data_type)
  local func = im[funcname]
  assert(func) -- see if function is really defined

  -- define function with "New" suffix
  im[funcname.."New"] = function (src_image, ...)
    -- create destination image
    local dst_image1 = im.ImageCreateBased(src_image, width, height, color_space, data_type)
    local dst_image2 = im.ImageCreateBased(src_image, width, height, color_space, data_type)
    local dst_image3 = im.ImageCreateBased(src_image, width, height, color_space, data_type)

    -- call previous method, repassing all parameters
    local ret = func(src_image, dst_image1, dst_image2, dst_image3, ...)
    if (ret) then
      return ret, dst_image1, dst_image2, dst_image3
    else
      return dst_image1, dst_image2, dst_image3
    end
  end
end

-------------------------------------------------------------------------------

local function hough_height(image)
  local function sqr(x) return x*x end
  local rmax = math.sqrt(sqr(image:Width()) + sqr(image:Height())) / 2
  return 2*rmax+1
end

OneSourceOneDest("AnalyzeFindRegions", nil, nil, nil, im.USHORT)
OneSourceOneDest("ProcessPerimeterLine")
OneSourceOneDest("ProcessRemoveByArea")
OneSourceOneDest("ProcessFillHoles")
OneSourceOneDest("ProcessHoughLines", 180, hough_height, im.GRAY, im.INT)
OneSourceOneDest("ProcessHoughLinesDraw")
OneSourceOneDest("ProcessDistanceTransform", nil, nil, nil, im.FLOAT)
OneSourceOneDest("ProcessRegionalMaximum", nil, nil, im.BINARY, nil)

function im.ProcessReduceNew (src_image, width, height)
  local dst_image = im.ImageCreateBased(src_image, width, height)
  return im.ProcessReduce(src_image, dst_image), dst_image
end

function im.ProcessResizeNew (src_image, width, height)
  local dst_image = im.ImageCreateBased(src_image, width, height)
  return im.ProcessResize(src_image, dst_image), dst_image
end

OneSourceOneDest("ProcessReduceBy4", function (image) return image:Width() / 2 end,
                                     function (image) return image:Height() / 2 end)

function im.ProcessCropNew (src_image, xmin, xmax, ymin, ymax)
  local width = xmax - xmin + 1
  local height = ymax - ymin + 1
  local dst_image = im.ImageCreateBased(src_image, width, height)
  im.ProcessCrop(src_image, dst_image, xmin, ymin)
  return dst_image
end

TwoSourcesOneDest("ProcessInsert")

function im.ProcessAddMarginsNew (src_image, xmin, xmax, ymin, ymax)
  local width = xmax - xmin + 1
  local height = ymax - ymin + 1
  local dst_image = im.ImageCreateBased(src_image, width, height)
  im.ProcessAddMargins(src_image, dst_image, xmin, ymin)
  return dst_image
end

function im.ProcessRotateNew (src_image, cos0, sin0, order)
  local width, height = im.ProcessCalcRotateSize(src_image:Width(), src_image:Height(), cos0, sin0)
  local dst_image = im.ImageCreateBased(src_image, width, height)
  return im.ProcessRotate(src_image, dst_image, cos0, sin0, order), dst_image
end

OneSourceOneDest("ProcessRotateRef")
OneSourceOneDest("ProcessRotate90", function (image) return image:Height() end, function (image) return image:Width() end)
OneSourceOneDest("ProcessRotate180")
OneSourceOneDest("ProcessMirror")
OneSourceOneDest("ProcessFlip")
OneSourceOneDest("ProcessRadial")
OneSourceOneDest("ProcessGrayMorphConvolve")
OneSourceOneDest("ProcessGrayMorphErode")
OneSourceOneDest("ProcessGrayMorphDilate")
OneSourceOneDest("ProcessGrayMorphOpen")
OneSourceOneDest("ProcessGrayMorphClose")
OneSourceOneDest("ProcessGrayMorphTopHat")
OneSourceOneDest("ProcessGrayMorphWell")
OneSourceOneDest("ProcessGrayMorphGradient")
OneSourceOneDest("ProcessBinMorphConvolve")
OneSourceOneDest("ProcessBinMorphErode")
OneSourceOneDest("ProcessBinMorphDilate")
OneSourceOneDest("ProcessBinMorphOpen")
OneSourceOneDest("ProcessBinMorphClose")
OneSourceOneDest("ProcessBinMorphOutline")
OneSourceOneDest("ProcessBinMorphThin")
OneSourceOneDest("ProcessMedianConvolve")
OneSourceOneDest("ProcessRangeConvolve")
OneSourceOneDest("ProcessRankClosestConvolve")
OneSourceOneDest("ProcessRankMaxConvolve")
OneSourceOneDest("ProcessRankMinConvolve")
OneSourceOneDest("ProcessConvolve")
OneSourceOneDest("ProcessConvolveSep")
OneSourceOneDest("ProcessConvolveRep")
OneSourceOneDest("ProcessConvolveDual")
OneSourceOneDest("ProcessCompassConvolve")
OneSourceOneDest("ProcessMeanConvolve")
OneSourceOneDest("ProcessGaussianConvolve")
OneSourceOneDest("ProcessBarlettConvolve")
OneSourceTwoDests("ProcessInterlaceSplit", nil, function (image) if (image:Height()) then return image:Height() else return image:Height()/2 end end)

function im.ProcessInterlaceSplitNew(src_image)
  -- create destination image
  local dst_height1 = src_image:Height()/2
  if math.mod(src_image:Height(), 2) then
    dst_height1 = dst_height1 + 1
  end

  local dst_image1 = im.ImageCreateBased(src_image, nil, dst_height1)
  local dst_image2 = im.ImageCreateBased(src_image, nil, src_image:Height()/2)

  -- call method, repassing all parameters
  im.ProcessInterlaceSplit(src_image, dst_image1, dst_image2)
  return dst_image1, dst_image2
end

local function int_datatype (image)
  local data_type = image:DataType()
  if data_type == im.BYTE or data_type == im.USHORT then
    data_type = im.INT
  end
  return data_type
end

OneSourceOneDest("ProcessDiffOfGaussianConvolve", nil, nil, nil, int_datatype)
OneSourceOneDest("ProcessLapOfGaussianConvolve", nil, nil, nil, int_datatype)
OneSourceOneDest("ProcessSobelConvolve")
OneSourceOneDest("ProcessSplineEdgeConvolve")
OneSourceOneDest("ProcessPrewittConvolve")
OneSourceOneDest("ProcessZeroCrossing")
OneSourceOneDest("ProcessCanny")
OneSourceOneDest("ProcessUnArithmeticOp")
TwoSourcesOneDest("ProcessArithmeticOp")
OneSourceOneDest("ProcessUnsharp")
OneSourceOneDest("ProcessSharp")
TwoSourcesOneDest("ProcessSharpKernel")

function im.ProcessArithmeticConstOpNew (src_image, src_const, op)
  local dst_image = im.ImageCreateBased(src_image)
  im.ProcessArithmeticConstOp(src_image, src_const, dst_image, op)
  return dst_image
end

TwoSourcesOneDest("ProcessBlendConst")
ThreeSourcesOneDest("ProcessBlend")
TwoSourcesOneDest("ProcessCompose")
OneSourceTwoDests("ProcessSplitComplex")
TwoSourcesOneDest("ProcessMergeComplex", nil, nil, nil, im.CFLOAT)

function im.ProcessMultipleMeanNew (src_image_list, dst_image)
  local dst_image = im.ImageCreateBased(src_image_list[1])
  im.ProcessMultipleMean(src_image_list, dst_image)
  return dst_image
end

function im.ProcessMultipleStdDevNew (src_image_list, mean_image)
  local dst_image = im.ImageCreateBased(src_image_list[1])
  im.ProcessMultipleStdDev(src_image_list, mean_image, dst_image)
  return dst_image
end

TwoSourcesOneDest("ProcessAutoCovariance")
OneSourceOneDest("ProcessMultiplyConj")
OneSourceOneDest("ProcessQuantizeRGBUniform", nil, nil, im.MAP, nil)
OneSourceOneDest("ProcessQuantizeGrayUniform")
OneSourceOneDest("ProcessExpandHistogram")
OneSourceOneDest("ProcessEqualizeHistogram")

function im.ProcessSplitYChromaNew (src_image)
  local y_image = im.ImageCreateBased(src_image, nil, nil, im.GRAY, im.BYTE)
  local chroma_image = im.ImageCreateBased(src_image, nil, nil, im.RGB, im.BYTE)
  im.ProcessSplitYChroma(src_image, y_image, chroma_image)
  return y_image, chroma_image
end

OneSourceThreeDests("ProcessSplitHSI", nil, nil, im.GRAY, im.FLOAT)
ThreeSourcesOneDest("ProcessMergeHSI", nil, nil, im.RGB, im.BYTE)

function im.ProcessSplitComponentsNew (src_image)
  local depth = src_image:Depth()
  local dst_images = {}
  for i = 1, depth do
    table.insert(dst_images, im.ImageCreateBased(src_image, nil, nil, im.GRAY))
  end
  im.ProcessSplitComponents(src_image, dst_images)
  return unpack(dst_images) --must replace this by table.unpack when 5.1 is not supported
end

function im.ProcessMergeComponentsNew (src_image_list)
  local dst_image = im.ImageCreateBased(src_image_list[1], nil, nil, im.RGB)
  im.ProcessMergeComponents(src_image_list, dst_image)
  return dst_image
end

OneSourceOneDest("ProcessNormalizeComponents", nil, nil, nil, im.FLOAT)
OneSourceOneDest("ProcessReplaceColor")
TwoSourcesOneDest("ProcessBitwiseOp")
OneSourceOneDest("ProcessBitwiseNot")
OneSourceOneDest("ProcessBitMask")
OneSourceOneDest("ProcessBitPlane")
OneSourceOneDest("ProcessToneGamut")
OneSourceOneDest("ProcessUnNormalize", nil, nil, nil, im.BYTE)
OneSourceOneDest("ProcessDirectConv", nil, nil, nil, im.BYTE)
OneSourceOneDest("ProcessNegative")
OneSourceOneDest("ProcessRangeContrastThreshold", nil, nil, im.BINARY, nil)
OneSourceOneDest("ProcessLocalMaxThreshold", nil, nil, im.BINARY, nil)
OneSourceOneDest("ProcessThreshold", nil, nil, im.BINARY, nil)
TwoSourcesOneDest("ProcessThresholdByDiff")
OneSourceOneDest("ProcessHysteresisThreshold", nil, nil, im.BINARY, nil)
OneSourceOneDest("ProcessUniformErrThreshold", nil, nil, im.BINARY, nil)
OneSourceOneDest("ProcessDifusionErrThreshold")
OneSourceOneDest("ProcessPercentThreshold")
OneSourceOneDest("ProcessOtsuThreshold")
OneSourceOneDest("ProcessMinMaxThreshold", nil, nil, im.BINARY, nil)
OneSourceOneDest("ProcessSliceThreshold", nil, nil, im.BINARY, nil)
OneSourceOneDest("ProcessPixelate")
OneSourceOneDest("ProcessPosterize")

